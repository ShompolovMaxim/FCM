#pragma once

#include <QObject>
#include <memory>

#include "model/entities/fcm.h"

class QWidget;
class ModelSetupPresenter;
class SimulationScenePresenter;

namespace Ui {
class MainWindow;
}

class SimulationPresenter : public QObject {
    Q_OBJECT
public:
    explicit SimulationPresenter(Ui::MainWindow* ui, std::shared_ptr<FCM>& fcm, std::shared_ptr<ModelSetupPresenter> modelSetupPresenter, std::shared_ptr<SimulationScenePresenter> simulationScenePresenter, QWidget* parentWidget, QObject *parent = nullptr);

    bool isActive() const;
    void retranslateUi();

public slots:
    void changeActivationFunction(int index);
    void updatePredictScaleLabel(double newScale);
    Experiment createExperiment();
    void addExperiment(const Experiment& experiment);
    void simulationFinished();
    void predict();
    void resetPredictionScene();
    void pauseResumePrediction();
    void speedUp();
    void slowDown();
    void stepForward();
    void stepBack();
    void finishSimulation();
    void updateProgress(size_t value, size_t maxStep, double metricValue);
    void onPredictToStaticChanged(bool checked);
    void loadExperiment();
    void onDeleteExperiment();

signals:
    void autosave();
    void loadFCMRequested(std::shared_ptr<FCM> fcm);

private:
    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter;
    std::shared_ptr<SimulationScenePresenter> simulationScenePresenter;
    std::shared_ptr<FCM>& fcm;

    double predictScale = 1;
    bool paused = false;
    double currentMetricValue = 0;
};
