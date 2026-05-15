#pragma once

#include <QObject>
#include <memory>
#include <vector>

#include "common/logger.h"
#include "model/entities/fcm.h"
#include "ui_main_window.h"

class FCM;
class ModelsSavingManager;
class CreationPresenter;
class ModelSetupPresenter;
class SensitivityPresenter;
class SimulationPresenter;
class StaticAnalysisPresenter;
class QCloseEvent;
class QEvent;

namespace Ui {
class MainWindow;
}

class ModelsSwitchingPresenter : public QObject {
    Q_OBJECT
public:
    ModelsSwitchingPresenter(
        Ui::MainWindow* ui,
        QWidget* parentWidget,
        std::shared_ptr<CreationPresenter>& creationPresenter,
        std::shared_ptr<ModelSetupPresenter>& modelSetupPresenter,
        std::shared_ptr<SimulationPresenter>& simulationPresenter,
        std::shared_ptr<SensitivityPresenter>& sensitivityPresenter,
        StaticAnalysisPresenter*& staticAnalysisPresenter,
        std::shared_ptr<ModelsSavingManager> savingManager,
        QObject *parent = nullptr
    );

    std::shared_ptr<FCM> currentModel() const { return fcm; }
    std::shared_ptr<FCM>& currentModelRef() { return fcm; }
    const std::vector<std::shared_ptr<FCM>>& models() const { return fcms; }
    std::vector<std::shared_ptr<FCM>>& modelsRef() { return fcms; }

    void nameChanged();
    void createNewModel();

    bool modelHasUnsavedChanges(std::shared_ptr<FCM> model);
    bool closeModel(size_t index);
    void closeOtherModels(size_t index);
    void rebuildModelsMenu();
    void addFCM(std::shared_ptr<FCM> fcm);
    void closeEvent(QCloseEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);
    void loadFCM(std::shared_ptr<FCM> fcm);
    void restoreCurrentModelName(const QString &name);
    void setCurrentModel(std::shared_ptr<FCM> fcm);

signals:
    void autosaveRequested();

private:
    void recreateScenes();
    void resetCommonUiState();

    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<FCM> fcm;
    std::vector<std::shared_ptr<FCM>> fcms;
    size_t currentModelIdx = 0;

    std::shared_ptr<CreationPresenter>& creationPresenter;
    std::shared_ptr<ModelSetupPresenter>& modelSetupPresenter;
    std::shared_ptr<SimulationPresenter>& simulationPresenter;
    std::shared_ptr<SensitivityPresenter>& sensitivityPresenter;
    StaticAnalysisPresenter*& staticAnalysisPresenter;
    std::shared_ptr<ModelsSavingManager> savingManager;
};

