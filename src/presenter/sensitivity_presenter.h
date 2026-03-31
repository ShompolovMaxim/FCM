#pragma once

#include "libs/qcustomplot/qcustomplot.h"

#include "prediction_parameters.h"

#include "model/entities/fcm.h"
#include "model/sensitivity_analysis/parameters.h"
#include "model/sensitivity_analysis/sensitivity_analizer.h"

#include "ui/graph_editor/graph_scene.h"

#include <QObject>

class SensitivityPresenter : public QObject {
    Q_OBJECT
public:
    SensitivityPresenter(GraphScene* graphScene, QCustomPlot* plot, std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent);
    ~SensitivityPresenter();

    void analize(PredictionParameters predictionParameters, SensitivityAnalysisParameters parameters, std::shared_ptr<FCM> fcm);

signals:
    void updateProgress(double progress);

private:
    GraphScene* graphScene;
    QCustomPlot* plot;
    std::shared_ptr<CreationPresenter> creationPresenter;
    QTimer* timer = new QTimer(this);
    std::thread workerThread;
    std::shared_ptr<SensitivityAnalizer> analizer;
};
