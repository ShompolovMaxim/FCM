#include "main_window.h"
#include "ui_main_window.h"

#include "common/logger.h"
#include "presenter/models/model_setup_presenter.h"
#include "presenter/simulation/simulation_presenter.h"
#include "ui/graph_editor/graph_scene.h"

#include "repository/migration_manager.h"
#include "repository/models_manager.h"
#include "repository/templates_manager.h"

#include <QMouseEvent>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    settingsPresenter = std::make_shared<SettingsPresenter>(ui, nullptr);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::showHelp);

    ui->influenceDirection->setItemData(0, "from", Qt::UserRole);
    ui->influenceDirection->setItemData(1, "on", Qt::UserRole);

    const QSettings settings("app.ini", QSettings::IniFormat);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(settings.value("database/path", "models.db").toString());
    if (!db.open()) {
        Logger::critical("Database open failed");
        qFatal("Cannot open database");
    }
    if (!MigrationManager::migrate(db)) {
        Logger::critical("Database migration failed");
        qFatal("Cannot apply database migrations");
    }
    auto savingManager = std::make_shared<ModelsSavingManager>(ModelsRepository(db));
    auto templatesManager = std::make_shared<TemplatesManager>(TemplatesRepository(db));

    modelsSwitchingPresenter = std::make_shared<ModelsSwitchingPresenter>(ui, this, creationPresenter, modelSetupPresenter, simulationPresenter, sensitivityPresenter, staticAnalysisPresenter, savingManager, nullptr);
    modelsJoinPresenter = std::make_shared<ModelsJoinPresenter>(modelsSwitchingPresenter->modelsRef(), templatesManager, savingManager, this, nullptr);

    creationPresenter = std::make_shared<CreationPresenter>(modelsSwitchingPresenter->currentModelRef(), this);
    ui->adjacencyTableView->setPresenter(creationPresenter);

    auto* scene = new GraphScene(modelsSwitchingPresenter->currentModel(), creationPresenter, ElementWindowMode::UpdateElement);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);
    ui->graphicsViewSensitivity->setScene(scene);

    auto* staticAnalysisScene = new GraphScene(modelsSwitchingPresenter->currentModel(), creationPresenter, ElementWindowMode::UpdateElement);
    staticAnalysisScene->setMode(EditMode::EditValues);
    staticAnalysisScene->blockConceptCreationColorEdit(true);
    ui->graphicsView->setScene(staticAnalysisScene);

    connect(ui->pushButtonMode, &QPushButton::clicked, scene, &GraphScene::switchMode);
    connect(ui->pushButtonScaleGraph, &QPushButton::clicked, ui->graphicsViewGraph, &GraphView::resetScale);
    connect(ui->pushButtonScalePredict, &QPushButton::clicked, ui->graphicsViewPredict, &GraphView::resetScale);
    connect(ui->pushButtonScaleSensitivity, &QPushButton::clicked, ui->graphicsViewSensitivity, &GraphView::resetScale);

    ui->factorsStatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui, creationPresenter, modelsSwitchingPresenter->currentModel());
    modelSetupPresenter = std::make_shared<ModelSetupPresenter>(ui, modelsSwitchingPresenter->currentModelRef(), creationPresenter, nullptr);
    simulationPresenter = std::make_shared<SimulationPresenter>(ui, modelsSwitchingPresenter->currentModelRef(), creationPresenter, this, nullptr);
    sensitivityPresenter = std::make_shared<SensitivityPresenter>(ui, modelsSwitchingPresenter->currentModelRef(), creationPresenter, this, nullptr);
    savingExportPresenter = std::make_shared<SavingExportPresenter>(ui, modelsSwitchingPresenter->modelsRef(), modelSetupPresenter, templatesManager, savingManager, this, nullptr);
    connect(savingExportPresenter.get(), &SavingExportPresenter::addFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::addFCM);
    connect(savingExportPresenter.get(), &SavingExportPresenter::loadFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::loadFCM);
    connect(savingExportPresenter.get(), &SavingExportPresenter::currentModelNameRestoreRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::restoreCurrentModelName);
    connect(modelsJoinPresenter.get(), &ModelsJoinPresenter::addFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::addFCM);
    connect(modelsJoinPresenter.get(), &ModelsJoinPresenter::loadFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::loadFCM);
    connect(modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::autosaveRequested, savingExportPresenter.get(), &SavingExportPresenter::autosave);
    connect(creationPresenter.get(), &CreationPresenter::autosave, savingExportPresenter.get(), &SavingExportPresenter::autosave);
    connect(simulationPresenter.get(), &SimulationPresenter::autosave, savingExportPresenter.get(), &SavingExportPresenter::autosave);
    connect(simulationPresenter.get(), &SimulationPresenter::loadFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::loadFCM);
    connect(modelSetupPresenter.get(), &ModelSetupPresenter::popagateTermUpdate, simulationPresenter.get(), &SimulationPresenter::refreshFromModelSetup);
    connect(modelSetupPresenter.get(), &ModelSetupPresenter::popagateTermUpdate, staticAnalysisPresenter, &StaticAnalysisPresenter::refreshFromModelSetup);
    connect(ui->actionJoinFCM, &QAction::triggered, modelsJoinPresenter.get(), &ModelsJoinPresenter::joinModels);
    connect(ui->tabWidget, &QTabWidget::currentChanged, modelSetupPresenter.get(), &ModelSetupPresenter::onCurrentTabChanged);
    connect(scene, &GraphScene::modeChanged, modelSetupPresenter.get(), &ModelSetupPresenter::updateModeButtonText);
    connect(ui->graphicsViewGraph, &GraphView::scaleChanged, modelSetupPresenter.get(), &ModelSetupPresenter::updateGraphScaleLabel);

    connect(ui->comboBoxActivationSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), sensitivityPresenter.get(), &SensitivityPresenter::changeActivationFunctionSensitivity);

    settingsPresenter->applyInitialLanguage();
    modelsSwitchingPresenter->loadFCM(modelsSwitchingPresenter->currentModel());
    ui->menuModels->installEventFilter(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (modelSetupPresenter && modelSetupPresenter->keyPressEvent(event)) {
        return;
    }

    QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (modelsSwitchingPresenter && modelsSwitchingPresenter->eventFilter(watched, event)) {
        return true;
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (modelsSwitchingPresenter) {
        modelsSwitchingPresenter->closeEvent(event);
    } else {
        event->accept();
    }
}

void MainWindow::showHelp() {
    if (!helpWindow) {
        helpWindow = new HelpWindow(this);
    }

    helpWindow->retranslate();
    helpWindow->show();
    helpWindow->raise();
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        if (modelSetupPresenter) {
            modelSetupPresenter->retranslateUi();
        }
        if (simulationPresenter) {
            simulationPresenter->retranslateUi();
        }
        if (sensitivityPresenter) {
            sensitivityPresenter->retranslateUi();
        }
        if (modelsSwitchingPresenter) {
            modelsSwitchingPresenter->retranslateUi();
        }
        if (creationPresenter) {
            creationPresenter->retranslateElementsWindows();
        }
        if (helpWindow) {
            helpWindow->retranslate();
        }
    }

    QMainWindow::changeEvent(event);
}

