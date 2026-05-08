#pragma once

#include <QObject>

#include <memory>
#include <vector>

#include "model/entities/fcm.h"
#include "model/entities/templates/template_type.h"

class QSettings;
class QWidget;
class ModelSetupPresenter;
class TemplatesManager;
class SavingManager;

namespace Ui {
class MainWindow;
}

class SavingExportPresenter : public QObject {
    Q_OBJECT
public:
    SavingExportPresenter(
        Ui::MainWindow* ui,
        std::shared_ptr<FCM> fcm,
        std::vector<std::shared_ptr<FCM>>& fcms,
        std::shared_ptr<ModelSetupPresenter> modelSetupPresenter,
        QSettings& settings,
        QWidget* parentWidget,
        QObject *parent = nullptr
    );
    void updateFCM(std::shared_ptr<FCM> newFcm, std::shared_ptr<ModelSetupPresenter> newModelSetupPresenter);
    std::optional<FCM> getSavedFCM(const QString& modelName) const;
    QList<QString> getSavedModelsNames() const;
    std::optional<FCM> getTemplateFCM(const QString& templateName) const;
    QList<QPair<QString, TemplateType>> getTemplatesNames() const;

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

private:
    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<FCM> fcm;
    std::vector<std::shared_ptr<FCM>>& fcms;
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter;
    QSettings& settings;

    std::shared_ptr<TemplatesManager> templatesManager;
    std::shared_ptr<SavingManager> savingManager;

    void deleteSavedModel(const QString &modelName);
    void deleteSavedTemplate(const QString &templateName);
};

