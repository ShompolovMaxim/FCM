#include "graph_scene.h"
#include "model/fuzzy_logic/numeric_fuzzifier.h"

GraphScene::GraphScene(std::shared_ptr<FCM> fcm, std::shared_ptr<CreationPresenter> presenter) : fuzzifier(std::make_shared<NumericFuzzifier>()), fcm(fcm), presenter(presenter) {
    if (!fcm) {
        return;
    }

    for (const auto& [_, concept] : fcm->concepts) {
        auto* n = new NodeItem(concept);
        counter++;
        addItem(n);
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

    connect(presenter.get(), &CreationPresenter::conceptCreated, this, &GraphScene::conceptCreated);
    connect(presenter.get(), &CreationPresenter::conceptUpdated, this, &GraphScene::conceptUpdated);
    connect(presenter.get(), &CreationPresenter::weightCreated, this, &GraphScene::weightCreated);
    connect(presenter.get(), &CreationPresenter::weightUpdated, this, &GraphScene::weightUpdated);
    connect(presenter.get(), &CreationPresenter::conceptDeleted, this, &GraphScene::conceptDeleted);
    connect(presenter.get(), &CreationPresenter::weightDeleted, this, &GraphScene::weightDeleted);
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
    n->setPos(concept->pos);
    n->setValue(concept->term);
    //n->setConcept(concept);
    nodes[concept->id] = n;
    counter++;
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
    nodes[concept->id]->setValue(concept->term);
}

void GraphScene::weightUpdated(std::shared_ptr<Weight> weight) {
    edges[weight->id]->setValue(weight->term);
}

void GraphScene::conceptDeleted(size_t id) {
    NodeItem* node = nodes[id];
    removeItem(node);
    nodes.erase(id);
    delete node;
}

void GraphScene::weightDeleted(size_t id) {
    EdgeItem* edge = edges[id];
    edge->src->removeEdge(edge);
    edge->dst->removeEdge(edge);

    removeItem(edge);
    edges.erase(id);
    delete edge;
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

            presenter->updateConcept(n->getId());
        }

        if (auto ed = qgraphicsitem_cast<EdgeItem*>(item)) {
            presenter->updateWeight(ed->getId());
        }
    }
    if (mode == EditMode::Create && editable) {
        QGraphicsScene::mousePressEvent(e);
    } else {
        e->accept();
    }
}

GraphScene* GraphScene::copy() const {
    auto copyScene = new GraphScene({}, presenter);
    copyScene->setFCM(std::make_shared<FCM>(*fcm));
    QMap<size_t, NodeItem*> nodes;
    for (QGraphicsItem* item : items()) {
        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            auto newNode = new NodeItem(n->getConcept());
            copyScene->addItem(newNode);
            newNode->setPos(n->pos());
            newNode->setValue(fcm->concepts[n->getId()]->term);
            nodes[newNode->getId()] = newNode;
        }
    }
    for (QGraphicsItem* item : items()) {
        if (auto e = qgraphicsitem_cast<EdgeItem*>(item)) {
            auto newEdge = new EdgeItem(nodes[e->src->getId()], nodes[e->dst->getId()], e->getId());
            copyScene->addItem(newEdge);
            newEdge->setPos(e->pos());
            newEdge->setValue(fcm->weights[e->getId()]->term);
            nodes[e->src->getId()]->addEdge(newEdge);
            nodes[e->dst->getId()]->addEdge(newEdge);
            newEdge->updatePosition();
        }
    }
    copyScene->setMode(EditMode::EditValues);
    copyScene->counter = counter;
    return copyScene;
}
