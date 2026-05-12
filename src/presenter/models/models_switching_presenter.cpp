#include "models_switching_presenter.h"

#include "presenter/models/creation_presenter.h"
#include "presenter/models/model_setup_presenter.h"
#include "presenter/analysis/sensitivity_presenter.h"
#include "presenter/simulation/simulation_presenter.h"
#include "presenter/analysis/static_analysis_presenter.h"

#include "model/join/models_joiner.h"
#include "model/entities/templates/templates_language_manager.h"

#include "repository/models_manager.h"
#include "repository/templates_manager.h"

#include "ui/graph_editor/graph_scene.h"
#include "ui/graph_editor/graph_view.h"
#include "ui/join_window/join_window.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>

#include <algorithm>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

ModelsSwitchingPresenter::ModelsSwitchingPresenter(
    Ui::MainWindow* ui,
    QWidget* parentWidget,
    std::shared_ptr<FCM>& fcm,
    std::vector<std::shared_ptr<FCM>>& fcms,
    std::shared_ptr<CreationPresenter>& creationPresenter,
    std::shared_ptr<ModelSetupPresenter>& modelSetupPresenter,
    std::shared_ptr<SimulationPresenter>& simulationPresenter,
    std::shared_ptr<SensitivityPresenter>& sensitivityPresenter,
    StaticAnalysisPresenter*& staticAnalysisPresenter,
    std::shared_ptr<TemplatesManager> templatesManager,
    std::shared_ptr<ModelsSavingManager> savingManager,
    QSettings& settings,
    QObject *parent
) : ui(ui),
    parentWidget(parentWidget),
    fcm(fcm),
    fcms(fcms),
    creationPresenter(creationPresenter),
    modelSetupPresenter(modelSetupPresenter),
    simulationPresenter(simulationPresenter),
    sensitivityPresenter(sensitivityPresenter),
    staticAnalysisPresenter(staticAnalysisPresenter),
    templatesManager(templatesManager),
    savingManager(savingManager),
    settings(settings),
    QObject{parent} {
    connect(ui->modelName, &QLineEdit::textChanged, this, &ModelsSwitchingPresenter::nameChanged);
    connect(ui->actionNew, &QAction::triggered, this, &ModelsSwitchingPresenter::createNewModel);
    connect(ui->actionJoinFCM, &QAction::triggered, this, &ModelsSwitchingPresenter::joinModels);
}

bool ModelsSwitchingPresenter::modelHasUnsavedChanges(std::shared_ptr<FCM> model) {
    if (model == fcm) {
        modelSetupPresenter->updateFCM();
    }

    std::optional<FCM> savedModel;
    if (model->dbId != -1) {
        savedModel = savingManager->getFCM(model->name);
    }

    auto defaultFcm = FCM();
    defaultFcm.name = model->name;
    return (!savedModel && *model != defaultFcm) || (savedModel && *model != *savedModel);
}

bool ModelsSwitchingPresenter::closeModel(size_t index) {
    if (index >= fcms.size() || fcms.size() <= 1) {
        return false;
    }

    auto model = fcms[index];
    if (modelHasUnsavedChanges(model)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            parentWidget,
            mainWindowTr("There are unsaved changes!"),
            mainWindowTr("Model \"%1\" has unsaved changes. Are you sure you want to close it?").arg(model->name),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
            );

        if (reply != QMessageBox::Yes) {
            return false;
        }
    }

    fcms.erase(fcms.begin() + static_cast<std::ptrdiff_t>(index));

    if (currentModelIdx == index) {
        if (index >= fcms.size()) {
            currentModelIdx = fcms.size() - 1;
        } else {
            currentModelIdx = index;
        }
        loadFCM(fcms[currentModelIdx]);
    } else {
        if (index < currentModelIdx) {
            --currentModelIdx;
        }
        rebuildModelsMenu();
    }

    return true;
}

void ModelsSwitchingPresenter::closeOtherModels(size_t index) {
    if (index >= fcms.size()) {
        return;
    }

    while (fcms.size() > 1) {
        size_t indexToClose = 0;
        if (indexToClose == index) {
            indexToClose = 1;
        }

        if (!closeModel(indexToClose)) {
            return;
        }

        if (indexToClose < index) {
            --index;
        }
    }

    loadFCM(fcms.front());
}

void ModelsSwitchingPresenter::rebuildModelsMenu() {
    ui->menuModels->clear();

    for (size_t i = 0; i < fcms.size(); ++i) {
        auto* modelMenu = ui->menuModels->addMenu(fcms[i]->name);
        QAction* modelAction = modelMenu->menuAction();
        modelAction->setCheckable(true);
        modelAction->setChecked(i == currentModelIdx);
        modelAction->setData(QVariant::fromValue(static_cast<qulonglong>(i)));

        QAction* closeAction = modelMenu->addAction(mainWindowTr("Close"));
        closeAction->setEnabled(fcms.size() > 1);
        connect(closeAction, &QAction::triggered, this, [this, i]() {
            if (i < fcms.size()) {
                closeModel(i);
            }
        });

        QAction* closeOtherAction = modelMenu->addAction(mainWindowTr("Close other models"));
        closeOtherAction->setEnabled(fcms.size() > 1);
        connect(closeOtherAction, &QAction::triggered, this, [this, i]() {
            if (i < fcms.size()) {
                closeOtherModels(i);
            }
        });
    }
}

void ModelsSwitchingPresenter::addFCM(std::shared_ptr<FCM> newFcm) {
    currentModelIdx = fcms.size();
    fcms.push_back(newFcm);
    rebuildModelsMenu();
}

void ModelsSwitchingPresenter::closeEvent(QCloseEvent* event) {
    for (const auto& model : fcms) {
        if (modelHasUnsavedChanges(model)) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                parentWidget,
                mainWindowTr("There are unsaved changes!"),
                mainWindowTr("There are unsaved changes! Are you sure you want to quit the program?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                event->accept();
            } else {
                event->ignore();
            }
            return;
        }
    }

    event->accept();
}

bool ModelsSwitchingPresenter::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->menuModels && event->type() == QEvent::MouseButtonRelease) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        QAction* action = ui->menuModels->actionAt(mouseEvent->pos());
        if (action && action->menu()) {
            size_t index = static_cast<size_t>(action->data().toULongLong());
            if (index < fcms.size()) {
                loadFCM(fcms[index]);
                ui->menuModels->hide();
                return true;
            }
        }
    }

    return false;
}

void ModelsSwitchingPresenter::recreateScenes() {
    ui->adjacencyTableView->loadFromFCM(fcm);
    ui->adjacencyTableView->setPresenter(creationPresenter);

    modelSetupPresenter->updateModeButtonText(EditMode::Create);

    auto* newScene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    connect(ui->pushButtonMode, &QPushButton::clicked, newScene, &GraphScene::switchMode);
    connect(newScene, &GraphScene::modeChanged, modelSetupPresenter.get(), &ModelSetupPresenter::updateModeButtonText);

    auto* oldSceneCreate = ui->graphicsViewGraph->scene();
    auto* oldScenePredict = ui->graphicsViewPredict->scene();
    auto* oldSceneSensitivity = ui->graphicsViewSensitivity->scene();
    ui->graphicsViewGraph->setScene(newScene);
    ui->graphicsViewPredict->setScene(newScene);
    ui->graphicsViewSensitivity->setScene(newScene);
    if (oldSceneCreate != oldScenePredict) {
        delete oldScenePredict;
    }
    if (oldSceneCreate != oldSceneSensitivity) {
        delete oldSceneSensitivity;
    }
    delete oldSceneCreate;

    auto* oldStaticAnalysisScene = ui->staticAnalysis->findChild<GraphView*>("graphicsView")->scene();
    auto* newStaticAnalysisScene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    newStaticAnalysisScene->blockConceptCreationColorEdit(true);
    newStaticAnalysisScene->setMode(EditMode::EditValues);
    ui->staticAnalysis->findChild<GraphView*>("graphicsView")->setScene(newStaticAnalysisScene);
    delete oldStaticAnalysisScene;
}

void ModelsSwitchingPresenter::resetCommonUiState() {
    ui->graphicsViewGraph->resetTransform();
    ui->graphicsViewPredict->resetTransform();
    ui->graphicsViewSensitivity->resetTransform();
    modelSetupPresenter->updateGraphScaleLabel(1.0);
    simulationPresenter->updatePredictScaleLabel(1.0);
    sensitivityPresenter->updateSensitivityScaleLabel(1.0);
    ui->useFuzzyValuesStatic->setChecked(false);
    ui->influenceDirection->setCurrentIndex(0);
    ui->influenceSteps->setValue(1);
    ui->graphConcept->setCurrentIndex(0);
}

void ModelsSwitchingPresenter::loadFCM(std::shared_ptr<FCM> newFcm) {
    setCurrentModel(newFcm);
    rebuildModelsMenu();
    emit currentModelChanged(fcm);

    creationPresenter->reconfigure(fcm);
    simulationPresenter->reconfigure();
    sensitivityPresenter->reconfigure();
    recreateScenes();
    staticAnalysisPresenter->reconfigure(fcm);
    modelSetupPresenter->reconfigure();
    resetCommonUiState();
}

void ModelsSwitchingPresenter::setCurrentModel(std::shared_ptr<FCM> newFcm) {
    fcm = newFcm;
    auto it = std::find(fcms.begin(), fcms.end(), fcm);
    if (it != fcms.end()) {
        currentModelIdx = static_cast<size_t>(std::distance(fcms.begin(), it));
    }
}

void ModelsSwitchingPresenter::nameChanged(QString newName) {
    fcm->name = newName;
    rebuildModelsMenu();
    emit autosaveRequested();
}

void ModelsSwitchingPresenter::createNewModel() {
    fcm = std::make_shared<FCM>();
    size_t counter = 1;
    QStringList fcmsNames;
    for (const auto& model : fcms) {
        fcmsNames.append(model->name);
    }
    while (fcmsNames.contains(mainWindowTr("New model") + (counter - 1 ? " (" + QString::number(counter) + ")" : ""))) {
        ++counter;
    }
    fcm->name = mainWindowTr("New model") + (counter - 1 ? " (" + QString::number(counter) + ")" : "");
    addFCM(fcm);
    loadFCM(fcm);
}

void ModelsSwitchingPresenter::joinModels() {
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

    addFCM(joinedFCM);
    loadFCM(joinedFCM);
}

