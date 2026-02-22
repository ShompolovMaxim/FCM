#pragma once

#include "prediction_parameters.h"
#include "simulation_parameters.h"
#include "ui/graph_editor/node_item.h"

class SimulationPresenter : public QObject
{
    Q_OBJECT
public:
    SimulationPresenter(QObject* parent = nullptr);
    ~SimulationPresenter();

    void simulate(PredictionParameters predictionParameters, SimulationParameters simulationParameters, QList<NodeItem*> nodes);

signals:
    void finished();
private:
    std::thread workerThread;
    QTimer* timer = new QTimer(this);
    int step = 0;
    QList<double> newNodesValues;
};
