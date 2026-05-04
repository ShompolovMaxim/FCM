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
        connect(n, &NodeItem::positionChanged, this, &GraphScene::conceptPositionChanged);
        n->setPos(concept->pos);
        n->setValue(concept->term);
        nodes[concept->id] = n;
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

void GraphScene::blockConceptCreationColorEdit(bool flag) {
    conceptCreationColorEditBlocked = flag;
}

void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem* item = itemAt(e->scenePos(), QTransform());

    bool editable = e->widget() == views()[0]->viewport();

    if (mode == EditMode::Create && editable) {

        if (e->button() == Qt::LeftButton && !item) {
            presenter->createConcept(e->scenePos());
        }

        if (auto n = qgraphicsitem_cast<NodeItem*>(item); n && e->button() == Qt::RightButton) {
            presenter->createWeight(n->getId());
        }
    }

    if ((mode == EditMode::EditValues || !editable) && e->button() == Qt::RightButton) {

        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {

            presenter->updateConcept(n->getId(), elementWindowMode);
        }

        if (auto ed = qgraphicsitem_cast<EdgeItem*>(item)) {
            presenter->updateWeight(ed->getId(), elementWindowMode);
        }
    }
    if (mode == EditMode::Create && editable) {
        QGraphicsScene::mousePressEvent(e);
    } else {
        e->accept();
    }
}

void GraphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    if (conceptPositionChangedFlag) {
        presenter->emitAutosave();
        conceptPositionChangedFlag = false;
    }
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

void GraphScene::conceptPositionChanged() {
    conceptPositionChangedFlag = true;
}
