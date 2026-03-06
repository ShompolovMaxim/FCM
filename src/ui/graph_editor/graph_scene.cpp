#include "graph_scene.h"
#include "model/fuzzy_logic/numeric_fuzzifier.h"

#include "ui/concept_window/concept_window.h"
#include "ui/weight_window/weight_window.h"

GraphScene::GraphScene(std::shared_ptr<FCM> fcm) : fuzzifier(std::make_shared<NumericFuzzifier>()), fcm(fcm) {
    if (!fcm) {
        return;
    }
    std::map<size_t, NodeItem*> nodes;
    for (const auto& [_, concept] : fcm->concepts) {
        auto* n = new NodeItem("F" + QString::number(counter), counter, std::make_shared<Concept>(concept));
        counter++;
        addItem(n);
        n->setPos(concept.pos);
        n->setValue(concept.value);
        nodes[concept.id] = n;
    }

    for (const auto& [_, weight] : fcm->weights) {
        auto* ed = new EdgeItem(nodes[weight.fromConceptId], nodes[weight.toConceptId], weight.id);
        addItem(ed);
        nodes[weight.fromConceptId]->addEdge(ed);
        nodes[weight.toConceptId]->addEdge(ed);
        ed->updatePosition();
        ed->setValue(weight.value);
    }
}

void GraphScene::switchMode() {
    if (mode == EditMode::Create) {
        mode = EditMode::EditValues;
    } else {
        mode = EditMode::Create;
    }
    modeChanged(mode);
}

void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem* item = itemAt(e->scenePos(), QTransform());

    bool editable = e->widget() == views()[0]->viewport();

    if (mode == EditMode::Create && editable) {

        if (e->button() == Qt::LeftButton && !item) {
            fcm->concepts[counter].id = counter;
            auto* n = new NodeItem("F" + QString::number(counter), counter, std::make_shared<Concept>(fcm->concepts[counter]));
            counter++;
            addItem(n);
            n->setPos(e->scenePos());
            fcm->concepts[n->getId()].id = n->getId();
            fcm->concepts[n->getId()].pos = e->scenePos();

            ConceptWindow* conceptWindow =
                new ConceptWindow(fcm->terms, fcm->concepts[n->getId()], n->getPredictedValues(), views().first()->window());

            conceptWindow->setAttribute(Qt::WA_DeleteOnClose);

            connect(conceptWindow, &ConceptWindow::applied,
                    [=](const Concept& concept)
                    {
                        fcm->concepts[n->getId()] = concept;
                        n->setConcept(std::make_shared<Concept>(concept));
                    });

            n->setWindow(conceptWindow);
            conceptWindow->show();
        }

        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            if (!firstNode) {
                firstNode = n;
            } else {
                if (firstNode == n) {
                    firstNode = nullptr;
                    return;
                }
                auto* ed = new EdgeItem(firstNode, n, edgesCounter++);
                addItem(ed);
                firstNode->addEdge(ed);
                n->addEdge(ed);
                ed->updatePosition();
                fcm->weights[ed->getId()].id = ed->getId();
                fcm->weights[ed->getId()].fromConceptId = ed->src->getId();
                fcm->weights[ed->getId()].toConceptId = ed->dst->getId();

                WeightWindow* weightWindow = new WeightWindow(fcm->terms, fcm->weights[ed->getId()], views().first()->window());

                weightWindow->setAttribute(Qt::WA_DeleteOnClose);

                connect(weightWindow, &WeightWindow::applied,
                        [=](const Weight& weight)
                        {
                            fcm->weights[ed->getId()] = weight;
                            ed->setValue(weight.value);
                            firstNode = nullptr;
                        });

                weightWindow->show();
            }
        }
    }

    if ((mode == EditMode::EditValues || !editable) && e->button() == Qt::RightButton) {

        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {

            ConceptWindow* conceptWindow =
                new ConceptWindow(fcm->terms, fcm->concepts[n->getId()], n->getPredictedValues(), views().first()->window());

            conceptWindow->setAttribute(Qt::WA_DeleteOnClose);

            connect(conceptWindow, &ConceptWindow::applied,
                    [=](const Concept& concept)
                    {
                        fcm->concepts[n->getId()] = concept;
                        n->setConcept(std::make_shared<Concept>(concept));
                    });

            n->setWindow(conceptWindow);

            conceptWindow->show();
        }

        if (auto ed = qgraphicsitem_cast<EdgeItem*>(item)) {

            WeightWindow* weightWindow = new WeightWindow(fcm->terms, fcm->weights[ed->getId()], views().first()->window());

            weightWindow->setAttribute(Qt::WA_DeleteOnClose);

            connect(weightWindow, &WeightWindow::applied,
                    [=](const Weight& weight)
                    {
                        fcm->weights[ed->getId()] = weight;
                        ed->setValue(weight.value);
                    });

            weightWindow->show();
        }
    }
    if (mode == EditMode::Create && editable) {
        QGraphicsScene::mousePressEvent(e);
    } else {
        e->accept();
    }
    //QGraphicsScene::mousePressEvent(e);
}

GraphScene* GraphScene::copy() const {
    auto copyScene = new GraphScene({});
    copyScene->setFCM(std::make_shared<FCM>(*fcm));
    QList<NodeItem*> nodes(counter);
    for (QGraphicsItem* item : items()) {
        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            auto newNode = new NodeItem(n->getName(), n->getId(), n->getConcept());
            copyScene->addItem(newNode);
            newNode->setPos(n->pos());
            newNode->setValue(n->getValue());
            newNode->setWindow(n->getWindow());
            nodes[newNode->getId()] = newNode;
        }
    }
    for (QGraphicsItem* item : items()) {
        if (auto e = qgraphicsitem_cast<EdgeItem*>(item)) {
            auto newEdge = new EdgeItem(nodes[e->src->getId()], nodes[e->dst->getId()], e->getId());
            copyScene->addItem(newEdge);
            newEdge->setPos(e->pos());
            newEdge->setValue(e->getValue());
            nodes[e->src->getId()]->addEdge(newEdge);
            nodes[e->dst->getId()]->addEdge(newEdge);
            newEdge->updatePosition();
        }
    }
    copyScene->setMode(EditMode::EditValues);
    copyScene->counter = counter;
    return copyScene;
}
