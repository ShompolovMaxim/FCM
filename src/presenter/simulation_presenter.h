#pragma once

#include "creation_presenter.h"
#include "prediction_parameters.h"
#include "scene_presenter.h"
#include "simulation_parameters.h"

#include "ui/graph_editor/node_item.h"

#include "model/predictor.h"

class GraphScene;
class ConceptWindow;
class WeightWindow;

class SimulationPresenter : public ScenePresenter
{
    Q_OBJECT
public:
    SimulationPresenter(std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent = nullptr);
    ~SimulationPresenter();

    void setRuntimeContext(std::shared_ptr<FCM> originalFcm, std::shared_ptr<FCM> runtimeFcm, GraphScene* runtimeScene);
    void simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes, QMap<QUuid, EdgeItem*> edges);

    void createConcept(const QPointF pos) override;
    void createWeight(QUuid nodeId) override;
    void createWeight(QUuid fromNodeId, QUuid toNodeId) override;
    void updateConcept(QUuid id, ElementWindowMode mode) override;
    void updateWeight(QUuid id, ElementWindowMode mode) override;
    void emitAutosave() override;

    void pause() { if (timer) timer->stop(); }
    void resume() { if (timer) timer->start(iterationTime); }
    void speedUp();
    void slowDown();
    bool moveStep(int delta);
    void reset();
    void finish();

    double getStepsPerSecond() { return 1000.0 / iterationTime; }

signals:
    void finished();
    void updateProgress(size_t step, size_t maxStep, double metricValue);
private:
    bool goToStep(size_t newStep);
    void stopExecution();
    void stopTimer(QTimer* t);
    void closeRuntimeWindows();
    void openRuntimeConceptWindow(QUuid id, ElementWindowMode mode);
    void openRuntimeWeightWindow(QUuid id, ElementWindowMode mode);
    void updateRuntimeConceptWindow(QUuid id);
    void updateRuntimeWeightWindow(QUuid id);

    std::thread workerThread;
    QTimer* timer = nullptr;
    QTimer* calculationTimer = nullptr;
    int step = 0;
    int lastStep = 0;
    size_t iterationTime = 1000;
    const double speedUpFactor = 2;
    const double slowDownFactor = 2;
    QList<NodeItem*> nodes;
    QMap<QUuid, EdgeItem*> edges;
    std::shared_ptr<Predictor> predictor;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<FCM> originalFcm;
    std::shared_ptr<FCM> runtimeFcm;
    GraphScene* runtimeScene = nullptr;
    std::map<QUuid, ConceptWindow*> runtimeConceptWindows;
    std::map<QUuid, WeightWindow*> runtimeWeightWindows;
    PredictionParameters predictionParameters;
};
