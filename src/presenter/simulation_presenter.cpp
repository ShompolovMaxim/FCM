#include "simulation_presenter.h"
#include "ui/graph_editor/edge_item.h"

SimulationPresenter::SimulationPresenter(std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent) : QObject(parent), creationPresenter(creationPresenter) {}

SimulationPresenter::~SimulationPresenter()
{
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void SimulationPresenter::simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes_, QMap<size_t, EdgeItem*> edges_, std::shared_ptr<FCM> fcm) {
    this->fcm = fcm;
    nodes = nodes_;
    edges = edges_;

    if (workerThread.joinable()) {
        workerThread.join();
    }

    step = -1;

    CalculationFCM calculationFCM;

    for (const auto& [id, concept] : fcm->concepts) {
        calculationFCM.concepts[id] = concept->value;
        calculationFCM.conceptsStartSteps[id] = concept->startStep;
    }
    for (const auto& [id, weight] : fcm->weights) {
        calculationFCM.weights[id] = CalculationWeight{
            id,
            weight.value,
            weight.fromConceptId,
            weight.toConceptId
        };
    }

    predictor = std::make_shared<Predictor>(predictionParameters, calculationFCM);
    workerThread = std::thread([this]() {
        predictor->perform();
    });

    iterationTime = static_cast<int>(1000 / simulationParameters.stepsPerSecond);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, predictionParameters]() {
        if (step + 1 >= predictionParameters.fixedSteps) {
            timer->stop();
            return;
        }

        goToStep(step+1);

    });

    timer->start(iterationTime);
}

bool SimulationPresenter::goToStep(size_t newStep) {
    if (predictor->getCount() > newStep) {
        auto newFCM = predictor->getFCM(newStep);
        for (auto* node : nodes) {
            node->setValue(newFCM.concepts[node->getId()]);
            fcm->concepts[node->getId()]->predictedValues = predictor->getConceptHistoryValues(node->getId(), newStep);
            creationPresenter->setConceptPredictedValues(node->getId());
        }
        for (auto* edge : edges) {
            edge->setValue(newFCM.weights[edge->getId()].value);
            fcm->weights[edge->getId()].predictedValues = predictor->getConceptHistoryValues(edge->getId(), newStep);
            creationPresenter->setWeightPredictedValues(edge->getId());
        }
        step = newStep;
        updateProgress(step);
        return true;
    }
    return false;
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
