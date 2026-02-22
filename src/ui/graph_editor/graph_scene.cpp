#include "graph_scene.h"
#include "model/fuzzy_logic/numeric_fuzzifier.h"

GraphScene::GraphScene(std::shared_ptr<FCM> fcm) : fuzzifier(std::make_shared<NumericFuzzifier>()), fcm(fcm) {}

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

    QStringList termsNames = {};
    for (const auto& term : fcm->terms) {
        termsNames.append(term.second.name);
    }
    termsNames.sort();

    if (mode == EditMode::Create && editable) {

        if (e->button() == Qt::LeftButton && !item) {
            auto* n = new NodeItem("F" + QString::number(counter), counter);
            counter++;
            addItem(n);
            n->setPos(e->scenePos());
            fcm->concepts[n->getId()].id = n->getId();

            /*bool ok;
            double v = QInputDialog::getDouble(
                nullptr, "Node value", "Value:", 0.0, 0, 1, 3, &ok);
            if (ok) {
                n->setValue(v);
            }*/

            bool ok;
            QString value = QInputDialog::getItem(
                nullptr,
                "Выбор значения",
                "Выберите элемент:",
                termsNames,
                0,
                false,
                &ok
                );

            if (ok) {
                Term t;
                for (const auto& term : fcm->terms) {
                    if (term.second.name == value) {
                        t = term.second;
                    }
                }
                n->setValue(t.value);
                fcm->concepts[n->getId()].termId = t.id;
            }
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

                /*bool ok;
                double v = QInputDialog::getDouble(
                    nullptr, "Edge value", "Value:", 0.0, -1, 1, 3, &ok);
                if (ok) {
                    ed->setValue(v);
                }*/

                bool ok;
                QString value = QInputDialog::getItem(
                    nullptr,
                    "Выбор значения",
                    "Выберите элемент:",
                    termsNames,
                    0,          // индекс по умолчанию
                    false,      // можно ли вводить вручную
                    &ok
                    );

                if (ok) {
                    Term t;
                    for (const auto& term : fcm->terms) {
                        if (term.second.name == value) {
                            t = term.second;
                        }
                    }
                    ed->setValue(t.value);
                    fcm->weights[ed->getId()].termId = t.id;
                }

                firstNode = nullptr;
            }
        }
    }

    if ((mode == EditMode::EditValues || !editable) && e->button() == Qt::RightButton) {

        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            /*bool ok;
            double v = QInputDialog::getDouble(
                nullptr, "Node value", "Value:",
                n->getValue(), -1000, 1000, 3, &ok);
            if (ok) n->setValue(v);*/

            auto currentTerm = fuzzifier->fuzzify(fcm, n->getValue());
            size_t currentTermInd = 0;
            for (size_t i = 0; i < termsNames.size(); ++i) {
                if (termsNames[i] == currentTerm.name) {
                    currentTermInd = i;
                }
            }

            bool ok;
            QString value = QInputDialog::getItem(
                nullptr,
                "Выбор значения",
                "Выберите элемент:",
                termsNames,
                currentTermInd,
                false,
                &ok
                );

            if (ok) {
                double v;
                for (const auto& term : fcm->terms) {
                    if (term.second.name == value) {
                        v = term.second.value;
                    }
                }
                n->setValue(v);
            }
        }

        if (auto ed = qgraphicsitem_cast<EdgeItem*>(item)) {
            /*bool ok;
            double v = QInputDialog::getDouble(
                nullptr, "Edge value", "Value:",
                ed->getValue(), -1000, 1000, 3, &ok);
            if (ok) {
                ed->setValue(v);
            }*/

            auto currentTerm = fuzzifier->fuzzify(fcm, ed->getValue());
            size_t currentTermInd = 0;
            for (size_t i = 0; i < termsNames.size(); ++i) {
                if (termsNames[i] == currentTerm.name) {
                    currentTermInd = i;
                }
            }

            bool ok;
            QString value = QInputDialog::getItem(
                nullptr,
                "Выбор значения",
                "Выберите элемент:",
                termsNames,
                currentTermInd,
                false,
                &ok
                );

            if (ok) {
                double v;
                for (const auto& term : fcm->terms) {
                    if (term.second.name == value) {
                        v = term.second.value;
                    }
                }
                ed->setValue(v);
            }
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
    auto copyScene = new GraphScene(std::make_shared<FCM>(*fcm));
    QList<NodeItem*> nodes(counter);
    for (QGraphicsItem* item : items()) {
        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            auto newNode = new NodeItem(n->getName(), n->getId());
            copyScene->addItem(newNode);
            newNode->setPos(n->pos());
            newNode->setValue(n->getValue());
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
