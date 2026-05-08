#include "main_window.h"
#include "ui_main_window.h"

#include "common/logger.h"
#include "presenter/model_setup_presenter.h"
#include "presenter/simulation_presenter.h"
#include "ui/join_window/join_window.h"
#include "ui/graph_editor/graph_scene.h"
#include "ui/save_as_window/save_as_window.h"
#include "ui/load_model_window/load_model_window.h"

#include "repository/json_repository.h"
#include "repository/migration_manager.h"

#include "model/join/models_joiner.h"
#include "model/entities/templates/templates_language_manager.h"

#include <QMouseEvent>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    auto lang = settings.value("language", "").toString();
    translatorRus.load("FCM_ru_RU.qm");
    translatorDefaultRus.load("qtbase_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translatorWidgetsRus.load("qt_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (lang.isEmpty()) {
        ui->actionEnglish->setChecked(true);
    } else {
        ui->actionRussian->setChecked(true);
        qApp->installTranslator(&translatorRus);
        qApp->installTranslator(&translatorDefaultRus);
        qApp->installTranslator(&translatorWidgetsRus);
        ui->retranslateUi(this);
    }
    connect(ui->actionRussian, &QAction::triggered, this, &MainWindow::setRussian);
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::setEnglish);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onCurrentTabChanged);
    ui->comboBoxAlgorithm->setItemData(0, "const weights", Qt::UserRole);
    ui->comboBoxAlgorithm->setItemData(1, "changing weights", Qt::UserRole);
    ui->comboBoxAlgorithmSensitivity->setItemData(0, "const weights", Qt::UserRole);
    ui->comboBoxAlgorithmSensitivity->setItemData(1, "changing weights", Qt::UserRole);
    ui->comboBoxActivation->setItemData(0, "bivalent", Qt::UserRole);
    ui->comboBoxActivation->setItemData(1, "trivalent", Qt::UserRole);
    ui->comboBoxActivation->setItemData(2, "threshold-linear", Qt::UserRole);
    ui->comboBoxActivation->setItemData(3, "sigmoid", Qt::UserRole);
    ui->comboBoxActivation->setItemData(4, "hyperbolic tangent", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(0, "bivalent", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(1, "trivalent", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(2, "threshold-linear", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(3, "sigmoid", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(4, "hyperbolic tangent", Qt::UserRole);
    ui->comboBoxMetric->setItemData(0, "MSE", Qt::UserRole);
    ui->comboBoxMetric->setItemData(1, "MAE", Qt::UserRole);
    ui->comboBoxMetric->setItemData(2, "MAPE", Qt::UserRole);
    ui->comboBoxMetricSensitivity->setItemData(0, "MSE", Qt::UserRole);
    ui->comboBoxMetricSensitivity->setItemData(1, "MAE", Qt::UserRole);
    ui->comboBoxMetricSensitivity->setItemData(2, "MAPE", Qt::UserRole);
    ui->sensitivityMeasureMetric->setItemData(0, "MSE", Qt::UserRole);
    ui->sensitivityMeasureMetric->setItemData(1, "MAE", Qt::UserRole);
    ui->sensitivityMeasureMetric->setItemData(2, "MAPE", Qt::UserRole);
    ui->influenceDirection->setItemData(0, "from", Qt::UserRole);
    ui->influenceDirection->setItemData(1, "on", Qt::UserRole);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("models.db");
    if (!db.open()) {
        Logger::critical("Database open failed");
        qFatal("Cannot open database");
    }
    if (!MigrationManager::migrate(db)) {
        Logger::critical("Database migration failed");
        qFatal("Cannot apply database migrations");
    }
    savingManager = std::make_shared<SavingManager>(ModelsRepository(db));
    templatesManager = std::make_shared<TemplatesManager>(TemplatesRepository(db));

    fcm = std::make_shared<FCM>();
    fcm->name = ui->modelName->text();

    creationPresenter = std::make_shared<CreationPresenter>(fcm, this);
    connect(creationPresenter.get(), &CreationPresenter::autosave, this, &MainWindow::autosave);
    ui->adjacencyTableView->setPresenter(creationPresenter);
    simulationScenePresenter = std::make_shared<SimulationScenePresenter>(creationPresenter, nullptr);

    auto* scene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);
    ui->graphicsViewSensitivity->setScene(scene);

    auto* staticAnalysisScene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    staticAnalysisScene->setMode(EditMode::EditValues);
    staticAnalysisScene->blockConceptCreationColorEdit(true);
    ui->staticAnalysis->findChild<GraphView*>("graphicsView")->setScene(staticAnalysisScene);

    connect(ui->pushButtonMode, &QPushButton::clicked, scene, &GraphScene::switchMode);
    connect(scene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    connect(ui->graphicsViewGraph, &GraphView::scaleChanged, this, &MainWindow::updateGraphScaleLabel);
    connect(ui->graphicsViewSensitivity, &GraphView::scaleChanged, this, &MainWindow::updateSensitivityScaleLabel);
    connect(ui->pushButtonScaleGraph, &QPushButton::clicked, ui->graphicsViewGraph, &GraphView::resetScale);
    connect(ui->pushButtonScalePredict, &QPushButton::clicked, ui->graphicsViewPredict, &GraphView::resetScale);
    connect(ui->pushButtonScaleSensitivity, &QPushButton::clicked, ui->graphicsViewSensitivity, &GraphView::resetScale);

    QStandardItemModel* experimentsModel = new QStandardItemModel();
    experimentsModel->setHorizontalHeaderLabels({tr("Algorithm"), tr("Value type"), tr("Activation function"), tr("Metric"), tr("Predict to static"), tr("Threshold"), tr("Steps less threshold"), tr("Fixed steps"), tr("Timestamp"), "", ""});
    ui->experimantsTable->setModel(experimentsModel);
    ui->experimantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionAutoSave, &QAction::toggled, this, &MainWindow::autosaveChange);
    connect(creationPresenter.get(), &CreationPresenter::autosave, this, &MainWindow::autosave);
    connect(ui->actionSaveAsTemplate, &QAction::triggered, this, &MainWindow::saveAsTemplate);
    connect(ui->actionOpenTemplate, &QAction::triggered, this, &MainWindow::openTemplate);
    connect(ui->actionExportPNG, &QAction::triggered, this, &MainWindow::onExportPng);
    connect(ui->actionExportJSON, &QAction::triggered, this, &MainWindow::onExportJson);
    connect(ui->actionImport, &QAction::triggered, this, &MainWindow::onImportJson);

    ui->fuzzyValuePlot->xAxis->setRange(-1.1, 1.1);
    ui->fuzzyValuePlot->yAxis->setRange(0, 1);
    ui->fuzzyValuePlot->xAxis->setLabel("x");
    ui->fuzzyValuePlot->yAxis->setLabel("μ(x)");
    ui->fuzzyValuePlot->addGraph();

    ui->factorsStatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, creationPresenter, fcm);
    recreatePresenters();

    connect(ui->comboBoxActivationSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changeActivationFunctionSensitivity);
    connect(ui->comboBoxAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxAlgorithmSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->comboBoxAlgorithmSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxAlgorithm, &QComboBox::setCurrentIndex);
    connect(ui->useFuzzyValues, &QCheckBox::toggled, ui->useFuzzyValuesSensitivity, &QCheckBox::setChecked);
    connect(ui->useFuzzyValuesSensitivity, &QCheckBox::toggled, ui->useFuzzyValues, &QCheckBox::setChecked);
    connect(ui->comboBoxActivation, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxActivationSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->comboBoxActivationSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxActivation, &QComboBox::setCurrentIndex);
    connect(ui->fuzzinessDegree, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->fuzzinessDegreeSensitivity, &QDoubleSpinBox::setValue);
    connect(ui->fuzzinessDegreeSensitivity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->fuzzinessDegree, &QDoubleSpinBox::setValue);
    connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, ui->checkBoxPredictToStaticSensitivity, &QCheckBox::setChecked);
    connect(ui->checkBoxPredictToStaticSensitivity, &QCheckBox::toggled, ui->checkBoxPredictToStatic, &QCheckBox::setChecked);
    connect(ui->comboBoxMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxMetricSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->comboBoxMetricSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxMetric, &QComboBox::setCurrentIndex);
    connect(ui->doubleSpinBoxThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->doubleSpinBoxThresholdSensitivity, &QDoubleSpinBox::setValue);
    connect(ui->doubleSpinBoxThresholdSensitivity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->doubleSpinBoxThreshold, &QDoubleSpinBox::setValue);
    connect(ui->spinBoxMetricSteps, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxMetricStepsSensitivity, &QSpinBox::setValue);
    connect(ui->spinBoxMetricStepsSensitivity, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxMetricSteps, &QSpinBox::setValue);
    connect(ui->spinBoxFixedSteps, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxFixedStepsSensitivity, &QSpinBox::setValue);
    connect(ui->spinBoxFixedStepsSensitivity, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxFixedSteps, &QSpinBox::setValue);
    connect(ui->pushButtonAnalizeSensitivity, &QPushButton::clicked, this, &MainWindow::analize);
    connect(ui->pushButtonResetSensitivity, &QPushButton::clicked, this, &MainWindow::resetSensitivity);
    connect(ui->showSensitivityPlot, &QPushButton::clicked, this, &MainWindow::showSensitivityPlot);
    ui->plotSensitivity->addGraph();
    ui->plotSensitivity->yAxis->setRange(-0.1, 1.1);
    ui->plotSensitivity->xAxis->setLabel(tr("max change"));
    ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
    ui->plotSensitivity->setGeometry(ui->graphicsViewSensitivity->geometry());

    connect(ui->actionModelSettings, &QAction::toggled, this, &MainWindow::changeModelSettingsVisibility);
    connect(ui->actionGraph, &QAction::toggled, this, &MainWindow::changeGraphVisibility);
    connect(ui->actionAdjacencyMatrix, &QAction::toggled, this, &MainWindow::changeAdjacencyMatrixVisibility);
    connect(ui->actionStaticAnalysis, &QAction::toggled, this, &MainWindow::changeStaticAnalysisVisibility);
    connect(ui->actionSimulation, &QAction::toggled, this, &MainWindow::changeSimulationVisibility);
    connect(ui->actionExperiments, &QAction::toggled, this, &MainWindow::changeExperimentsVisibility);
    connect(ui->actionSensitivityAnalysis, &QAction::toggled, this, &MainWindow::changeSensitivityAnalysisVisibility);
    ui->tabWidget->setTabVisible(0, settings.value("tabs/modelSettings", true).toBool());
    ui->tabWidget->setTabVisible(1, settings.value("tabs/graph", true).toBool());
    ui->tabWidget->setTabVisible(2, settings.value("tabs/adjMatrix", true).toBool());
    ui->tabWidget->setTabVisible(3, settings.value("tabs/staticAnalysis", true).toBool());
    ui->tabWidget->setTabVisible(4, settings.value("tabs/simulation", true).toBool());
    ui->tabWidget->setTabVisible(5, settings.value("tabs/experiments", true).toBool());
    ui->tabWidget->setTabVisible(6, settings.value("tabs/sensitivity", true).toBool());
    ui->actionModelSettings->setChecked(settings.value("tabs/modelSettings", true).toBool());
    ui->actionGraph->setChecked(settings.value("tabs/graph", true).toBool());
    ui->actionAdjacencyMatrix->setChecked(settings.value("tabs/adjMatrix", true).toBool());
    ui->actionStaticAnalysis->setChecked(settings.value("tabs/staticAnalysis", true).toBool());
    ui->actionSimulation->setChecked(settings.value("tabs/simulation", true).toBool());
    ui->actionExperiments->setChecked(settings.value("tabs/experiments", true).toBool());
    ui->actionSensitivityAnalysis->setChecked(settings.value("tabs/sensitivity", true).toBool());

    qApp->installEventFilter(&toolTipController);
    toolTipController.setEnabled(settings.value("tooltips", true).toBool());
    connect(ui->actionShowTooltips, &QAction::toggled, this, &MainWindow::changeShowTooltips);
    ui->actionShowTooltips->setChecked(settings.value("tooltips", true).toBool());
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::showHelp);

    QObject::connect(ui->modelName, &QLineEdit::textChanged, this, &MainWindow::nameChanged);
    addFCM(fcm);
    ui->modelName->setText(tr("New model"));
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::createNewModel);

    connect(ui->actionJoinFCM, &QAction::triggered, this, &MainWindow::joinModels);
    ui->menuModels->installEventFilter(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape &&
        ui->tabWidget->currentWidget() == ui->graph &&
        creationPresenter &&
        creationPresenter->hasPendingWeightStart()) {
        cancelPendingWeightCreation();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::cancelPendingWeightCreation() {
    if (!creationPresenter || !creationPresenter->hasPendingWeightStart()) {
        return;
    }

    if (auto* scene = qobject_cast<GraphScene*>(ui->graphicsViewGraph->scene())) {
        scene->cancelPendingWeightCreation();
    }
}

void MainWindow::onCurrentTabChanged(int index) {
    if (ui->tabWidget->currentWidget() != ui->graph &&
        creationPresenter &&
        creationPresenter->hasPendingWeightStart()) {
        cancelPendingWeightCreation();
    }
}

bool MainWindow::modelHasUnsavedChanges(std::shared_ptr<FCM> model) {
    if (model == fcm) {
        updateFCM();
    }

    std::optional<FCM> savedModel;
    if (model->dbId != -1) {
        savedModel = savingManager->getFCM(model->name);
    }

    auto defaultFcm = FCM();
    defaultFcm.name = model->name;
    return (!savedModel && *model != defaultFcm) || (savedModel && *model != *savedModel);
}

void MainWindow::deleteSavedModel(const QString &modelName) {
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

    if (fcm->name == modelName) {
        ui->actionAutoSave->setEnabled(false);
        ui->actionAutoSave->setChecked(false);
    }

    emit modelDeletionFinished(modelName, true);
}

void MainWindow::deleteSavedTemplate(const QString &templateName) {
    if (!templatesManager->deleteTemplate(templateName)) {
        Logger::warn("Main template delete failed");
        emit modelDeletionFinished(templateName, false);
        return;
    }

    emit modelDeletionFinished(templateName, true);
}

bool MainWindow::closeModel(size_t index) {
    if (index >= fcms.size() || fcms.size() <= 1) {
        return false;
    }

    auto model = fcms[index];
    if (modelHasUnsavedChanges(model)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("There are unsaved changes!"),
            tr("Model \"%1\" has unsaved changes. Are you sure you want to close it?").arg(model->name),
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

void MainWindow::closeOtherModels(size_t index) {
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

void MainWindow::rebuildModelsMenu() {
    ui->menuModels->clear();

    for (size_t i = 0; i < fcms.size(); ++i) {
        auto* modelMenu = ui->menuModels->addMenu(fcms[i]->name);
        QAction* modelAction = modelMenu->menuAction();
        modelAction->setCheckable(true);
        modelAction->setChecked(i == currentModelIdx);
        modelAction->setData(QVariant::fromValue(static_cast<qulonglong>(i)));

        QAction* closeAction = modelMenu->addAction(tr("Close"));
        closeAction->setEnabled(fcms.size() > 1);
        connect(closeAction, &QAction::triggered, this, [this, i]() {
            if (i < fcms.size()) {
                closeModel(i);
            }
        });

        QAction* closeOtherAction = modelMenu->addAction(tr("Close other models"));
        closeOtherAction->setEnabled(fcms.size() > 1);
        connect(closeOtherAction, &QAction::triggered, this, [this, i]() {
            if (i < fcms.size()) {
                closeOtherModels(i);
            }
        });
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
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

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    for (const auto& model : fcms) {
        if (modelHasUnsavedChanges(model)) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                tr("There are unsaved changes!"),
                tr("There are unsaved changes! Are you sure you want to quit the program?"),
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

void MainWindow::recreatePresenters() {
    modelSetupPresenter = std::make_shared<ModelSetupPresenter>(ui, fcm, creationPresenter, staticAnalysisPresenter, simulationScenePresenter, this);
    simulationPresenter = std::make_shared<SimulationPresenter>(ui, fcm, modelSetupPresenter, simulationScenePresenter, this, this);
    connect(simulationPresenter.get(), &SimulationPresenter::autosave, this, &MainWindow::autosave);
    connect(simulationPresenter.get(), &SimulationPresenter::loadFCMRequested, this, &MainWindow::loadFCM);
}

void MainWindow::updateGraphScaleLabel(double newScale) {
    ui->labelScaleGraph->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    graphScale = newScale;
}

void MainWindow::updateSensitivityScaleLabel(double newScale) {
    ui->labelScaleSensitivity->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    sensitivityScale = newScale;
}

void MainWindow::updateModeButtonText(EditMode newMode) {
    ui->pushButtonMode->setText(newMode == EditMode::EditValues ? MainWindow::tr("Mode: Edit values") : MainWindow::tr("Mode: Create"));
    editMode = newMode;
}

void MainWindow::changeActivationFunctionSensitivity(int index) {
    ui->fuzzinessDegreeSensitivity->setEnabled(index == 3 || index == 4);
}

SensitivityAnalysisParameters MainWindow::getSensitivityParameters() {
    return {
        ui->doubleSpinBoxMaxChange->value(),
        ui->changeConcepts->isChecked(),
        ui->changeWeights->isChecked(),
        10,
        1000,
        ui->sensitivityMeasureMetric->currentData(Qt::UserRole).toString()
    };
}

void MainWindow::analize() {
    if (!modelSetupPresenter->checkElementsHaveValues()) {
        return;
    }

    activeSensitivity = true;

    ui->pushButtonAnalizeSensitivity->setEnabled(false);
    ui->pushButtonResetSensitivity->setEnabled(true);
    ui->doubleSpinBoxMaxChange->setEnabled(false);
    ui->changeConcepts->setEnabled(false);
    ui->changeWeights->setEnabled(false);

    sensitivityPresenter = std::make_shared<SensitivityPresenter>(ui->plotSensitivity, creationPresenter);

    auto* sensitivityScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy(sensitivityPresenter, ElementWindowMode::SensitivityAnalysis);
    auto* oldSensitivityScene = ui->graphicsViewGraph->scene();
    ui->graphicsViewSensitivity->setScene(sensitivityScene);
    if (oldSensitivityScene != ui->graphicsViewGraph->scene()) {
        delete oldSensitivityScene;
    }

    connect(sensitivityPresenter.get(), &SensitivityPresenter::updateProgress, this, &MainWindow::updateSensitivityProgress);
    sensitivityPresenter->setRuntimeContext(fcm, sensitivityScene->getFCM(), sensitivityScene);
    sensitivityPresenter->analize(simulationPresenter->getPredictionParameters(), getSensitivityParameters());
}

void MainWindow::resetSensitivity() {
    if (!activeSensitivity) {
        return;
    }

    ui->pushButtonAnalizeSensitivity->setEnabled(true);
    ui->pushButtonResetSensitivity->setEnabled(false);
    ui->doubleSpinBoxMaxChange->setEnabled(true);
    ui->changeConcepts->setEnabled(true);
    ui->changeWeights->setEnabled(true);
    sensitivityPresenter->reset();
    ui->progressBarSensitivity->setValue(0);
    activeSensitivity = false;
    auto* sensitivityScene = ui->graphicsViewSensitivity->scene();
    ui->graphicsViewSensitivity->setScene(ui->graphicsViewGraph->scene());
    delete sensitivityScene;
    ui->plotSensitivity->graph(0)->data()->clear();
    ui->plotSensitivity->replot();
}

void MainWindow::showSensitivityPlot() {
    int index = ui->stackedWidgetSensitivity->currentIndex();
    ui->stackedWidgetSensitivity->setCurrentIndex(index == 0 ? 1 : 0);
    if (sensitivityPlotShown) {;
        ui->showSensitivityPlot->setText(MainWindow::tr("FCM Sensitivity"));
    } else {
        ui->showSensitivityPlot->setText(MainWindow::tr("Elements Sensitivity"));
    }
    sensitivityPlotShown = !sensitivityPlotShown;
}

void MainWindow::updateSensitivityProgress(double progress) {
    ui->progressBarSensitivity->setValue(static_cast<int>(progress * 100));
}

void MainWindow::updateFCM() {
    fcm->name = ui->modelName->text();
    fcm->description = ui->modelNotes->markdownText();
    fcm->predictionParameters = simulationPresenter->getPredictionParameters();
    fcm->autoConfigureTermsColors = ui->autoColorConfiguration->isChecked();
    fcm->autoConfigureNumericValues = ui->autoNumericConfiguration->isChecked();
    fcm->autoConfigureFuzzyValues = ui->autoFuzzyConfiguration->isChecked();
}

void MainWindow::saveAs() {
    updateFCM();

    const auto modelsNames = savingManager->getModelsNames();
    SaveAsWindow saveAsWindow(modelsNames, modelsNames, fcm->name, MainWindow::tr("Save FCM"), this);

    if (saveAsWindow.exec() == QDialog::Accepted) {
        QString newName = saveAsWindow.savingModelName();
        fcm->name = newName;
        savingManager->saveAs(*fcm);
        ui->modelName->setText(newName);
        ui->actionAutoSave->setEnabled(true);
    }
}

void MainWindow::save() {
    if (fcm->dbId == -1) {
        saveAs();
    } else {
        updateFCM();
        savingManager->saveFCM(*fcm);
    }
}

void MainWindow::loadFCM(std::shared_ptr<FCM> newFCM) {
    QSignalBlocker b1(ui->modelName);
    QSignalBlocker b2(ui->termValue);
    QSignalBlocker b3(ui->termValueL);
    QSignalBlocker b4(ui->termValueM);
    QSignalBlocker b5(ui->termValueU);
    QSignalBlocker b6(ui->termNotes);
    auto* conceptsGroup = ui->treeWidgetTerms->topLevelItem(0);
    auto* weightsGroup = ui->treeWidgetTerms->topLevelItem(1);
    qDeleteAll(conceptsGroup->takeChildren());
    qDeleteAll(weightsGroup->takeChildren());

    fcm = newFCM;
    auto it = std::find(fcms.begin(), fcms.end(), fcm);
    if (it != fcms.end()) {
        currentModelIdx = static_cast<size_t>(std::distance(fcms.begin(), it));
    }
    rebuildModelsMenu();

    if (simulationScenePresenter->isActive()) {
        simulationPresenter->resetPredictionScene();
    }
    if (activeSensitivity) {
        resetSensitivity();
    }

    ui->modelName->setText(fcm->name);
    ui->modelNotes->setMarkdownText(fcm->description);

    QTreeWidgetItem* firstItem = nullptr;

    std::vector<std::pair<decltype(fcm->terms)::key_type, decltype(fcm->terms)::mapped_type>> sortedTerms(
        fcm->terms.begin(), fcm->terms.end()
        );

    std::sort(sortedTerms.begin(), sortedTerms.end(),
        [](const auto& a, const auto& b) {
            return a.second->value < b.second->value;
        });

    for (auto& [id, term] : sortedTerms) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, term->name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(id));
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if (term->type == ElementType::Node) {
            conceptsGroup->addChild(item);
        } else {
            weightsGroup->addChild(item);
        }

        if (!firstItem) {
            firstItem = item;
        }
    }

    ui->treeWidgetTerms->expandAll();

    if (creationPresenter) {
        creationPresenter->closeWindows();
    }
    creationPresenter = std::make_shared<CreationPresenter>(fcm, this);
    ui->adjacencyTableView->loadFromFCM(fcm);
    ui->adjacencyTableView->setPresenter(creationPresenter);
    simulationScenePresenter = std::make_shared<SimulationScenePresenter>(creationPresenter, this);
    connect(creationPresenter.get(), &CreationPresenter::autosave, this, &MainWindow::autosave);
    if (editMode == EditMode::EditValues) {
        updateModeButtonText(EditMode::Create);
    }

    auto newScene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    connect(ui->pushButtonMode, &QPushButton::clicked, newScene, &GraphScene::switchMode);
    connect(newScene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    auto oldSceneCreate = ui->graphicsViewGraph->scene();
    auto oldScenePredict = ui->graphicsViewPredict->scene();
    auto oldSceneSensitivity = ui->graphicsViewSensitivity->scene();
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

    delete staticAnalysisPresenter;
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, creationPresenter, fcm);
    recreatePresenters();

    ui->experimantsTable->model()->removeRows(0, ui->experimantsTable->model()->rowCount());

    for (const auto& experiment : fcm->experiments) {
        simulationPresenter->addExperiment(experiment);
    }

    ui->autoColorConfiguration->setChecked(fcm->autoConfigureTermsColors);
    ui->autoNumericConfiguration->setChecked(fcm->autoConfigureNumericValues);
    ui->autoFuzzyConfiguration->setChecked(fcm->autoConfigureFuzzyValues);

    int indexAlgorithm = ui->comboBoxAlgorithm->findData(fcm->predictionParameters.algorithm, Qt::UserRole);
    ui->comboBoxAlgorithm->setCurrentIndex(indexAlgorithm);
    ui->useFuzzyValues->setChecked(fcm->predictionParameters.useFuzzyValues);
    int indexActivation = ui->comboBoxActivation->findData(fcm->predictionParameters.activationFunction, Qt::UserRole);
    ui->comboBoxActivation->setCurrentIndex(indexActivation);
    int indexMetric = ui->comboBoxMetric->findData(fcm->predictionParameters.metric, Qt::UserRole);
    ui->comboBoxMetric->setCurrentIndex(indexMetric);
    ui->checkBoxPredictToStatic->setChecked(fcm->predictionParameters.predictToStatic);
    ui->doubleSpinBoxThreshold->setValue(fcm->predictionParameters.threshold);
    ui->spinBoxMetricSteps->setValue(fcm->predictionParameters.stepsLessThreshold);
    ui->spinBoxFixedSteps->setValue(fcm->predictionParameters.fixedSteps);
    ui->fuzzinessDegree->setValue(fcm->predictionParameters.fuzzinessDegree);

    ui->actionAutoSave->setEnabled(fcm->dbId != -1);
    ui->actionAutoSave->setChecked(fcm->autosaveOn);

    ui->graphicsViewGraph->resetTransform();
    ui->graphicsViewPredict->resetTransform();
    ui->graphicsViewSensitivity->resetTransform();
    updateGraphScaleLabel(1.0);
    simulationPresenter->updatePredictScaleLabel(1.0);
    updateSensitivityScaleLabel(1.0);
    ui->doubleSpinBoxMaxChange->setValue(0.1);
    ui->changeConcepts->setChecked(true);
    ui->changeWeights->setChecked(false);

    ui->useFuzzyValuesStatic->setChecked(false);
    ui->influenceDirection->setCurrentIndex(0);
    ui->influenceSteps->setValue(1);
    ui->graphConcept->setCurrentIndex(0);
}

void MainWindow::open() {
    const auto modelsNames = savingManager->getModelsNames();

    LoadModelWindow* loadModelWindow = new LoadModelWindow(modelsNames, MainWindow::tr("Open FCM"), this);
    connect(loadModelWindow, &LoadModelWindow::deleteModelRequested, this, &MainWindow::deleteSavedModel);
    connect(this, &MainWindow::modelDeletionFinished, loadModelWindow, &LoadModelWindow::onModelDeleted);

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

    fcm = std::make_shared<FCM>(*model);
    addFCM(fcm);
    loadFCM(fcm);
}

void MainWindow::autosaveChange(bool flag) {
    fcm->autosaveOn = flag;
    save();
}

void MainWindow::autosave() {
    if (fcm->autosaveOn) {
        save();
    }
}

void MainWindow::saveAsTemplate() {
    updateFCM();

    const auto templatesNamesWithTypes = templatesManager->getTemplatesNames();
    const auto filteredTemplatesNames = TemplatesLanguageManager::filterTemplateNamesForCurrentLanguage(
        templatesNamesWithTypes,
        settings
    );
    const auto allTemplatesNames = TemplatesLanguageManager::extractTemplateNames(templatesNamesWithTypes);
    SaveAsWindow saveAsWindow(
        filteredTemplatesNames,
        allTemplatesNames,
        fcm->name,
        MainWindow::tr("Save FCM Template"),
        this
    );

    if (saveAsWindow.exec() == QDialog::Accepted) {
        fcm->name = saveAsWindow.savingModelName();
        if (!templatesManager->createTemplate(*fcm)) {
            Logger::warn("Main template save failed");
        }
        ui->modelName->setText(fcm->name);
    }
}

void MainWindow::openTemplate() {
    const auto templatesNamesWithTypes = templatesManager->getTemplatesNames();
    const auto templatesNames = TemplatesLanguageManager::filterTemplateNamesForCurrentLanguage(
        templatesNamesWithTypes,
        settings
    );
    LoadModelWindow* loadModelWindow = new LoadModelWindow(templatesNames, MainWindow::tr("Open FCM Template"), this);
    connect(loadModelWindow, &LoadModelWindow::deleteModelRequested, this, &MainWindow::deleteSavedTemplate);
    connect(this, &MainWindow::modelDeletionFinished, loadModelWindow, &LoadModelWindow::onModelDeleted);

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

    fcm = std::make_shared<FCM>(*model);
    addFCM(fcm);
    loadFCM(fcm);
}

void MainWindow::onExportPng()
{
    QString proposedName = "fcm.png";
    if (!ui->modelName->text().isEmpty()) {
        proposedName = ui->modelName->text() + ".png";
    }
    QString fileName = QFileDialog::getSaveFileName(
        this,
        MainWindow::tr("Export as PNG"),
        proposedName,
        "PNG Images (*.png)"
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
        fileName += ".png";

    QPixmap pixmap = ui->graphicsViewGraph->grab();

    if (!pixmap.save(fileName, "PNG")) {
        QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Unable to save PNG to the selected file!"));
    }
}

void MainWindow::onExportJson() {
    QString proposedName = "fcm.json";
    if (!ui->modelName->text().isEmpty()) {
        proposedName = ui->modelName->text() + ".json";
    }
    QString fileName = QFileDialog::getSaveFileName(
        this,
        MainWindow::tr("Save FCM Model"),
        proposedName,
        "JSON files (*.json)"
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".json"))
        fileName += ".json";

    updateFCM();
    if (!JsonRepository::exportToJson(*fcm, fileName)) {
        Logger::warn("Main json export failed");
        QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Failed to save file."));
    }
}

void MainWindow::onImportJson() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        MainWindow::tr("Open FCM Model"),
        "",
        "JSON files (*.json)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    auto model = JsonRepository::importFromJson(fileName);

    if (!model) {
        Logger::warn("Main json import failed");
        QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Failed to load file."));
        return;
    }

    fcm = std::make_shared<FCM>(*model);
    addFCM(fcm);
    loadFCM(fcm);
}

void MainWindow::changeModelSettingsVisibility(bool checked) {
    ui->tabWidget->setTabVisible(0, checked);
    settings.setValue("tabs/modelSettings", checked);
}

void MainWindow::changeGraphVisibility(bool checked) {
    ui->tabWidget->setTabVisible(1, checked);
    settings.setValue("tabs/graph", checked);
}

void MainWindow::changeAdjacencyMatrixVisibility(bool checked) {
    ui->tabWidget->setTabVisible(2, checked);
    settings.setValue("tabs/adjMatrix", checked);
}

void MainWindow::changeStaticAnalysisVisibility(bool checked) {
    ui->tabWidget->setTabVisible(3, checked);
    settings.setValue("tabs/staticAnalysis", checked);
}

void MainWindow::changeSimulationVisibility(bool checked) {
    ui->tabWidget->setTabVisible(4, checked);
    settings.setValue("tabs/simulation", checked);
}

void MainWindow::changeExperimentsVisibility(bool checked) {
    ui->tabWidget->setTabVisible(5, checked);
    settings.setValue("tabs/experiments", checked);
}

void MainWindow::changeSensitivityAnalysisVisibility(bool checked) {
    ui->tabWidget->setTabVisible(6, checked);
    settings.setValue("tabs/sensitivity", checked);
}

void MainWindow::changeShowTooltips(bool checked) {
    toolTipController.setEnabled(checked);
    settings.setValue("tooltips", checked);
}

void MainWindow::showHelp() {
    if (!helpWindow) {
        helpWindow = new HelpWindow(this);
    }

    helpWindow->retranslate();
    helpWindow->show();
    helpWindow->raise();
}

void MainWindow::addFCM(std::shared_ptr<FCM> newFcm) {
    fcm = newFcm;
    currentModelIdx = fcms.size();
    fcms.push_back(fcm);
    rebuildModelsMenu();
}

void MainWindow::switchModel() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        Logger::warn("Model action missing");
        return;
    }
    size_t index = static_cast<size_t>(action->data().toULongLong());
    if (index >= fcms.size()) {
        Logger::warn("Model index invalid");
        return;
    }
    fcm = fcms[index];
    loadFCM(fcm);
}

void MainWindow::nameChanged(QString newName) {
    fcm->name = newName;
    rebuildModelsMenu();
    autosave();
}

void MainWindow::createNewModel() {
    fcm = std::make_shared<FCM>();
    size_t counter = 1;
    QStringList fcmsNames;
    for (const auto& model : fcms) {
        fcmsNames.append(model->name);
    }
    while (fcmsNames.contains(MainWindow::tr("New model") + (counter - 1 ? " (" + QString::number(counter) + ")" : ""))) {
        ++counter;
    }
    fcm->name = MainWindow::tr("New model") + (counter - 1 ? " (" + QString::number(counter) + ")" : "");
    addFCM(fcm);
    loadFCM(fcm);
}

void MainWindow::joinModels() {
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

    JoinWindow* joinWindow = new JoinWindow(unsavedModelsNames, savedModelsNames, templatesNames, this);

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
            QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Failed to load one of the selected saved models."));
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
            QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Failed to load the selected terms model template."));
            return;
        }

        baseFCM = std::make_shared<FCM>(*model);
    }

    if (baseFCM == nullptr) {
        QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Please select a valid terms model before proceeding!"));
        return;
    }

    auto joinedFCM = ModelsJoiner().join(baseFCM, joinFCMs, joinWindow->getJoinMode(), joinWindow->getResultName());

    addFCM(joinedFCM);
    loadFCM(joinedFCM);
}

void MainWindow::setEnglish() {
    QSignalBlocker b1(ui->actionRussian);
    QSignalBlocker b2(ui->actionEnglish);
    ui->actionEnglish->setChecked(true);
    ui->actionRussian->setChecked(false);
    qApp->removeTranslator(&translatorRus);
    qApp->removeTranslator(&translatorDefaultRus);
    qApp->removeTranslator(&translatorWidgetsRus);
    creationPresenter->retranslateElementsWindows();
    settings.setValue("language", "");
    if (helpWindow) {
        helpWindow->retranslate();
    }
}

void MainWindow::setRussian() {
    QSignalBlocker b1(ui->actionEnglish);
    QSignalBlocker b2(ui->actionRussian);
    ui->actionRussian->setChecked(true);
    ui->actionEnglish->setChecked(false);
    qApp->installTranslator(&translatorRus);
    qApp->installTranslator(&translatorDefaultRus);
    qApp->installTranslator(&translatorWidgetsRus);
    creationPresenter->retranslateElementsWindows();
    settings.setValue("language", "RU");
    if (helpWindow) {
        helpWindow->retranslate();
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);

        QSignalBlocker blocker(ui->treeWidgetTerms);
        auto* conceptsGroup = ui->treeWidgetTerms->topLevelItem(0);
        auto* weightsGroup = ui->treeWidgetTerms->topLevelItem(1);
        conceptsGroup->setText(0, tr("Concepts terms"));
        weightsGroup->setText(0, tr("Weights terms"));
        ui->plotSensitivity->xAxis->setLabel(tr("max change"));
        ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
        ui->plotSensitivity->replot();

        ui->plotSensitivity->xAxis->setLabel(tr("max change"));
        ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
        ui->labelScaleGraph->setText(QString(MainWindow::tr("Scale: %1%")).arg(graphScale*100, 0, 'f', 2));
        ui->labelScaleSensitivity->setText(QString(MainWindow::tr("Scale: %1%")).arg(sensitivityScale*100, 0, 'f', 2));
        ui->pushButtonMode->setText(editMode == EditMode::EditValues ? MainWindow::tr("Mode: Edit values") : MainWindow::tr("Mode: Create"));
        if (sensitivityPlotShown) {
            ui->showSensitivityPlot->setText(MainWindow::tr("Elements Sensitivity"));
        } else {
            ui->showSensitivityPlot->setText(MainWindow::tr("FCM Sensitivity"));
        }
        simulationPresenter->retranslateUi();
    }

    QMainWindow::changeEvent(event);
}

