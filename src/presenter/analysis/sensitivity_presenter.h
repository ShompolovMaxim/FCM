#pragma once

#include <QObject>

#include "model/entities/fcm.h"
#include "model/sensitivity_analysis/parameters.h"
#include "presenter/analysis/sensitivity_scene_presenter.h"

class QWidget;
class CreationPresenter;
class SimulationPresenter;
namespace Ui {
class MainWindow;
}

class SensitivityPresenter : public QObject {
    Q_OBJECT
public:
    explicit SensitivityPresenter(
        Ui::MainWindow* ui,
        std::shared_ptr<FCM>& fcm,
        std::shared_ptr<CreationPresenter> creationPresenter,
        QWidget* parentWidget,
        QObject *parent = nullptr
    );

    SensitivityAnalysisParameters getSensitivityParameters();
    bool isActive() const;
    void reconfigure();
    void retranslateUi();

public slots:
    void analize();
    void changeActivationFunctionSensitivity(int index);
    void showSensitivityPlot();
    void updateSensitivityProgress(double progress);
    void resetSensitivity();
    void updateSensitivityScaleLabel(double newScale);

signals:

private:
    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<SensitivityScenePresenter> sensitivityScenePresenter;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<FCM>& fcm;
    double sensitivityScale = 1;
};
