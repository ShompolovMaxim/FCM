#pragma once

#include "libs/qcustomplot/qcustomplot.h"

#include "prediction_parameters.h"
#include "scene_presenter.h"

#include "model/entities/fcm.h"
#include "model/sensitivity_analysis/parameters.h"
#include "model/sensitivity_analysis/sensitivity_analizer.h"

#include "ui/graph_editor/graph_scene.h"

class CreationPresenter;
class ConceptWindow;
class WeightWindow;

class SensitivityPresenter : public ScenePresenter {
    Q_OBJECT
public:
    SensitivityPresenter(QCustomPlot* plot, std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent = nullptr);
    ~SensitivityPresenter();

    void setRuntimeContext(std::shared_ptr<FCM> originalFcm, std::shared_ptr<FCM> runtimeFcm, GraphScene* runtimeScene);
    void analize(PredictionParameters predictionParameters, SensitivityAnalysisParameters parameters);
    void reset();

    void createConcept(const QPointF pos) override;
    void createWeight(QUuid nodeId) override;
    void createWeight(QUuid fromNodeId, QUuid toNodeId) override;
    void updateConcept(QUuid id, ElementWindowMode mode) override;
    void updateWeight(QUuid id, ElementWindowMode mode) override;
    void emitAutosave() override;

signals:
    void updateProgress(double progress);

private:
    void stopExecution();
    void closeRuntimeWindows();
    void openRuntimeConceptWindow(QUuid id, ElementWindowMode mode);
    void openRuntimeWeightWindow(QUuid id, ElementWindowMode mode);
    void updateRuntimeConceptWindow(QUuid id);
    void updateRuntimeWeightWindow(QUuid id);

    GraphScene* graphScene;
    QCustomPlot* plot;
    std::shared_ptr<CreationPresenter> creationPresenter;
    QTimer* timer = nullptr;
    std::thread workerThread;
    std::shared_ptr<SensitivityAnalizer> analizer;
    std::shared_ptr<FCM> originalFcm;
    std::shared_ptr<FCM> runtimeFcm;
    std::map<QUuid, ConceptWindow*> runtimeConceptWindows;
    std::map<QUuid, WeightWindow*> runtimeWeightWindows;
};
