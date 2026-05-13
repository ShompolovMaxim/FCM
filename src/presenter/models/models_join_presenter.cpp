#include "models_join_presenter.h"

#include "common/logger.h"

#include "model/entities/templates/templates_language_manager.h"
#include "model/join/models_joiner.h"

#include "repository/models_manager.h"
#include "repository/templates_manager.h"

#include "ui/join_window/join_window.h"

#include <QCoreApplication>
#include <QMessageBox>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

ModelsJoinPresenter::ModelsJoinPresenter(
    std::vector<std::shared_ptr<FCM>>& fcms,
    std::shared_ptr<TemplatesManager> templatesManager,
    std::shared_ptr<ModelsSavingManager> savingManager,
    QSettings& settings,
    QWidget* parentWidget,
    QObject *parent
) : fcms(fcms),
    templatesManager(templatesManager),
    savingManager(savingManager),
    parentWidget(parentWidget),
    settings(settings),
    QObject{parent} {}

void ModelsJoinPresenter::joinModels() {
    QList<QString> unsavedModelsNames;
    unsavedModelsNames.reserve(fcms.size());
    for (const auto& model : fcms) {
        if (model->dbId == -1) {
            unsavedModelsNames.push_back(model->name);
        }
    }
    const auto savedModelsNames = savingManager->getModelsNames();
    const auto templatesNamesWithTypes = templatesManager->getTemplatesNames();
    const auto templatesNames = TemplatesLanguageManager::filterTemplateNamesForCurrentLanguage(
        templatesNamesWithTypes,
        settings
        );

    JoinWindow* joinWindow = new JoinWindow(unsavedModelsNames, savedModelsNames, templatesNames, parentWidget);

    if (joinWindow->exec() != QDialog::Accepted) {
        return;
    }

    std::shared_ptr<FCM> baseFCM;
    std::vector<std::shared_ptr<FCM>> joinFCMs;

    for (const auto& modelName : joinWindow->getModelsToJoin().value(JoinGroupType::Unsaved)) {
        for (auto unsavedFCM : fcms) {
            if (unsavedFCM->name == modelName) {
                joinFCMs.push_back(unsavedFCM);
                if (modelName == joinWindow->getTermsModel()) {
                    baseFCM = unsavedFCM;
                }
                break;
            }
        }
    }

    for (const auto& modelName : joinWindow->getModelsToJoin().value(JoinGroupType::Saved)) {
        auto model = savingManager->getFCM(modelName);
        if (!model) {
            Logger::warn("Join saved model load failed");
            QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Failed to load one of the selected saved models."));
            return;
        }

        auto savedFCM = std::make_shared<FCM>(*model);
        joinFCMs.push_back(savedFCM);
        if (modelName == joinWindow->getTermsModel()) {
            baseFCM = savedFCM;
        }
    }

    const auto termsModel = joinWindow->getTermsModel();
    if (baseFCM == nullptr && templatesNames.contains(termsModel)) {
        auto model = templatesManager->getFCM(termsModel);
        if (!model) {
            Logger::warn("Join template load failed");
            QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Failed to load the selected terms model template."));
            return;
        }

        baseFCM = std::make_shared<FCM>(*model);
    }

    if (baseFCM == nullptr) {
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Please select a valid terms model before proceeding!"));
        return;
    }

    auto joinedFCM = ModelsJoiner().join(baseFCM, joinFCMs, joinWindow->getJoinMode(), joinWindow->getResultName());

    emit addFCMRequested(joinedFCM);
    emit loadFCMRequested(joinedFCM);
}
