#pragma once

#include <QObject>
#include <memory>
#include <vector>

#include "common/logger.h"
#include "model/entities/fcm.h"
#include "ui_main_window.h"

class FCM;
class TemplatesManager;
class SavingManager;
class ModelSetupPresenter;
class QSettings;

namespace Ui {
class MainWindow;
}

class ModelsSwitchingPresenter : public QObject {
    Q_OBJECT
public:
    ModelsSwitchingPresenter(
        Ui::MainWindow* ui,
        QWidget* parentWidget,
        std::shared_ptr<FCM>& fcm,
        std::vector<std::shared_ptr<FCM>>& fcms,
        std::shared_ptr<ModelSetupPresenter>& modelSetupPresenter,
        std::shared_ptr<TemplatesManager> templatesManager,
        std::shared_ptr<SavingManager> savingManager,
        QSettings& settings,
        QObject *parent = nullptr
    );

    void nameChanged(QString newName);
    void createNewModel();
    void switchModel();

    void joinModels();

    bool modelHasUnsavedChanges(std::shared_ptr<FCM> model);
    bool closeModel(size_t index);
    void closeOtherModels(size_t index);
    void rebuildModelsMenu();
    void addFCM(std::shared_ptr<FCM> fcm);
    void setCurrentModel(std::shared_ptr<FCM> fcm);

signals:
    void autosaveRequested();
    void loadFCMRequested(std::shared_ptr<FCM> fcm);

private:
    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<FCM>& fcm;
    std::vector<std::shared_ptr<FCM>>& fcms;
    size_t currentModelIdx = 0;

    std::shared_ptr<ModelSetupPresenter>& modelSetupPresenter;
    std::shared_ptr<TemplatesManager> templatesManager;
    std::shared_ptr<SavingManager> savingManager;
    QSettings& settings;
};

