#include "sensitivity_presenter.h"

#include "ui/graph_editor/color_value_adapter/color_value_adapter.h"

#include <algorithm>

SensitivityPresenter::SensitivityPresenter(GraphScene* graphScene, QCustomPlot* plot, std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent)
    : graphScene(graphScene), plot(plot), creationPresenter(creationPresenter), QObject(parent) {}

SensitivityPresenter::~SensitivityPresenter() {
    stopExecution();
}

void SensitivityPresenter::analize(PredictionParameters predictionParameters, SensitivityAnalysisParameters parameters, std::shared_ptr<FCM> fcm) {
    stopExecution();

    this->fcm = fcm;
    analizer = std::make_shared<SensitivityAnalizer>(parameters, predictionParameters);

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

    auto analizerForThread = analizer;
    workerThread = std::thread([analizerForThread, calculationFCM]() {
        analizerForThread->analize(calculationFCM);
    });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, predictionParameters, parameters, fcm]() {
        if (!analizer) {
            return;
        }

        auto finished = analizer->finished();

        QColor red = QColor(255, 0, 0);
        double maxSensitivity = 0.0;
        if (parameters.changeConcepts) {
            for (auto& [id, concept] : fcm->concepts) {
                if (!analizer->getConceptFinished(id)) {
                    continue;
                }
                maxSensitivity = std::max(analizer->getConceptSensitivity(id), maxSensitivity);
            }
        }
        if (parameters.changeWeights) {
            for (auto& [id, weight] : fcm->weights) {
                if (!analizer->getWeightFinished(id)) {
                    continue;
                }
                maxSensitivity = std::max(analizer->getWeightSensitivity(id), maxSensitivity);
            }
        }

        if (parameters.changeConcepts) {
            for (auto& [id, concept] : fcm->concepts) {
                if (!analizer->getConceptFinished(id)) {
                    continue;
                }
                graphScene->setConceptColor(id, maxSensitivity ? ColorValueAdapter().getColor(std::min(analizer->getConceptSensitivity(id), 1.0), 0, 1) : red, false);
                concept->sensitivity = analizer->getConceptChangeSensitivity(id);
                creationPresenter->setConceptPredictedValues(id);
            }
        }
        if (parameters.changeWeights) {
            for (auto& [id, weight] : fcm->weights) {
                if (!analizer->getWeightFinished(id)) {
                    continue;
                }
                graphScene->setWeightColor(id, maxSensitivity ? ColorValueAdapter().getColor(std::min(analizer->getWeightSensitivity(id) / maxSensitivity, 1.0), 0, 1) : red);
                weight->sensitivity = analizer->getWeightChangeSensitivity(id);
                creationPresenter->setWeightPredictedValues(id);
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

    for (const auto& [id, _] : fcm->concepts) {
        fcm->concepts[id]->sensitivity = {};
        creationPresenter->setConceptPredictedValues(id);
    }
    for (const auto& [id, _] : fcm->weights) {
        fcm->weights[id]->sensitivity = {};
        creationPresenter->setWeightPredictedValues(id);
    }
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


