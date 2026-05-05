#include "graph_scene.h"
#include "model/entities/helpers/fcm_copy.h"

GraphScene::GraphScene(std::shared_ptr<FCM> fcm, std::shared_ptr<ScenePresenter> presenter, ElementWindowMode elementWindowMode)
    : fcm(fcm), presenter(presenter), elementWindowMode(elementWindowMode) {
    setSceneRect(-10000, -10000, 20000, 20000);
    if (!fcm) {
        return;
    }

    for (const auto& [_, concept] : fcm->concepts) {
        auto* n = new NodeItem(concept);
        addItem(n);
        n->setPos(concept->pos);
        n->setValue(concept->term);
        nodes[concept->id] = n;
        connect(n, &NodeItem::positionChanged, this, &GraphScene::conceptPositionChanged);
    }

    for (const auto& [_, weight] : fcm->weights) {
        auto* ed = new EdgeItem(nodes[weight->fromConceptId], nodes[weight->toConceptId], weight->id);
        addItem(ed);
        nodes[weight->fromConceptId]->addEdge(ed);
        nodes[weight->toConceptId]->addEdge(ed);
        ed->updatePosition();
        ed->setValue(weight->term);
        edges[weight->id] = ed;
    }

    connect(presenter.get(), &ScenePresenter::conceptCreated, this, &GraphScene::conceptCreated);
    connect(presenter.get(), &ScenePresenter::conceptUpdated, this, &GraphScene::conceptUpdated);
    connect(presenter.get(), &ScenePresenter::weightCreated, this, &GraphScene::weightCreated);
    connect(presenter.get(), &ScenePresenter::weightUpdated, this, &GraphScene::weightUpdated);
    connect(presenter.get(), &ScenePresenter::conceptDeleted, this, &GraphScene::conceptDeleted);
    connect(presenter.get(), &ScenePresenter::weightDeleted, this, &GraphScene::weightDeleted);
}

void GraphScene::switchMode() {
    if (mode == EditMode::Create) {
        cancelPendingWeightCreation();
        mode = EditMode::EditValues;
    } else {
        mode = EditMode::Create;
    }
    modeChanged(mode);
}

void GraphScene::conceptCreated(std::shared_ptr<Concept> concept) {
    auto* n = new NodeItem(concept);
    addItem(n);
    connect(n, &NodeItem::positionChanged, this, &GraphScene::conceptPositionChanged);
    n->setPos(concept->pos);
    if (!conceptCreationColorEditBlocked) {
        n->setValue(concept->term);
    }
    nodes[concept->id] = n;
}

void GraphScene::weightCreated(std::shared_ptr<Weight> weight) {
    auto* ed = new EdgeItem(nodes[weight->fromConceptId], nodes[weight->toConceptId], weight->id);
    addItem(ed);
    nodes[weight->fromConceptId]->addEdge(ed);
    nodes[weight->toConceptId]->addEdge(ed);
    ed->updatePosition();
    edges[weight->id] = ed;
}

void GraphScene::conceptUpdated(std::shared_ptr<Concept> concept) {
    nodes[concept->id]->setPos(concept->pos);
    nodes[concept->id]->setName(concept->name);
    if (!conceptCreationColorEditBlocked) {
        nodes[concept->id]->setValue(concept->term);
    }
}

void GraphScene::weightUpdated(std::shared_ptr<Weight> weight) {
    edges[weight->id]->setValue(weight->term);
}

void GraphScene::conceptDeleted(QUuid id) {
    NodeItem* node = nodes[id];
    removeItem(node);
    nodes.erase(id);
    delete node;
}

void GraphScene::weightDeleted(QUuid id) {
    EdgeItem* edge = edges[id];
    edge->src->removeEdge(edge);
    edge->dst->removeEdge(edge);

    removeItem(edge);
    edges.erase(id);
    delete edge;
}

void GraphScene::setConceptColor(QUuid id, QColor color, bool highlight) {
    nodes[id]->setBrush(color);
    nodes[id]->highlight(highlight);
}

void GraphScene::setWeightColor(QUuid id, QColor color) {
    edges[id]->setColor(color);
}

void GraphScene::updatePendingWeightPreview(const QPointF& scenePos) {
    if (mode != EditMode::Create) {
        clearPendingWeightPreview();
        return;
    }
    syncPendingWeightPreview(scenePos);
}

void GraphScene::cancelPendingWeightCreation() {
    presenter->cancelPendingWeightCreation();
    clearPendingWeightPreview();
}

void GraphScene::blockConceptCreationColorEdit(bool flag) {
    conceptCreationColorEditBlocked = flag;
}

void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    NodeItem* clickedNode = nullptr;
    EdgeItem* clickedEdge = nullptr;
    for (QGraphicsItem* item : items(e->scenePos())) {
        if (!clickedNode) {
            clickedNode = findNodeItem(item);
        }
        if (!clickedEdge) {
            clickedEdge = findEdgeItem(item);
        }
        if (clickedNode && clickedEdge) {
            break;
        }
    }
    if (clickedNode) {
        clickedEdge = nullptr;
    }

    bool editable = e->widget() == views()[0]->viewport();

    if (mode == EditMode::Create && editable) {

        if (e->button() == Qt::LeftButton && !clickedNode && !clickedEdge) {
            presenter->createConcept(e->scenePos());
        }

        if (clickedNode && e->button() == Qt::RightButton) {
            presenter->createWeight(clickedNode->getId());
            syncPendingWeightPreview(e->scenePos());
        }
    }

    if ((mode == EditMode::EditValues || !editable) && e->button() == Qt::RightButton) {

        if (clickedNode) {
            presenter->updateConcept(clickedNode->getId(), elementWindowMode);
        }

        if (clickedEdge) {
            presenter->updateWeight(clickedEdge->getId(), elementWindowMode);
        }
    }
    if (mode == EditMode::Create && editable) {
        QGraphicsScene::mousePressEvent(e);
    } else {
        e->accept();
    }
}

NodeItem* GraphScene::findNodeItem(QGraphicsItem* item) const {
    while (item) {
        if (auto* node = qgraphicsitem_cast<NodeItem*>(item)) {
            return node;
        }
        item = item->parentItem();
    }
    return nullptr;
}

EdgeItem* GraphScene::findEdgeItem(QGraphicsItem* item) const {
    while (item) {
        if (auto* edge = qgraphicsitem_cast<EdgeItem*>(item)) {
            return edge;
        }
        item = item->parentItem();
    }
    return nullptr;
}

void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    syncPendingWeightPreview(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void GraphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    if (conceptPositionChangedFlag) {
        presenter->emitAutosave();
        conceptPositionChangedFlag = false;
    }
}

void GraphScene::syncPendingWeightPreview(const QPointF& scenePos) {
    if (!presenter->hasPendingWeightStart()) {
        clearPendingWeightPreview();
        return;
    }

    const auto pendingId = presenter->pendingWeightStartId();
    if (!pendingId.has_value()) {
        clearPendingWeightPreview();
        return;
    }

    const auto nodeIt = nodes.find(*pendingId);
    if (nodeIt == nodes.end()) {
        clearPendingWeightPreview();
        return;
    }

    updatePreviewGeometry(nodeIt->second, scenePos);
}

void GraphScene::clearPendingWeightPreview() {
    if (previewArrow) {
        removeItem(previewArrow);
        delete previewArrow;
        previewArrow = nullptr;
    }
    if (previewEdge) {
        removeItem(previewEdge);
        delete previewEdge;
        previewEdge = nullptr;
    }
}

void GraphScene::updatePreviewGeometry(NodeItem* startNode, const QPointF& scenePos) {
    if (!previewEdge) {
        previewEdge = addPath(QPainterPath(), QPen(QColor(0, 0, 0), 2), Qt::NoBrush);
        previewEdge->setZValue(1);
        previewEdge->setAcceptedMouseButtons(Qt::NoButton);
        QPen pen = previewEdge->pen();
        pen.setCosmetic(true);
        previewEdge->setPen(pen);
    }

    if (!previewArrow) {
        previewArrow = addPolygon(QPolygonF(), Qt::NoPen, QBrush(QColor(0, 0, 0)));
        previewArrow->setZValue(1);
        previewArrow->setAcceptedMouseButtons(Qt::NoButton);
    }

    const auto geometry = EdgeItem::buildGeometry(startNode->scenePos(), scenePos, 25, 0);
    if (!geometry.visible) {
        previewEdge->setPath(QPainterPath());
        previewArrow->setVisible(false);
        return;
    }

    previewArrow->setVisible(true);
    previewEdge->setPath(geometry.path);
    previewArrow->setPolygon(geometry.arrow);
}

GraphScene* GraphScene::copy(std::shared_ptr<ScenePresenter> presenter, ElementWindowMode elementWindowMode) const {
    auto copyScene = new GraphScene({}, presenter, elementWindowMode);
    copyScene->setFCM(cloneFCMForRuntime(fcm));
    for (const auto& [id, concept] : copyScene->fcm->concepts) {
        auto* newNode = new NodeItem(concept);
        copyScene->addItem(newNode);
        connect(newNode, &NodeItem::positionChanged, copyScene, &GraphScene::conceptPositionChanged);
        newNode->setPos(concept->pos);
        newNode->setValue(concept->term);
        copyScene->nodes[id] = newNode;
    }
    for (const auto& [id, weight] : copyScene->fcm->weights) {
        auto* newEdge = new EdgeItem(copyScene->nodes[weight->fromConceptId], copyScene->nodes[weight->toConceptId], id);
        copyScene->addItem(newEdge);
        copyScene->nodes[weight->fromConceptId]->addEdge(newEdge);
        copyScene->nodes[weight->toConceptId]->addEdge(newEdge);
        newEdge->setValue(weight->term);
        newEdge->updatePosition();
        copyScene->edges[id] = newEdge;
    }
    copyScene->setMode(EditMode::EditValues);
    return copyScene;
}

void GraphScene::conceptPositionChanged(QUuid id) {
    conceptPositionChangedFlag = true;
    if (nodes.find(id) != nodes.end()) {
        presenter->updateConceptPosition(id, nodes[id]->pos());
    }
}
