#include "creation_presenter.h"

CreationPresenter::CreationPresenter(std::shared_ptr<FCM> fcm, QObject* parent) : QObject(parent), fcm(fcm) {}

void CreationPresenter::createConcept(const QPointF pos) {
    auto id = QUuid::createUuid();
    fcm->concepts[id] = std::make_shared<Concept>();
    fcm->concepts[id]->id = id;
    fcm->concepts[id]->pos = pos;
    emit conceptCreated(fcm->concepts[id]);

    ConceptWindow* conceptWindow = new ConceptWindow(fcm->terms, fcm->concepts[id], dynamic_cast<QWidget*>(parent()));
    conceptWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(conceptWindow, &ConceptWindow::applied,
        [=](const std::shared_ptr<Concept>& concept) {
            *fcm->concepts[id] = *concept;
            emit conceptUpdated(fcm->concepts[id]);
        });

    connect(conceptWindow, &QObject::destroyed, this, [=]() {
        conceptWindows.erase(id);
    });

    connect(conceptWindow, &ConceptWindow::deleted, this, &CreationPresenter::deleteConcept);

    conceptWindows[id] = conceptWindow;
    conceptWindow->show();
}

void CreationPresenter::updateConcept(QUuid id) {
    if (conceptWindows.find(id) != conceptWindows.end()) {
        return;
    }

    ConceptWindow* conceptWindow = new ConceptWindow(fcm->terms, fcm->concepts[id], dynamic_cast<QWidget*>(parent()));
    conceptWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(conceptWindow, &ConceptWindow::applied,
        [=](const std::shared_ptr<Concept>& concept) {
            *fcm->concepts[id] = *concept;
            emit conceptUpdated(fcm->concepts[id]);
        });

    connect(conceptWindow, &QObject::destroyed, this, [=]() {
        conceptWindows.erase(id);
    });

    connect(conceptWindow, &ConceptWindow::deleted, this, &CreationPresenter::deleteConcept);

    conceptWindows[id] = conceptWindow;
    conceptWindow->show();
}

void CreationPresenter::createWeight(QUuid nodeId) {
    if (!firstNodeId) {
        firstNodeId = nodeId;
        return;
    }
    createWeight(*firstNodeId, nodeId);
    firstNodeId = {};
}

void CreationPresenter::createWeight(QUuid fromNodeId, QUuid toNodeId) {
    if (fromNodeId == toNodeId) {
        return;
    }
    for (const auto& [_, weight] : fcm->weights) {
        if (weight->fromConceptId == fromNodeId && weight->toConceptId == toNodeId) {
            return;
        }
    }
    auto id = QUuid::createUuid();
    fcm->weights[id] = std::make_shared<Weight>();
    fcm->weights[id]->id = id;
    fcm->weights[id]->fromConceptId = fromNodeId;
    fcm->weights[id]->toConceptId = toNodeId;
    emit weightCreated(fcm->weights[id]);


    WeightWindow* weightWindow = new WeightWindow(fcm->terms, fcm->weights[id], dynamic_cast<QWidget*>(parent()));
    weightWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(weightWindow, &WeightWindow::applied,
            [=](const std::shared_ptr<Weight>& weight) {
                *fcm->weights[id] = *weight;
                emit weightUpdated(fcm->weights[id]);
            });

    connect(weightWindow, &QObject::destroyed, this, [=]() {
        weightWindows.erase(id);
    });

    connect(weightWindow, &WeightWindow::deleted, this, &CreationPresenter::deleteWeight);

    weightWindows[id] = weightWindow;
    weightWindow->show();
}

void CreationPresenter::updateWeight(QUuid id) {
    if (weightWindows.find(id) != weightWindows.end()) {
        return;
    }

    WeightWindow* weightWindow = new WeightWindow(fcm->terms, fcm->weights[id], dynamic_cast<QWidget*>(parent()));
    weightWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(weightWindow, &WeightWindow::applied,
            [=](const std::shared_ptr<Weight>& weight) {
                *fcm->weights[id] = *weight;
                emit weightUpdated(fcm->weights[id]);
            });

    connect(weightWindow, &QObject::destroyed, this, [=]() {
        weightWindows.erase(id);
    });

    connect(weightWindow, &WeightWindow::deleted, this, &CreationPresenter::deleteWeight);

    weightWindows[id] = weightWindow;
    weightWindow->show();
}

void CreationPresenter::deleteConcept(QUuid id) {
    if (conceptWindows.find(id) != conceptWindows.end()) {
        conceptWindows[id]->deleteLater();
    }

    std::vector<QUuid> connectedWeights;
    for (const auto& [wid, weight] : fcm->weights) {
        if (weight->fromConceptId == id || weight->toConceptId == id) {
            connectedWeights.push_back(wid);
        }
    }

    for (QUuid wid : connectedWeights) {
        deleteWeight(wid);
    }

    fcm->concepts.erase(id);
    emit conceptDeleted(id);
}

void CreationPresenter::deleteWeight(QUuid id) {
    if (weightWindows.find(id) != weightWindows.end()) {
        weightWindows[id]->deleteLater();
    }
    fcm->weights.erase(id);
    weightWindows.erase(id);
    emit weightDeleted(id);
}

void CreationPresenter::setConceptPredictedValues(QUuid id) {
    if (conceptWindows.find(id) == conceptWindows.end()) {
        return;
    }
    conceptWindows[id]->setPredictedValues();
}

void CreationPresenter::setWeightPredictedValues(QUuid id) {
    if (weightWindows.find(id) == weightWindows.end()) {
        return;
    }
    weightWindows[id]->setPredictedValues();
}

void CreationPresenter::updateTerm(QUuid id) {
    for (auto [_, concept] : fcm->concepts) {
        if (concept->term && concept->term->id == id) {
            emit conceptUpdated(concept);
        }
    }
    for (auto [_, weight] : fcm->weights) {
        if (weight->term && weight->term->id == id) {
            emit weightUpdated(weight);
        }
    }
    updateTermsLists();
}

void CreationPresenter::deleteTerm(QUuid id) {
    fcm->terms.erase(id);
    for (auto [_, concept] : fcm->concepts) {
        if (concept->term && concept->term->id == id) {
            concept->term = nullptr;
            emit conceptUpdated(concept);
        }
    }
    for (auto [_, weight] : fcm->weights) {
        if (weight->term && weight->term->id == id) {
            weight->term = nullptr;
            emit weightUpdated(weight);
        }
    }
    updateTermsLists();
}

void CreationPresenter::updateTermsLists() {
    for (auto [_, window] : conceptWindows) {
        window->updateTermsList();
    }
    for (auto [_, window] : weightWindows) {
        window->updateTermsList();
    }
}
