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

    void simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes, QMap<size_t, EdgeItem*> edges, std::shared_ptr<FCM> fcm);

    void pause() { if (timer) timer->stop(); }
    void resume() { if (timer) timer->start(iterationTime); }
    void speedUp();
    void slowDown();
    bool moveStep(int delta);

    double getStepsPerSecond() { return 1000.0 / iterationTime; }

signals:
    void finished();
    void updateProgress(int value);
private:
    bool goToStep(size_t newStep);

    std::thread workerThread;
    QTimer* timer = new QTimer(this);
    int step = 0;
    int lastStep = 0;
    QList<double> newNodesValues;
    size_t iterationTime = 1000;
    double speedUpFactor = 2;
    double slowDownFactor = 2;
    QList<NodeItem*> nodes;
    QMap<size_t, EdgeItem*> edges;
    std::shared_ptr<Predictor> predictor;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<FCM> fcm;
};
