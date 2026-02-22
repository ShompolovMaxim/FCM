#pragma once
#include "model/predictor.h"
#include "simulation_presenter.h"
#include "ui/graph_editor/edge_item.h"

SimulationPresenter::SimulationPresenter(QObject* parent) : QObject(parent) {}

SimulationPresenter::~SimulationPresenter()
{
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void SimulationPresenter::simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes) {
    if (workerThread.joinable()) {
        workerThread.join();
    }

    step = 0;

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

    auto predictor = std::make_shared<Predictor>(predictionParameters, fcm);
    workerThread = std::thread([this, predictor]() {
        predictor->perform();
    });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, nodes, predictor, predictionParameters]() {
        if (step >= predictionParameters.fixedSteps) {
            timer->stop();
            return;
        }

        if (predictor->getCount() > step) {
            auto newFCM = predictor->getFCM(step);
            for (auto* node : nodes) {
                node->setValue(newFCM.concepts[node->getId()]);
            }
            ++step;
        }
    });

    timer->start(1000);
}
