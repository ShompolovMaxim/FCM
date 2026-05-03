#pragma once

#include "creation_presenter.h"
#include "prediction_parameters.h"
#include "simulation_parameters.h"

#include "ui/graph_editor/node_item.h"

#include "model/predictor.h"

class SimulationPresenter : public QObject
{
    Q_OBJECT
public:
    SimulationPresenter(std::shared_ptr<CreationPresenter> creationPresenter, QObject* parent = nullptr);
    ~SimulationPresenter();

    void simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes, QMap<QUuid, EdgeItem*> edges, std::shared_ptr<FCM> fcm);

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
    std::shared_ptr<FCM> fcm;
    PredictionParameters predictionParameters;
};
