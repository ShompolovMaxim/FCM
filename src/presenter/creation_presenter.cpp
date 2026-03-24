#include "creation_presenter.h"

CreationPresenter::CreationPresenter(std::shared_ptr<FCM> fcm, QObject* parent) : QObject(parent), fcm(fcm) {}

void CreationPresenter::createConcept(const QPointF pos) {
    auto id = fcm->conceptsCounter++;
    fcm->concepts[id] = std::make_shared<Concept>();
    fcm->concepts[id]->id = id;
    fcm->concepts[id]->pos = pos;
    emit conceptCreated(fcm->concepts[id]);

    ConceptWindow* conceptWindow = new ConceptWindow(fcm->terms, fcm->concepts[id], fcm->concepts[id]->predictedValues, dynamic_cast<QWidget*>(parent()));
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

void CreationPresenter::updateConcept(size_t id) {
    if (conceptWindows.find(id) != conceptWindows.end()) {
        return;
    }

    ConceptWindow* conceptWindow = new ConceptWindow(fcm->terms, fcm->concepts[id], fcm->concepts[id]->predictedValues, dynamic_cast<QWidget*>(parent()));
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

void CreationPresenter::createWeight(size_t nodeId) {
    if (!firstNodeId) {
        firstNodeId = nodeId;
        return;
    }
    createWeight(*firstNodeId, nodeId);
    firstNodeId = {};
}

void CreationPresenter::createWeight(size_t fromNodeId, size_t toNodeId) {
    if (fromNodeId == toNodeId) {
        return;
    }
    for (const auto& [_, weight] : fcm->weights) {
        if (weight.fromConceptId == fromNodeId && weight.toConceptId == toNodeId) {
            return;
        }
    }
    auto id = fcm->weightsCounter++;
    fcm->weights[id].id = id;
    fcm->weights[id].fromConceptId = fromNodeId;
    fcm->weights[id].toConceptId = toNodeId;
    emit weightCreated(std::make_shared<Weight>(fcm->weights[id]));


    WeightWindow* weightWindow = new WeightWindow(fcm->terms, fcm->weights[id], fcm->weights[id].predictedValues, dynamic_cast<QWidget*>(parent()));
    weightWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(weightWindow, &WeightWindow::applied,
            [=](const Weight& weight) {
                fcm->weights[id] = weight;
                emit weightUpdated(std::make_shared<Weight>(fcm->weights[id]));
            });

    connect(weightWindow, &QObject::destroyed, this, [=]() {
        weightWindows.erase(id);
    });

    connect(weightWindow, &WeightWindow::deleted, this, &CreationPresenter::deleteWeight);

    weightWindows[id] = weightWindow;
    weightWindow->show();
}

void CreationPresenter::updateWeight(size_t id) {
    if (weightWindows.find(id) != weightWindows.end()) {
        return;
    }

    WeightWindow* weightWindow = new WeightWindow(fcm->terms, fcm->weights[id], fcm->weights[id].predictedValues, dynamic_cast<QWidget*>(parent()));
    weightWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(weightWindow, &WeightWindow::applied,
            [=](const Weight& weight) {
                fcm->weights[id] = weight;
                emit weightUpdated(std::make_shared<Weight>(fcm->weights[id]));
            });

    connect(weightWindow, &QObject::destroyed, this, [=]() {
        weightWindows.erase(id);
    });

    connect(weightWindow, &WeightWindow::deleted, this, &CreationPresenter::deleteWeight);

    weightWindows[id] = weightWindow;
    weightWindow->show();
}

void CreationPresenter::deleteConcept(size_t id) {
    std::vector<size_t> connectedWeights;
    for (const auto& [wid, weight] : fcm->weights) {
        if (weight.fromConceptId == id || weight.toConceptId == id) {
            connectedWeights.push_back(wid);
        }
    }

    for (size_t wid : connectedWeights) {
        deleteWeight(wid);
    }

    fcm->concepts.erase(id);
    emit conceptDeleted(id);
}

void CreationPresenter::deleteWeight(size_t id) {
    fcm->weights.erase(id);
    weightWindows.erase(id);
    emit weightDeleted(id);
}

void CreationPresenter::setConceptPredictedValues(size_t id) {
    if (conceptWindows.find(id) == conceptWindows.end()) {
        return;
    }
    conceptWindows[id]->setPredictedValues(fcm->concepts[id]->predictedValues);
}

void CreationPresenter::setWeightPredictedValues(size_t id) {
    if (weightWindows.find(id) == weightWindows.end()) {
        return;
    }
    weightWindows[id]->setPredictedValues(fcm->weights[id].predictedValues);
}
