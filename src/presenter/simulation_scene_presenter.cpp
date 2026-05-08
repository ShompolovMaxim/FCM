#include "simulation_scene_presenter.h"

#include "common/logger.h"
#include "creation_presenter.h"
#include "ui/concept_window/concept_window.h"
#include "ui/graph_editor/color_value_adapter/linear_approximation_adapter.h"
#include "ui/graph_editor/edge_item.h"
#include "ui/graph_editor/graph_scene.h"
#include "ui/weight_window/weight_window.h"

SimulationScenePresenter::SimulationScenePresenter(std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent)
    : ScenePresenter(parent), creationPresenter(creationPresenter) {}

SimulationScenePresenter::~SimulationScenePresenter() {
    stopExecution();
    closeRuntimeWindows();
}

void SimulationScenePresenter::setRuntimeContext(std::shared_ptr<FCM> originalFcm_, std::shared_ptr<FCM> runtimeFcm_, GraphScene* runtimeScene_) {
    closeRuntimeWindows();
    originalFcm = originalFcm_;
    runtimeFcm = runtimeFcm_;
    runtimeScene = runtimeScene_;
}

void SimulationScenePresenter::updateConcept(QUuid id, ElementWindowMode mode) {
    if (originalFcm && originalFcm->concepts.find(id) != originalFcm->concepts.end()) {
        creationPresenter->updateConcept(id, mode);
        return;
    }
    openRuntimeConceptWindow(id, mode);
}

void SimulationScenePresenter::updateWeight(QUuid id, ElementWindowMode mode) {
    if (originalFcm && originalFcm->weights.find(id) != originalFcm->weights.end()) {
        creationPresenter->updateWeight(id, mode);
        return;
    }
    openRuntimeWeightWindow(id, mode);
}

void SimulationScenePresenter::emitAutosave() {}

void SimulationScenePresenter::simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes_, QMap<QUuid, EdgeItem*> edges_) {
    stopExecution();

    this->predictionParameters = predictionParameters;
    nodes = nodes_;
    edges = edges_;

    step = -1;

    CalculationFCM calculationFCM;

    if (!runtimeFcm) {
        return;
    }

    for (const auto& [id, concept] : runtimeFcm->concepts) {
        if (!concept || !concept->term) {
            Logger::warn("Simulation concept term missing");
            return;
        }
        calculationFCM.concepts[id] = CalculationConcept{
            id,
            concept->term->value,
            concept->term->fuzzyValue,
            concept->startStep
        };
    }
    for (const auto& [id, weight] : runtimeFcm->weights) {
        if (!weight || !weight->term) {
            Logger::warn("Simulation weight term missing");
            return;
        }
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

    if (simulationParameters.stepsPerSecond <= 0) {
        Logger::warn("Simulation speed invalid");
        stopExecution();
        return;
    }

    iterationTime = static_cast<int>(1000 / simulationParameters.stepsPerSecond);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, predictionParameters]() {
        goToStep(step+1);

        if (predictor->getFinished() && step >= predictor->getCount()) {
            timer->stop();
            emit finished();
        }
    });

    if (simulationParameters.realTime) {
        timer->start(iterationTime);
    } else {
        finish();
    }
}

void SimulationScenePresenter::finish() {
    if (!predictor) {
        Logger::warn("Simulation state missing");
        return;
    }
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

bool SimulationScenePresenter::goToStep(size_t newStep) {
    if (!predictor || !runtimeFcm) {
        Logger::warn("Simulation state missing");
        return false;
    }

    if (predictor->getCount() >= newStep) {
        auto newFCM = predictor->getFCM(newStep);
        auto colorValueAdapter = LinearApproximationColorValueAdapter(originalFcm->terms);
        for (auto* node : nodes) {
            if (!node) {
                Logger::warn("Simulation node missing");
                continue;
            }
            auto runtimeConceptIt = runtimeFcm->concepts.find(node->getId());
            if (runtimeConceptIt == runtimeFcm->concepts.end()) {
                Logger::warn("Simulation concept missing");
                continue;
            }
            auto newConceptIt = newFCM.concepts.find(node->getId());
            if (newConceptIt == newFCM.concepts.end()) {
                Logger::warn("Simulation concept missing");
                continue;
            }
            double value = predictionParameters.useFuzzyValues ? newConceptIt->second.triangularFuzzyValue.defuzzify() : newConceptIt->second.value;
            auto color = colorValueAdapter.getColor(value, 0, 1, true, predictionParameters.useFuzzyValues);
            node->setColor(color);
            auto predictedValues = predictor->getConceptHistoryValues(node->getId(), newStep);
            runtimeConceptIt->second->predictedValues = predictedValues;
            if (originalFcm) {
                auto originalConceptIt = originalFcm->concepts.find(node->getId());
                if (originalConceptIt != originalFcm->concepts.end()) {
                    originalConceptIt->second->predictedValues = predictedValues;
                    creationPresenter->setConceptPredictedValues(node->getId());
                }
            }
            updateRuntimeConceptWindow(node->getId());
        }
        for (auto* edge : edges) {
            if (!edge) {
                Logger::warn("Simulation edge missing");
                continue;
            }
            auto runtimeWeightIt = runtimeFcm->weights.find(edge->getId());
            if (runtimeWeightIt == runtimeFcm->weights.end()) {
                Logger::warn("Simulation weight missing");
                continue;
            }
            auto newWeightIt = newFCM.weights.find(edge->getId());
            if (newWeightIt == newFCM.weights.end()) {
                Logger::warn("Simulation weight missing");
                continue;
            }
            double value = predictionParameters.useFuzzyValues ? newWeightIt->second.triangularFuzzyValue.defuzzify() : newWeightIt->second.value;
            auto color = colorValueAdapter.getColor(value, -1, 1, false, predictionParameters.useFuzzyValues);
            edge->setColor(color);
            auto predictedValues = predictor->getWeightHistoryValues(edge->getId(), newStep);
            runtimeWeightIt->second->predictedValues = predictedValues;
            if (originalFcm) {
                auto originalWeightIt = originalFcm->weights.find(edge->getId());
                if (originalWeightIt != originalFcm->weights.end()) {
                    originalWeightIt->second->predictedValues = predictedValues;
                    creationPresenter->setWeightPredictedValues(edge->getId());
                }
            }
            updateRuntimeWeightWindow(edge->getId());
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

void SimulationScenePresenter::stopTimer(QTimer* t) {
    if (t) {
        t->stop();
        t->disconnect(this);
        delete t;
    }
}

bool SimulationScenePresenter::moveStep(int delta) {
    int newStep = step + delta;
    if (newStep < 0) {
        return false;
    }
    if (!goToStep(newStep)) {
        return false;
    }
    return true;
}

void SimulationScenePresenter::speedUp() {
    iterationTime = static_cast<int>(iterationTime / speedUpFactor);
    if (timer && timer->isActive()) {
        timer->start(iterationTime);
    }
}

void SimulationScenePresenter::slowDown() {
    iterationTime = static_cast<int>(iterationTime * slowDownFactor);
    if (timer && timer->isActive()) {
        timer->start(iterationTime);
    }
}

void SimulationScenePresenter::reset() {
    stopExecution();
    active = false;
    step = 0;
    lastStep = 0;
    nodes.clear();
    edges.clear();

    if (runtimeFcm) {
        for (const auto& [id, _] : runtimeFcm->concepts) {
            runtimeFcm->concepts[id]->predictedValues = {};
            updateRuntimeConceptWindow(id);
        }
        for (const auto& [id, _] : runtimeFcm->weights) {
            runtimeFcm->weights[id]->predictedValues = {};
            updateRuntimeWeightWindow(id);
        }
    }
    if (originalFcm) {
        for (const auto& [id, _] : originalFcm->concepts) {
            originalFcm->concepts[id]->predictedValues = {};
            creationPresenter->setConceptPredictedValues(id);
        }
        for (const auto& [id, _] : originalFcm->weights) {
            originalFcm->weights[id]->predictedValues = {};
            creationPresenter->setWeightPredictedValues(id);
        }
    }
    closeRuntimeWindows();
}

void SimulationScenePresenter::stopExecution() {
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

void SimulationScenePresenter::closeRuntimeWindows() {
    std::vector<ConceptWindow*> conceptWindowsToDelete;
    conceptWindowsToDelete.reserve(runtimeConceptWindows.size());
    for (const auto& [_, window] : runtimeConceptWindows) {
        conceptWindowsToDelete.push_back(window);
    }

    std::vector<WeightWindow*> weightWindowsToDelete;
    weightWindowsToDelete.reserve(runtimeWeightWindows.size());
    for (const auto& [_, window] : runtimeWeightWindows) {
        weightWindowsToDelete.push_back(window);
    }

    runtimeConceptWindows.clear();
    runtimeWeightWindows.clear();

    for (auto* window : conceptWindowsToDelete) {
        delete window;
    }
    for (auto* window : weightWindowsToDelete) {
        delete window;
    }
}

void SimulationScenePresenter::openRuntimeConceptWindow(QUuid id, ElementWindowMode mode) {
    if (!runtimeFcm) {
        return;
    }

    auto conceptIt = runtimeFcm->concepts.find(id);
    if (conceptIt == runtimeFcm->concepts.end()) {
        return;
    }

    if (runtimeConceptWindows.find(id) != runtimeConceptWindows.end()) {
        runtimeConceptWindows[id]->raise();
        return;
    }

    QWidget* parentWidget = runtimeScene && !runtimeScene->views().isEmpty() ? runtimeScene->views().first() : nullptr;
    auto* conceptWindow = new ConceptWindow(runtimeFcm->terms, conceptIt->second, ElementWindowMode::PredictionResultsDisabled, parentWidget);
    conceptWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(conceptWindow, &QDialog::finished, this, [this, id]() {
        runtimeConceptWindows.erase(id);
    });
    runtimeConceptWindows[id] = conceptWindow;
    conceptWindow->show();
}

void SimulationScenePresenter::openRuntimeWeightWindow(QUuid id, ElementWindowMode mode) {
    if (!runtimeFcm) {
        return;
    }

    auto weightIt = runtimeFcm->weights.find(id);
    if (weightIt == runtimeFcm->weights.end()) {
        return;
    }

    if (runtimeWeightWindows.find(id) != runtimeWeightWindows.end()) {
        runtimeWeightWindows[id]->raise();
        return;
    }

    QWidget* parentWidget = runtimeScene && !runtimeScene->views().isEmpty() ? runtimeScene->views().first() : nullptr;
    auto* weightWindow = new WeightWindow(runtimeFcm->terms, weightIt->second, ElementWindowMode::PredictionResultsDisabled, parentWidget);
    weightWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(weightWindow, &QDialog::finished, this, [this, id]() {
        runtimeWeightWindows.erase(id);
    });
    runtimeWeightWindows[id] = weightWindow;
    weightWindow->show();
}

void SimulationScenePresenter::updateRuntimeConceptWindow(QUuid id) {
    auto it = runtimeConceptWindows.find(id);
    if (it == runtimeConceptWindows.end()) {
        return;
    }
    it->second->setPredictedValues();
}

void SimulationScenePresenter::updateRuntimeWeightWindow(QUuid id) {
    auto it = runtimeWeightWindows.find(id);
    if (it == runtimeWeightWindows.end()) {
        return;
    }
    it->second->setPredictedValues();
}

