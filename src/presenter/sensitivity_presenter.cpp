#include "sensitivity_presenter.h"

#include "ui/graph_editor/color_value_adapter/color_value_adapter.h"

#include <algorithm>

SensitivityPresenter::SensitivityPresenter(GraphScene* graphScene, QCustomPlot* plot, std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent)
    : graphScene(graphScene), plot(plot), creationPresenter(creationPresenter), QObject(parent) {}

SensitivityPresenter::~SensitivityPresenter() {
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void SensitivityPresenter::analize(PredictionParameters predictionParameters, SensitivityAnalysisParameters parameters, std::shared_ptr<FCM> fcm) {
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

    workerThread = std::thread([this, calculationFCM]() {
        analizer->analize(calculationFCM);
    });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, predictionParameters, parameters, fcm]() {
        auto finished = analizer->finished();
        if (parameters.changeConcepts) {
            for (auto& [id, concept] : fcm->concepts) {
                if (!analizer->getConceptFinished(id)) {
                    continue;
                }
                graphScene->setConceptColor(id, ColorValueAdapter().getColor(std::min(analizer->getConceptSensitivity(id), 1.0), 0, 1), false);
                concept->sensitivity = analizer->getConceptChangeSensitivity(id);
                creationPresenter->setConceptPredictedValues(id);
            }
        }
        if (parameters.changeWeights) {
            for (auto& [id, weight] : fcm->weights) {
                if (!analizer->getWeightFinished(id)) {
                    continue;
                }
                graphScene->setWeightColor(id, ColorValueAdapter().getColor(std::min(analizer->getWeightSensitivity(id), 1.0), 0, 1));
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


