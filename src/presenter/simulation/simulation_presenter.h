#pragma once

#include <QObject>
#include <memory>

#include "model/entities/fcm.h"

class QWidget;
class SimulationScenePresenter;
class CreationPresenter;

namespace Ui {
class MainWindow;
}

class SimulationPresenter : public QObject {
    Q_OBJECT
public:
    explicit SimulationPresenter(Ui::MainWindow* ui, std::shared_ptr<FCM>& fcm, std::shared_ptr<CreationPresenter> creationPresenter, QWidget* parentWidget, QObject *parent = nullptr);

    bool isActive() const;
    bool moveStep(int delta);
    void reconfigure();
    void retranslateUi();

public slots:
    void refreshFromModelSetup();
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
    void recreateScenePresenter();

    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<SimulationScenePresenter> simulationScenePresenter;
    std::shared_ptr<FCM>& fcm;

    double predictScale = 1;
    bool paused = false;
    double currentMetricValue = 0;
};
