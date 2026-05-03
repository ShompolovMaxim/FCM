#include "simulation_presenter.h"

#include "ui/graph_editor/color_value_adapter/linear_approximation_adapter.h"
#include "ui/graph_editor/edge_item.h"

SimulationPresenter::SimulationPresenter(std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent) : QObject(parent), creationPresenter(creationPresenter) {}

SimulationPresenter::~SimulationPresenter() {
    stopExecution();
}

void SimulationPresenter::simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes_, QMap<QUuid, EdgeItem*> edges_, std::shared_ptr<FCM> fcm) {
    stopExecution();

    this->predictionParameters = predictionParameters;
    this->fcm = fcm;
    nodes = nodes_;
    edges = edges_;

    step = -1;

    CalculationFCM calculationFCM;

    for (const auto& [id, concept] : fcm->concepts) {
        calculationFCM.concepts[id] = CalculationConcept{
            id,
            concept->term->value,
            concept->term->fuzzyValue,
            concept->startStep
        };
    }
    for (const auto& [id, weight] : fcm->weights) {
        calculationFCM.weights[id] = CalculationWeight{
            id,
            weight->term->value,
            weight->term->fuzzyValue,
            weight->fromConceptId,
            weight->toConceptId
        };
    }

    predictor = std::make_shared<Predictor>(predictionParameters, calculationFCM);
    auto predictorForThread = predictor;
    workerThread = std::thread([predictorForThread]() {
        predictorForThread->perform();
    });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, predictionParameters]() {
        goToStep(step+1);

        if (predictor->getFinished() && step >= predictor->getCount()) {
            timer->stop();
            emit finished();
        }
    });

    iterationTime = static_cast<int>(1000 / simulationParameters.stepsPerSecond);

    if (simulationParameters.realTime) {
        timer->start(iterationTime);
    } else {
        finish();
    }
}

void SimulationPresenter::finish() {
    stopTimer(calculationTimer);
    calculationTimer = new QTimer(this);
    connect(calculationTimer, &QTimer::timeout, this, [this]() {
        if (!predictionParameters.predictToStatic) {
            updateProgress(predictor->getCount(), predictionParameters.fixedSteps, 0.0);
        }
        if (predictor->getFinished()) {
            goToStep(predictor->getCount());
            calculationTimer->stop();
            emit finished();
        }
    });
    calculationTimer->start(200);
}

bool SimulationPresenter::goToStep(size_t newStep) {
    if (!predictor || !fcm) {
        return false;
    }

    if (predictor->getCount() >= newStep) {
        auto newFCM = predictor->getFCM(newStep);
        auto colorValueAdapter = LinearApproximationColorValueAdapter(fcm->terms);
        for (auto* node : nodes) {
            double value = predictionParameters.useFuzzyValues ? newFCM.concepts[node->getId()].triangularFuzzyValue.defuzzify() : newFCM.concepts[node->getId()].value;
            auto color = colorValueAdapter.getColor(value, 0, 1, true, predictionParameters.useFuzzyValues);
            node->setColor(color);
            fcm->concepts[node->getId()]->predictedValues = predictor->getConceptHistoryValues(node->getId(), newStep);
            creationPresenter->setConceptPredictedValues(node->getId());
        }
        for (auto* edge : edges) {
            double value = predictionParameters.useFuzzyValues ? newFCM.weights[edge->getId()].triangularFuzzyValue.defuzzify() : newFCM.weights[edge->getId()].value;
            auto color = colorValueAdapter.getColor(value, -1, 1, false, predictionParameters.useFuzzyValues);
            edge->setColor(color);
            fcm->weights[edge->getId()]->predictedValues = predictor->getWeightHistoryValues(edge->getId(), newStep);
            creationPresenter->setWeightPredictedValues(edge->getId());
        }
        step = newStep;
        auto maxStep = predictor->getCount();
        if (!predictor->getFinished() && maxStep < 15) {
            maxStep = 10000000;
        }
        if (!predictionParameters.predictToStatic) {
            maxStep = predictionParameters.fixedSteps;
        }
        updateProgress(step, maxStep, newFCM.metricValue);
        return true;
    }
    return false;
}

void SimulationPresenter::stopTimer(QTimer* t) {
    if (t) {
        t->stop();
        t->disconnect(this);
        delete t;
    }
}

bool SimulationPresenter::moveStep(int delta) {
    int newStep = step + delta;
    if (newStep < 0) {
        return false;
    }
    if (!goToStep(newStep)) {
        return false;
    }
    return true;
}

void SimulationPresenter::speedUp() {
    iterationTime = static_cast<int>(iterationTime / speedUpFactor);
    if (timer && timer->isActive()) {
        timer->start(iterationTime);
    }
}

void SimulationPresenter::slowDown() {
    iterationTime = static_cast<int>(iterationTime * slowDownFactor);
    if (timer && timer->isActive()) {
        timer->start(iterationTime);
    }
}

void SimulationPresenter::reset() {
    stopExecution();
    step = 0;
    lastStep = 0;
    nodes.clear();
    edges.clear();

    for (const auto& [id, _] : fcm->concepts) {
        fcm->concepts[id]->predictedValues = {};
        creationPresenter->setConceptPredictedValues(id);
    }
    for (const auto& [id, _] : fcm->weights) {
        fcm->weights[id]->predictedValues = {};
        creationPresenter->setWeightPredictedValues(id);
    }
}

void SimulationPresenter::stopExecution() {
    stopTimer(timer);
    timer = nullptr;

    stopTimer(calculationTimer);
    calculationTimer = nullptr;

    if (predictor) {
        predictor->requestStop();
    }

    if (workerThread.joinable()) {
        workerThread.join();
    }

    predictor = {};
}
