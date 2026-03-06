#include "simulation_presenter.h"
#include "ui/graph_editor/edge_item.h"

SimulationPresenter::SimulationPresenter(QObject* parent) : QObject(parent) {}

SimulationPresenter::~SimulationPresenter()
{
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void SimulationPresenter::simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes_) {
    nodes = nodes_;

    if (workerThread.joinable()) {
        workerThread.join();
    }

    step = -1;

    CalculationFCM fcm;
    fcm.concepts = std::vector<double>(nodes.size());
    fcm.weights = std::vector<std::vector<double>>(nodes.size(), std::vector<double>(nodes.size()));

    for (const auto& node : nodes) {
        fcm.concepts[node->getId()] = node->getValue();
        for (const auto& edge : node->getEdges()) {
            if (edge->src == node) {
                fcm.weights[node->getId()][edge->dst->getId()] = edge->getValue();
            }
        }
    }

    predictor = std::make_shared<Predictor>(predictionParameters, fcm);
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

        /*if (predictor->getCount() > step + 1) {
            ++step;
            auto newFCM = predictor->getFCM(step);
            for (auto* node : nodes) {
                node->setValue(newFCM.concepts[node->getId()]);
            }
        }*/

        goToStep(step+1);

    });

    timer->start(iterationTime);
}

bool SimulationPresenter::goToStep(size_t newStep) {
    if (predictor->getCount() > newStep) {
        auto newFCM = predictor->getFCM(newStep);
        for (auto* node : nodes) {
            node->setValue(newFCM.concepts[node->getId()]);
            node->setPredictedValues(predictor->getConceptHistoryValues(node->getId(), newStep));
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
