#include "sensitivity_presenter.h"

#include "creation_presenter.h"
#include "ui/concept_window/concept_window.h"
#include "ui/graph_editor/color_value_adapter/color_value_adapter.h"
#include "ui/weight_window/weight_window.h"

#include <algorithm>

SensitivityPresenter::SensitivityPresenter(QCustomPlot* plot, std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent)
    : ScenePresenter(parent), graphScene(nullptr), plot(plot), creationPresenter(creationPresenter) {}

SensitivityPresenter::~SensitivityPresenter() {
    stopExecution();
    closeRuntimeWindows();
}

void SensitivityPresenter::setRuntimeContext(std::shared_ptr<FCM> originalFcm_, std::shared_ptr<FCM> runtimeFcm_, GraphScene* runtimeScene) {
    closeRuntimeWindows();
    originalFcm = originalFcm_;
    runtimeFcm = runtimeFcm_;
    graphScene = runtimeScene;
}

void SensitivityPresenter::createConcept(const QPointF) {}

void SensitivityPresenter::createWeight(QUuid) {}

void SensitivityPresenter::createWeight(QUuid, QUuid) {}

void SensitivityPresenter::updateConcept(QUuid id, ElementWindowMode mode) {
    if (originalFcm && originalFcm->concepts.find(id) != originalFcm->concepts.end()) {
        creationPresenter->updateConcept(id, mode);
        return;
    }
    openRuntimeConceptWindow(id, mode);
}

void SensitivityPresenter::updateWeight(QUuid id, ElementWindowMode mode) {
    if (originalFcm && originalFcm->weights.find(id) != originalFcm->weights.end()) {
        creationPresenter->updateWeight(id, mode);
        return;
    }
    openRuntimeWeightWindow(id, mode);
}

void SensitivityPresenter::emitAutosave() {}

void SensitivityPresenter::analize(PredictionParameters predictionParameters, SensitivityAnalysisParameters parameters) {
    stopExecution();

    if (!runtimeFcm) {
        return;
    }

    analizer = std::make_shared<SensitivityAnalizer>(parameters, predictionParameters);

    CalculationFCM calculationFCM;

    for (const auto& [id, concept] : runtimeFcm->concepts) {
        calculationFCM.concepts[id] = CalculationConcept{
            id,
            concept->term->value,
            concept->term->fuzzyValue,
            concept->startStep
        };
    }
    for (const auto& [id, weight] : runtimeFcm->weights) {
        calculationFCM.weights[id] = CalculationWeight{
            id,
            weight->term->value,
            weight->term->fuzzyValue,
            weight->fromConceptId,
            weight->toConceptId
        };
    }

    auto analizerForThread = analizer;
    workerThread = std::thread([analizerForThread, calculationFCM]() {
        analizerForThread->analize(calculationFCM);
    });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, parameters]() {
        if (!analizer) {
            return;
        }

        auto finished = analizer->finished();

        QColor red = QColor(255, 0, 0);
        double maxSensitivity = 0.0;
        if (parameters.changeConcepts) {
            for (auto& [id, concept] : runtimeFcm->concepts) {
                if (!analizer->getConceptFinished(id)) {
                    continue;
                }
                maxSensitivity = std::max(analizer->getConceptSensitivity(id), maxSensitivity);
            }
        }
        if (parameters.changeWeights) {
            for (auto& [id, weight] : runtimeFcm->weights) {
                if (!analizer->getWeightFinished(id)) {
                    continue;
                }
                maxSensitivity = std::max(analizer->getWeightSensitivity(id), maxSensitivity);
            }
        }

        if (parameters.changeConcepts) {
            for (auto& [id, concept] : runtimeFcm->concepts) {
                if (!analizer->getConceptFinished(id)) {
                    continue;
                }
                graphScene->setConceptColor(id, maxSensitivity ? ColorValueAdapter().getColor(std::min(analizer->getConceptSensitivity(id), 1.0) / maxSensitivity, 0, 1) : red, false);
                concept->sensitivity = analizer->getConceptChangeSensitivity(id);
                if (originalFcm) {
                    auto originalConceptIt = originalFcm->concepts.find(id);
                    if (originalConceptIt != originalFcm->concepts.end()) {
                        originalConceptIt->second->sensitivity = concept->sensitivity;
                        creationPresenter->setConceptPredictedValues(id);
                    }
                }
                updateRuntimeConceptWindow(id);
            }
        }
        if (parameters.changeWeights) {
            for (auto& [id, weight] : runtimeFcm->weights) {
                if (!analizer->getWeightFinished(id)) {
                    continue;
                }
                graphScene->setWeightColor(id, maxSensitivity ? ColorValueAdapter().getColor(std::min(analizer->getWeightSensitivity(id) / maxSensitivity, 1.0), 0, 1) : red);
                weight->sensitivity = analizer->getWeightChangeSensitivity(id);
                if (originalFcm) {
                    auto originalWeightIt = originalFcm->weights.find(id);
                    if (originalWeightIt != originalFcm->weights.end()) {
                        originalWeightIt->second->sensitivity = weight->sensitivity;
                        creationPresenter->setWeightPredictedValues(id);
                    }
                }
                updateRuntimeWeightWindow(id);
            }
        }
        if (analizer->getFcmFinished()) {
            QVector<double> x;
            QVector<double> y;
            for (const auto& [change, sensitivity] : analizer->getFcmSensitivity()) {
                x.push_back(change);
                y.push_back(sensitivity);
            }
            plot->graph(0)->setData(x, y);
            plot->graph(0)->rescaleKeyAxis();
            plot->graph(0)->rescaleValueAxis();
            plot->replot();
        }
        updateProgress(analizer->getProgress());
        if (finished) {
            timer->stop();
            return;
        }
    });

    timer->start(100);
}

void SensitivityPresenter::reset() {
    stopExecution();

    if (runtimeFcm) {
        for (const auto& [id, _] : runtimeFcm->concepts) {
            runtimeFcm->concepts[id]->sensitivity = {};
            updateRuntimeConceptWindow(id);
        }
        for (const auto& [id, _] : runtimeFcm->weights) {
            runtimeFcm->weights[id]->sensitivity = {};
            updateRuntimeWeightWindow(id);
        }
    }
    if (originalFcm) {
        for (const auto& [id, _] : originalFcm->concepts) {
            originalFcm->concepts[id]->sensitivity = {};
            creationPresenter->setConceptPredictedValues(id);
        }
        for (const auto& [id, _] : originalFcm->weights) {
            originalFcm->weights[id]->sensitivity = {};
            creationPresenter->setWeightPredictedValues(id);
        }
    }
    closeRuntimeWindows();
}

void SensitivityPresenter::stopExecution() {
    if (timer) {
        timer->stop();
        timer->disconnect(this);
        delete timer;
        timer = nullptr;
    }

    if (analizer) {
        analizer->requestStop();
    }

    if (workerThread.joinable()) {
        workerThread.join();
    }

    analizer.reset();
}

void SensitivityPresenter::closeRuntimeWindows() {
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

void SensitivityPresenter::openRuntimeConceptWindow(QUuid id, ElementWindowMode mode) {
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

    QWidget* parentWidget = graphScene && !graphScene->views().isEmpty() ? graphScene->views().first() : nullptr;
    auto* conceptWindow = new ConceptWindow(runtimeFcm->terms, conceptIt->second, ElementWindowMode::SensitivityAnalysisDisabled, parentWidget);
    conceptWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(conceptWindow, &QDialog::finished, this, [this, id]() {
        runtimeConceptWindows.erase(id);
    });
    runtimeConceptWindows[id] = conceptWindow;
    conceptWindow->show();
}

void SensitivityPresenter::openRuntimeWeightWindow(QUuid id, ElementWindowMode mode) {
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

    QWidget* parentWidget = graphScene && !graphScene->views().isEmpty() ? graphScene->views().first() : nullptr;
    auto* weightWindow = new WeightWindow(runtimeFcm->terms, weightIt->second, ElementWindowMode::SensitivityAnalysisDisabled, parentWidget);
    weightWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(weightWindow, &QDialog::finished, this, [this, id]() {
        runtimeWeightWindows.erase(id);
    });
    runtimeWeightWindows[id] = weightWindow;
    weightWindow->show();
}

void SensitivityPresenter::updateRuntimeConceptWindow(QUuid id) {
    auto it = runtimeConceptWindows.find(id);
    if (it == runtimeConceptWindows.end()) {
        return;
    }
    it->second->setPredictedValues();
}

void SensitivityPresenter::updateRuntimeWeightWindow(QUuid id) {
    auto it = runtimeWeightWindows.find(id);
    if (it == runtimeWeightWindows.end()) {
        return;
    }
    it->second->setPredictedValues();
}


