#include "saving_export_presenter.h"
#include "ui_main_window.h"

#include "common/logger.h"
#include "presenter/models/model_setup_presenter.h"

#include "model/entities/templates/templates_language_manager.h"

#include "repository/models_manager.h"
#include "repository/templates_manager.h"
#include "repository/json_repository.h"

#include "ui/save_as_window/save_as_window.h"
#include "ui/load_model_window/load_model_window.h"

#include <QCoreApplication>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

SavingExportPresenter::SavingExportPresenter(
    Ui::MainWindow* ui,
    std::vector<std::shared_ptr<FCM>>& fcms,
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter,
    std::shared_ptr<TemplatesManager> templatesManager,
    std::shared_ptr<ModelsSavingManager> savingManager,
    QWidget* parentWidget,
    QObject *parent
    ) : ui(ui),
        parentWidget(parentWidget),
        fcms(fcms),
        modelSetupPresenter(modelSetupPresenter),
        templatesManager(templatesManager),
        savingManager(savingManager),
        QObject(parent) {
    connect(ui->actionSaveAs, &QAction::triggered, this, &SavingExportPresenter::saveAs);
    connect(ui->actionSave, &QAction::triggered, this, &SavingExportPresenter::save);
    connect(ui->actionOpen, &QAction::triggered, this, &SavingExportPresenter::open);
    connect(ui->actionAutoSave, &QAction::toggled, this, &SavingExportPresenter::autosaveChange);
    connect(ui->actionSaveAsTemplate, &QAction::triggered, this, &SavingExportPresenter::saveAsTemplate);
    connect(ui->actionOpenTemplate, &QAction::triggered, this, &SavingExportPresenter::openTemplate);
    connect(ui->actionExportPNG, &QAction::triggered, this, &SavingExportPresenter::onExportPng);
    connect(ui->actionExportJSON, &QAction::triggered, this, &SavingExportPresenter::onExportJson);
    connect(ui->actionImport, &QAction::triggered, this, &SavingExportPresenter::onImportJson);
}

std::shared_ptr<FCM> SavingExportPresenter::currentModel() const {
    return modelSetupPresenter ? modelSetupPresenter->currentModel() : nullptr;
}

void SavingExportPresenter::saveAs() {
    modelSetupPresenter->updateFCM();
    auto fcm = currentModel();
    if (!fcm) {
        return;
    }

    const auto modelsNames = savingManager->getModelsNames();
    SaveAsWindow saveAsWindow(modelsNames, modelsNames, fcm->name, mainWindowTr("Save FCM"), parentWidget);

    if (saveAsWindow.exec() == QDialog::Accepted) {
        QString newName = saveAsWindow.savingModelName();
        fcm->name = newName;
        savingManager->saveAs(*fcm);
        ui->modelName->setText(newName);
        ui->actionAutoSave->setEnabled(true);
    }
}

void SavingExportPresenter::save() {
    auto fcm = currentModel();
    if (!fcm) {
        return;
    }

    if (fcm->dbId == -1) {
        saveAs();
    } else {
        modelSetupPresenter->updateFCM();
        savingManager->saveFCM(*fcm);
    }
}

void SavingExportPresenter::open() {
    const auto modelsNames = savingManager->getModelsNames();

    LoadModelWindow* loadModelWindow = new LoadModelWindow(modelsNames, mainWindowTr("Open FCM"), parentWidget);
    connect(loadModelWindow, &LoadModelWindow::deleteModelRequested, this, &SavingExportPresenter::deleteSavedModel);
    connect(this, &SavingExportPresenter::modelDeletionFinished, loadModelWindow, &LoadModelWindow::onModelDeleted);

    if (loadModelWindow->exec() != QDialog::Accepted) {
        return;
    }
    QString modelName = loadModelWindow->selectedModelName();
    if (modelName.isEmpty()) {
        return;
    }

    auto model = savingManager->getFCM(modelName);
    if (!model) {
        Logger::warn("Main model load failed");
        return;
    }

    auto newFcm = std::make_shared<FCM>(*model);
    emit addFCMRequested(newFcm);
    emit loadFCMRequested(newFcm);
}

void SavingExportPresenter::autosaveChange(bool flag) {
    auto fcm = currentModel();
    if (!fcm) {
        return;
    }

    fcm->autosaveOn = flag;
    save();
}

void SavingExportPresenter::autosave() {
    auto fcm = currentModel();
    if (!fcm) {
        return;
    }

    if (fcm->autosaveOn) {
        save();
    }
}

void SavingExportPresenter::saveAsTemplate() {
    modelSetupPresenter->updateFCM();
    auto fcm = currentModel();
    if (!fcm) {
        return;
    }

    const auto templatesNamesWithTypes = templatesManager->getTemplatesNames();
    const auto filteredTemplatesNames = TemplatesLanguageManager::filterTemplateNamesForCurrentLanguage(
        templatesNamesWithTypes
    );
    const auto allTemplatesNames = TemplatesLanguageManager::extractTemplateNames(templatesNamesWithTypes);
    SaveAsWindow saveAsWindow(
        filteredTemplatesNames,
        allTemplatesNames,
        fcm->name,
        mainWindowTr("Save FCM Template"),
        parentWidget
        );

    if (saveAsWindow.exec() == QDialog::Accepted) {
        fcm->name = saveAsWindow.savingModelName();
        if (!templatesManager->createTemplate(*fcm)) {
            Logger::warn("Main template save failed");
        }
        ui->modelName->setText(fcm->name);
    }
}

void SavingExportPresenter::openTemplate() {
    const auto templatesNamesWithTypes = templatesManager->getTemplatesNames();
    const auto templatesNames = TemplatesLanguageManager::filterTemplateNamesForCurrentLanguage(
        templatesNamesWithTypes
    );
    LoadModelWindow* loadModelWindow = new LoadModelWindow(templatesNames, mainWindowTr("Open FCM Template"), parentWidget);
    connect(loadModelWindow, &LoadModelWindow::deleteModelRequested, this, &SavingExportPresenter::deleteSavedTemplate);
    connect(this, &SavingExportPresenter::modelDeletionFinished, loadModelWindow, &LoadModelWindow::onModelDeleted);

    if (loadModelWindow->exec() != QDialog::Accepted) {
        return;
    }
    QString templateName = loadModelWindow->selectedModelName();
    if (templateName.isEmpty()) {
        return;
    }

    auto model = templatesManager->getFCM(templateName);
    if (!model) {
        Logger::warn("Main template load failed");
        return;
    }

    auto newFcm = std::make_shared<FCM>(*model);
    emit addFCMRequested(newFcm);
    emit loadFCMRequested(newFcm);
}

void SavingExportPresenter::onExportPng()
{
    QString proposedName = "fcm.png";
    if (!ui->modelName->text().isEmpty()) {
        proposedName = ui->modelName->text() + ".png";
    }
    QString fileName = QFileDialog::getSaveFileName(
        parentWidget,
        mainWindowTr("Export as PNG"),
        proposedName,
        "PNG Images (*.png)"
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
        fileName += ".png";

    QPixmap pixmap = ui->graphicsViewGraph->grab();

    if (!pixmap.save(fileName, "PNG")) {
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Unable to save PNG to the selected file!"));
    }
}

void SavingExportPresenter::onExportJson() {
    QString proposedName = "fcm.json";
    if (!ui->modelName->text().isEmpty()) {
        proposedName = ui->modelName->text() + ".json";
    }
    QString fileName = QFileDialog::getSaveFileName(
        parentWidget,
        mainWindowTr("Save FCM Model"),
        proposedName,
        "JSON files (*.json)"
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".json"))
        fileName += ".json";

    modelSetupPresenter->updateFCM();
    auto fcm = currentModel();
    if (!fcm) {
        return;
    }

    if (!JsonRepository::exportToJson(*fcm, fileName)) {
        Logger::warn("Main json export failed");
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Failed to save file."));
    }
}

void SavingExportPresenter::onImportJson() {
    QString fileName = QFileDialog::getOpenFileName(
        parentWidget,
        mainWindowTr("Open FCM Model"),
        "",
        "JSON files (*.json)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    auto model = JsonRepository::importFromJson(fileName);

    if (!model) {
        Logger::warn("Main json import failed");
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Failed to load file."));
        return;
    }

    auto newFcm = std::make_shared<FCM>(*model);
    emit addFCMRequested(newFcm);
    emit loadFCMRequested(newFcm);
}

void SavingExportPresenter::deleteSavedModel(const QString &modelName) {
    auto currentFcm = currentModel();
    auto model = savingManager->getFCM(modelName);
    if (!model || !savingManager->deleteFCM(model->dbId)) {
        Logger::warn("Main model delete failed");
        emit modelDeletionFinished(modelName, false);
        return;
    }

    for (const auto& openModel : fcms) {
        if (openModel->dbId == model->dbId) {
            openModel->dbId = -1;
            openModel->autosaveOn = false;
        }
    }

    if (currentFcm && currentFcm->name == modelName) {
        ui->actionAutoSave->setEnabled(false);
        ui->actionAutoSave->setChecked(false);
    }

    emit modelDeletionFinished(modelName, true);
}

void SavingExportPresenter::deleteSavedTemplate(const QString &templateName) {
    if (!templatesManager->deleteTemplate(templateName)) {
        Logger::warn("Main template delete failed");
        emit modelDeletionFinished(templateName, false);
        return;
    }

    emit modelDeletionFinished(templateName, true);
}
