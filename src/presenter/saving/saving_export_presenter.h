#pragma once

#include <QObject>

#include <memory>
#include <vector>

#include "model/entities/fcm.h"
class QWidget;
class ModelSetupPresenter;
class TemplatesManager;
class ModelsSavingManager;

namespace Ui {
class MainWindow;
}

class SavingExportPresenter : public QObject {
    Q_OBJECT
public:
    SavingExportPresenter(
        Ui::MainWindow* ui,
        std::vector<std::shared_ptr<FCM>>& fcms,
        std::shared_ptr<ModelSetupPresenter> modelSetupPresenter,
        std::shared_ptr<TemplatesManager> templatesManager,
        std::shared_ptr<ModelsSavingManager> savingManager,
        QWidget* parentWidget,
        QObject *parent = nullptr
    );

    void saveAs();
    void save();
    void open();
    void autosaveChange(bool flag);
    void autosave();
    void saveAsTemplate();
    void openTemplate();
    void onExportPng();
    void onExportJson();
    void onImportJson();

signals:
    void modelDeletionFinished(const QString &modelName, bool success);
    void addFCMRequested(std::shared_ptr<FCM> fcm);
    void loadFCMRequested(std::shared_ptr<FCM> fcm);
    void currentModelNameRestoreRequested(const QString &name);

private:
    std::shared_ptr<FCM> currentModel() const;

    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::vector<std::shared_ptr<FCM>>& fcms;
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter;

    std::shared_ptr<TemplatesManager> templatesManager;
    std::shared_ptr<ModelsSavingManager> savingManager;

    void deleteSavedModel(const QString &modelName);
    void deleteSavedTemplate(const QString &templateName);
};

