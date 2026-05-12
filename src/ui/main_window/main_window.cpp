#include "main_window.h"
#include "ui_main_window.h"

#include "common/logger.h"
#include "presenter/model_setup_presenter.h"
#include "presenter/simulation_presenter.h"
#include "ui/graph_editor/graph_scene.h"

#include "repository/migration_manager.h"
#include "repository/saving_manager.h"
#include "repository/templates_manager.h"

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

    ui->influenceDirection->setItemData(0, "from", Qt::UserRole);
    ui->influenceDirection->setItemData(1, "on", Qt::UserRole);

    fcm = std::make_shared<FCM>();
    fcm->name = ui->modelName->text();

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
    auto savingManager = std::make_shared<SavingManager>(ModelsRepository(db));
    auto templatesManager = std::make_shared<TemplatesManager>(TemplatesRepository(db));

    creationPresenter = std::make_shared<CreationPresenter>(fcm, this);
    ui->adjacencyTableView->setPresenter(creationPresenter);

    auto* scene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);
    ui->graphicsViewSensitivity->setScene(scene);

    auto* staticAnalysisScene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    staticAnalysisScene->setMode(EditMode::EditValues);
    staticAnalysisScene->blockConceptCreationColorEdit(true);
    ui->staticAnalysis->findChild<GraphView*>("graphicsView")->setScene(staticAnalysisScene);

    connect(ui->pushButtonMode, &QPushButton::clicked, scene, &GraphScene::switchMode);
    connect(ui->pushButtonScaleGraph, &QPushButton::clicked, ui->graphicsViewGraph, &GraphView::resetScale);
    connect(ui->pushButtonScalePredict, &QPushButton::clicked, ui->graphicsViewPredict, &GraphView::resetScale);
    connect(ui->pushButtonScaleSensitivity, &QPushButton::clicked, ui->graphicsViewSensitivity, &GraphView::resetScale);

    ui->factorsStatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, creationPresenter, fcm);
    modelSetupPresenter = std::make_shared<ModelSetupPresenter>(ui, fcm, creationPresenter, staticAnalysisPresenter, simulationPresenter, nullptr);
    simulationPresenter = std::make_shared<SimulationPresenter>(ui, fcm, modelSetupPresenter, creationPresenter, this, nullptr);
    sensitivityPresenter = std::make_shared<SensitivityPresenter>(ui, fcm, modelSetupPresenter, simulationPresenter, creationPresenter, this, nullptr);
    modelsSwitchingPresenter = std::make_shared<ModelsSwitchingPresenter>(ui, this, fcm, fcms, creationPresenter, modelSetupPresenter, simulationPresenter, sensitivityPresenter, staticAnalysisPresenter, templatesManager, savingManager, settings, nullptr);
    savingExportPresenter = std::make_shared<SavingExportPresenter>(ui, fcm, fcms, modelSetupPresenter, templatesManager, savingManager, settings, this, nullptr);
    connect(savingExportPresenter.get(), &SavingExportPresenter::addFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::addFCM);
    connect(savingExportPresenter.get(), &SavingExportPresenter::loadFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::loadFCM);
    connect(modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::autosaveRequested, savingExportPresenter.get(), &SavingExportPresenter::autosave);
    connect(modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::currentModelChanged, savingExportPresenter.get(), &SavingExportPresenter::updateFCM);
    connect(creationPresenter.get(), &CreationPresenter::autosave, savingExportPresenter.get(), &SavingExportPresenter::autosave);
    connect(simulationPresenter.get(), &SimulationPresenter::autosave, savingExportPresenter.get(), &SavingExportPresenter::autosave);
    connect(simulationPresenter.get(), &SimulationPresenter::loadFCMRequested, modelsSwitchingPresenter.get(), &ModelsSwitchingPresenter::loadFCM);
    connect(ui->tabWidget, &QTabWidget::currentChanged, modelSetupPresenter.get(), &ModelSetupPresenter::onCurrentTabChanged);
    connect(scene, &GraphScene::modeChanged, modelSetupPresenter.get(), &ModelSetupPresenter::updateModeButtonText);
    connect(ui->graphicsViewGraph, &GraphView::scaleChanged, modelSetupPresenter.get(), &ModelSetupPresenter::updateGraphScaleLabel);

    connect(ui->comboBoxActivationSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), sensitivityPresenter.get(), &SensitivityPresenter::changeActivationFunctionSensitivity);

    modelsSwitchingPresenter->addFCM(fcm);
    ui->modelName->setText(tr("New model"));
    modelsSwitchingPresenter->loadFCM(fcm);
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
        modelSetupPresenter->retranslateUi();
        simulationPresenter->retranslateUi();
        sensitivityPresenter->retranslateUi();
    }

    QMainWindow::changeEvent(event);
}
