#include "main_window.h"
#include "ui_main_window.h"

#include "ui/join_window/join_window.h"
#include "ui/graph_editor/graph_scene.h"
#include "ui/save_as_window/save_as_window.h"
#include "ui/save_current_state_window/save_current_state_window.h"
#include "ui/load_model_window/load_model_window.h"

#include "repository/json_repository.h"
#include "repository/migration_manager.h"

#include "model/join/models_joiner.h"

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
    ui->influenceDirection->setItemData(0, "from", Qt::UserRole);
    ui->influenceDirection->setItemData(1, "on", Qt::UserRole);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("models.db");
    if (!db.open()) {
        qFatal("Cannot open database");
    }
    MigrationManager::migrate(db);
    savingManager = std::make_shared<SavingManager>(ModelsRepository(db));
    templatesManager = std::make_shared<TemplatesManager>(TemplatesRepository(db));

    fcm = std::make_shared<FCM>();

    creationPresenter = std::make_shared<CreationPresenter>(fcm, this);
    connect(creationPresenter.get(), &CreationPresenter::autosave, this, &MainWindow::autosave);
    ui->adjacencyTableView->setPresenter(creationPresenter);
    presenter = std::make_shared<SimulationPresenter>(creationPresenter, nullptr);

    auto* scene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);
    ui->graphicsViewSensitivity->setScene(scene);

    auto* staticAnalysisScene = new GraphScene(fcm, creationPresenter, ElementWindowMode::UpdateElement);
    staticAnalysisScene->setMode(EditMode::EditValues);
    staticAnalysisScene->blockConceptCreationColorEdit(true);
    ui->staticAnalysis->findChild<GraphView*>("graphicsView")->setScene(staticAnalysisScene);

    connect(ui->modelNotes, &QTextEdit::textChanged, this, &MainWindow::descriptionChanged);
    connect(ui->textEditNotesPredict, &QTextEdit::textChanged, this, &MainWindow::descriptionChanged);
    connect(ui->textEditNotesSensitivity, &QTextEdit::textChanged, this, &MainWindow::descriptionChanged);

    connect(ui->pushButtonMode, &QPushButton::clicked, scene, &GraphScene::switchMode);
    connect(scene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    connect(ui->graphicsViewGraph, &GraphView::scaleChanged, this, &MainWindow::updateGraphScaleLabel);
    connect(ui->graphicsViewPredict, &GraphView::scaleChanged, this, &MainWindow::updatePredictScaleLabel);
    connect(ui->graphicsViewSensitivity, &GraphView::scaleChanged, this, &MainWindow::updateSensitivityScaleLabel);
    connect(ui->pushButtonScaleGraph, &QPushButton::clicked, ui->graphicsViewGraph, &GraphView::resetScale);
    connect(ui->pushButtonScalePredict, &QPushButton::clicked, ui->graphicsViewPredict, &GraphView::resetScale);
    connect(ui->pushButtonScaleSensitivity, &QPushButton::clicked, ui->graphicsViewSensitivity, &GraphView::resetScale);
    connect(ui->pushButtonPredict, &QPushButton::clicked, this, &MainWindow::predict);
    connect(ui->pushButtonReset, &QPushButton::clicked, this, &MainWindow::resetPredictionScene);
    connect(ui->pushButtonPause, &QPushButton::clicked, this, &MainWindow::pauseResumePrediction);
    connect(ui->pushButtonSpeedUp, &QPushButton::clicked, this, &MainWindow::speedUp);
    connect(ui->pushButtonSlowDown, &QPushButton::clicked, this, &MainWindow::slowDown);
    connect(ui->pushButtonForward, &QPushButton::clicked, this, &MainWindow::stepForward);
    connect(ui->pushButtonBack, &QPushButton::clicked, this, &MainWindow::stepBack);
    connect(ui->pushButtonFinish, &QPushButton::clicked, this, &MainWindow::finishSimulation);
    connect(&*presenter, &SimulationPresenter::updateProgress, this, &MainWindow::updateProgress);
    connect(&*presenter, &SimulationPresenter::finished, this, &MainWindow::simulationFinished);
    connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, this, &MainWindow::onPredictToStaticChanged);

    connect(ui->createTermButton, &QPushButton::clicked, this, &MainWindow::onCreateTerm);
    connect(ui->deleteTermButton, &QPushButton::clicked, this, &MainWindow::onDeleteTerm);
    connect(ui->termColorButton, &QPushButton::clicked, this, &MainWindow::onChooseTermColor);
    connect(ui->termValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueChanged);
    connect(ui->termValueL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueLChanged);
    connect(ui->termValueM, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueMChanged);
    connect(ui->termValueU, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueUChanged);
    connect(ui->termNotes, &QTextEdit::textChanged, this, &MainWindow::termNotesChanged);

    conceptsGroup = new QTreeWidgetItem(ui->treeWidgetTerms);
    conceptsGroup->setText(0, tr("Concepts terms"));
    weightsGroup = new QTreeWidgetItem(ui->treeWidgetTerms);
    weightsGroup->setText(0, tr("Weights terms"));
    connect(ui->treeWidgetTerms, &QTreeWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);
    connect(ui->treeWidgetTerms, &QTreeWidget::itemChanged, this, &MainWindow::onItemChanged);

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

    connect(ui->comboBoxAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), std::bind(&MainWindow::autosave, this));
    connect(ui->useFuzzyValues, &QCheckBox::toggled, std::bind(&MainWindow::autosave, this));
    connect(ui->comboBoxActivation, QOverload<int>::of(&QComboBox::currentIndexChanged), std::bind(&MainWindow::autosave, this));
    connect(ui->comboBoxMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), std::bind(&MainWindow::autosave, this));
    connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, std::bind(&MainWindow::autosave, this));
    connect(ui->doubleSpinBoxThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), std::bind(&MainWindow::autosave, this));
    connect(ui->spinBoxMetricSteps, QOverload<int>::of(&QSpinBox::valueChanged), std::bind(&MainWindow::autosave, this));
    connect(ui->spinBoxFixedSteps, QOverload<int>::of(&QSpinBox::valueChanged), std::bind(&MainWindow::autosave, this));

    ui->fuzzyValuePlot->xAxis->setRange(-1.1, 1.1);
    ui->fuzzyValuePlot->yAxis->setRange(0, 1);
    ui->fuzzyValuePlot->xAxis->setLabel("x");
    ui->fuzzyValuePlot->yAxis->setLabel("μ(x)");
    ui->fuzzyValuePlot->addGraph();

    ui->factorsStatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, creationPresenter, fcm);

    connect(ui->comboBoxAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxAlgorithmSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->comboBoxAlgorithmSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxAlgorithm, &QComboBox::setCurrentIndex);
    connect(ui->useFuzzyValues, &QCheckBox::toggled, ui->useFuzzyValuesSensitivity, &QCheckBox::setChecked);
    connect(ui->useFuzzyValuesSensitivity, &QCheckBox::toggled, ui->useFuzzyValues, &QCheckBox::setChecked);
    connect(ui->comboBoxActivation, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxActivationSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->comboBoxActivationSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxActivation, &QComboBox::setCurrentIndex);
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
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}

void MainWindow::descriptionChanged() {
    QSignalBlocker b1(ui->modelNotes);
    QSignalBlocker b2(ui->textEditNotesPredict);
    QSignalBlocker b3(ui->textEditNotesSensitivity);
    if (sender() == ui->modelNotes) {
        ui->textEditNotesPredict->setMarkdownText(ui->modelNotes->markdownText());
        ui->textEditNotesSensitivity->setMarkdownText(ui->modelNotes->markdownText());
    }
    if (sender() == ui->textEditNotesPredict) {
        ui->modelNotes->setMarkdownText(ui->textEditNotesPredict->markdownText());
        ui->textEditNotesSensitivity->setMarkdownText(ui->textEditNotesPredict->markdownText());
    }
    if (sender() == ui->textEditNotesSensitivity) {
        ui->modelNotes->setMarkdownText(ui->textEditNotesSensitivity->markdownText());
        ui->textEditNotesPredict->setMarkdownText(ui->textEditNotesSensitivity->markdownText());
    }
}

void MainWindow::updateGraphScaleLabel(double newScale) {
    ui->labelScaleGraph->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    graphScale = newScale;
}

void MainWindow::updatePredictScaleLabel(double newScale) {
    ui->labelScalePredict->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    predictScale = newScale;
}

void MainWindow::updateSensitivityScaleLabel(double newScale) {
    ui->labelScaleSensitivity->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    sensitivityScale = newScale;
}

void MainWindow::updateModeButtonText(EditMode newMode) {
    ui->pushButtonMode->setText(newMode == EditMode::EditValues ? MainWindow::tr("Mode: Edit values") : MainWindow::tr("Mode: Create"));
    editMode = newMode;
}

void MainWindow::addExperiment(const Experiment& experiment) {
    auto experimentsModel = ui->experimantsTable->model();
    int row = experimentsModel->rowCount();
    experimentsModel->insertRow(row);
    experimentsModel->setData(experimentsModel->index(row, 0), MainWindow::tr(experiment.predictionParameters.algorithm.toUtf8().constData()));
    experimentsModel->setData(experimentsModel->index(row, 1), experiment.predictionParameters.useFuzzyValues ? tr("fuzzy") : tr("numeric"));
    experimentsModel->setData(experimentsModel->index(row, 2), MainWindow::tr(experiment.predictionParameters.activationFunction.toUtf8().constData()));
    experimentsModel->setData(experimentsModel->index(row, 3), MainWindow::tr(experiment.predictionParameters.metric.toUtf8().constData()));
    experimentsModel->setData(experimentsModel->index(row, 4), experiment.predictionParameters.predictToStatic ? tr("yes") : tr("no"));
    experimentsModel->setData(experimentsModel->index(row, 5), experiment.predictionParameters.threshold);
    experimentsModel->setData(experimentsModel->index(row, 6), experiment.predictionParameters.stepsLessThreshold);
    experimentsModel->setData(experimentsModel->index(row, 7), experiment.predictionParameters.fixedSteps);
    experimentsModel->setData(experimentsModel->index(row, 8), experiment.timestamp);
    experimentsModel->setData(experimentsModel->index(row, 9), "");
    experimentsModel->setData(experimentsModel->index(row, 10), "");
    for (int column = 0; column < experimentsModel->columnCount(); ++column) {
        experimentsModel->setData(experimentsModel->index(row, column), Qt::AlignCenter, Qt::TextAlignmentRole);
    }
    QPushButton* btn = new QPushButton(tr("Load"), ui->experimantsTable);
    btn->setProperty("row", row);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 9), btn);
    QPushButton* deleteButton = new QPushButton(tr("Delete"), ui->experimantsTable);
    deleteButton->setProperty("row", row);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 10), deleteButton);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteExperiment);
    QObject::connect(btn, &QPushButton::clicked, this, &MainWindow::loadExperiment);
}

bool MainWindow::checkElementsHaveValues() {
    for (const auto& [_, concept] : fcm->concepts) {
        if (!concept->term) {
            QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Not every concept has a value!"));
            return false;
        }
    }
    for (const auto& [_, weight] : fcm->weights) {
        if (!weight->term) {
            QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Not every weight has a value!"));
            return false;
        }
    }
    return true;
}

void MainWindow::simulationFinished() {
    if (!paused) {
        pauseResumePrediction();
    }
    ui->pushButtonBack->setEnabled(true);
    ui->pushButtonForward->setEnabled(true);
    ui->pushButtonSlowDown->setEnabled(true);
    ui->pushButtonPause->setEnabled(true);
    ui->pushButtonSpeedUp->setEnabled(true);
    ui->pushButtonFinish->setEnabled(true);
}

void MainWindow::predict() {
    if (!checkElementsHaveValues()) {
        return;
    }

    activeSimulation = true;

    auto predictionParameters = getPredictionParameters();

    auto simulationParameters = SimulationParameters{
        ui->checkBoxRealTime->isChecked(),
        ui->doubleSpinBoxStepsPerSecond->value()
    };

    createExperiment();

    ui->pushButtonPredict->setEnabled(false);
    ui->pushButtonReset->setEnabled(true);
    ui->checkBoxRealTime->setEnabled(false);
    ui->doubleSpinBoxStepsPerSecond->setEnabled(false);
    if (simulationParameters.realTime) {
        ui->pushButtonBack->setEnabled(true);
        ui->pushButtonForward->setEnabled(true);
        ui->pushButtonSlowDown->setEnabled(true);
        ui->pushButtonPause->setEnabled(true);
        ui->pushButtonSpeedUp->setEnabled(true);
        ui->pushButtonFinish->setEnabled(true);
    }

    auto* predictScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy(presenter, ElementWindowMode::PredictionResults);
    auto* oldPredictScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewPredict->setScene(predictScene);
    if (oldPredictScene != ui->graphicsViewGraph->scene()) {
        delete oldPredictScene;
    }

    QList<NodeItem*> nodes;
    QMap<QUuid, EdgeItem*> edges;
    for (QGraphicsItem* item : predictScene->items()) {
        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            nodes.append(n);
        }
        if (auto ed = qgraphicsitem_cast<EdgeItem*>(item)) {
            edges[ed->getId()] = ed;
        }
    }

    paused = false;

    presenter->setRuntimeContext(fcm, predictScene->getFCM(), predictScene);
    presenter->simulate(predictionParameters, simulationParameters, nodes, edges);
}

void MainWindow::resetPredictionScene() {
    if (!activeSimulation) {
        return;
    }
    presenter->reset();
    ui->pushButtonPredict->setEnabled(true);
    ui->pushButtonReset->setEnabled(false);
    ui->checkBoxRealTime->setEnabled(true);
    ui->doubleSpinBoxStepsPerSecond->setEnabled(true);
    ui->pushButtonBack->setEnabled(false);
    ui->pushButtonForward->setEnabled(false);
    ui->pushButtonSlowDown->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonSpeedUp->setEnabled(false);
    ui->pushButtonFinish->setEnabled(false);
    ui->progressBarPredict->setValue(0);
    activeSimulation = false;
    auto* predictionScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewPredict->setScene(ui->graphicsViewGraph->scene());
    delete predictionScene;
}

void MainWindow::pauseResumePrediction() {
    if (!paused) {
        ui->pushButtonPause->setText(MainWindow::tr("Resume"));
        presenter->pause();
        paused = true;
    } else {
        ui->pushButtonPause->setText(MainWindow::tr("Pause"));
        presenter->resume();
        paused = false;
    }
}

void MainWindow::speedUp() {
    presenter->speedUp();
    ui->doubleSpinBoxStepsPerSecond->setValue(presenter->getStepsPerSecond());
}

void MainWindow::slowDown() {
    presenter->slowDown();
    ui->doubleSpinBoxStepsPerSecond->setValue(presenter->getStepsPerSecond());
}

void MainWindow::stepForward() {
    if (!presenter->moveStep(ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Value of step is not calculated or step out of range!"));
    }
}

void MainWindow::stepBack() {
    if (!presenter->moveStep(-ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(this, MainWindow::tr("Error"), MainWindow::tr("Value of step is not calculated or step out of range!"));
    }
}

void MainWindow::finishSimulation() {
    ui->pushButtonBack->setEnabled(false);
    ui->pushButtonForward->setEnabled(false);
    ui->pushButtonSlowDown->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonSpeedUp->setEnabled(false);
    ui->pushButtonFinish->setEnabled(false);
    presenter->finish();
}

void MainWindow::updateProgress(size_t value, size_t maxStep, double metricValue) {
    ui->progressBarPredict->setMaximum(maxStep);
    ui->progressBarPredict->setValue(value);
    ui->labelMetricValue->setText(QString(MainWindow::tr("Metric value: %1")).arg(metricValue, 0, 'f', 4));
    currentMetricValue = metricValue;
}

SensitivityAnalysisParameters MainWindow::getSensitivityParameters() {
    return {
        ui->doubleSpinBoxMaxChange->value(),
        ui->changeConcepts->isChecked(),
        ui->changeWeights->isChecked(),
        10,
        1000
    };
}

void MainWindow::analize() {
    if (!checkElementsHaveValues()) {
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
    sensitivityPresenter->analize(getPredictionParameters(), getSensitivityParameters());
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

void MainWindow::onCreateTerm() {
    QTreeWidgetItem* currentItem = ui->treeWidgetTerms->currentItem();

    QTreeWidgetItem* targetGroup = conceptsGroup;
    ElementType type = ElementType::Node;

    if (currentItem) {
        if (currentItem == weightsGroup || (currentItem->parent() == weightsGroup)) {
            targetGroup = weightsGroup;
            type = ElementType::Edge;
        }
    }

    auto id = QUuid::createUuid();

    fcm->terms[id] = std::make_shared<Term>();
    fcm->terms[id]->id = id;
    fcm->terms[id]->name = MainWindow::tr("New term");
    fcm->terms[id]->type = type;

    if (type == ElementType::Node) {
        fcm->terms[id]->color = ColorValueAdapter().getColor(fcm->terms[id]->value, 0, 1);
    } else {
        fcm->terms[id]->color = ColorValueAdapter().getColor(fcm->terms[id]->value, -1, 1);
    }


    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, MainWindow::tr("New term"));
    item->setData(0, Qt::UserRole, QVariant::fromValue(id));
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    targetGroup->addChild(item);

    ui->treeWidgetTerms->setCurrentItem(item);
    ui->treeWidgetTerms->editItem(item, 0);

    creationPresenter->updateTerm(id);
    popagateTermUpdate();
}

void MainWindow::onDeleteTerm() {
    QTreeWidgetItem  *current = ui->treeWidgetTerms->currentItem();
    if (current && current->parent()) {
        auto* parent = current->parent();
        auto id = current->data(0, Qt::UserRole).toUuid();
        if (fcm->terms[id]->dbId != -1) {
            fcm->deletedTermsIds.push_back(fcm->terms[id]->dbId);
        }
        creationPresenter->deleteTerm(id);
        popagateTermUpdate();
        delete current;
        ui->treeWidgetTerms->setCurrentItem(parent);
    }
}

void MainWindow::onChooseTermColor() {
    QColor color = QColorDialog::getColor(Qt::white, this, QString(MainWindow::tr("Choose term %1 color")).arg(fcm->terms[currentTermId]->name));
    if (color.isValid()) {
        fcm->terms[currentTermId]->color = color;
        ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
        creationPresenter->updateTerm(currentTermId);
        popagateTermUpdate();
    }
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    ui->createTermButton->setEnabled(current);

    QSignalBlocker b1(ui->termValue);
    QSignalBlocker b2(ui->termValueL);
    QSignalBlocker b3(ui->termValueM);
    QSignalBlocker b4(ui->termValueU);
    QSignalBlocker b5(ui->termNotes);

    if (!current || !current->parent()) {
        ui->termValue->setValue(0);
        ui->termValueL->setValue(0);
        ui->termValueM->setValue(0);
        ui->termValueU->setValue(0);
        ui->termNotes->setMarkdownText("");
        ui->termValue->setEnabled(false);
        ui->termValueL->setEnabled(false);
        ui->termValueM->setEnabled(false);
        ui->termValueU->setEnabled(false);
        ui->termNotes->setEnabled(false);
        ui->deleteTermButton->setEnabled(false);
        ui->termColorButton->setEnabled(false);
        ui->termColorButton->setStyleSheet("");
        ui->fuzzyValuePlot->graph(0)->data()->clear();
        ui->fuzzyValuePlot->replot();
        return;
    }

    currentTermId = current->data(0, Qt::UserRole).toUuid();

    ui->termValue->setEnabled(true);
    ui->termValueL->setEnabled(true);
    ui->termValueM->setEnabled(true);
    ui->termValueU->setEnabled(true);
    ui->termNotes->setEnabled(true);
    ui->deleteTermButton->setEnabled(true);
    ui->termColorButton->setEnabled(true);

    double mn = fcm->terms[currentTermId]->type == ElementType::Node ? 0.0 : -1.0;
    ui->termValue->setMinimum(mn);
    ui->termValue->setMaximum(1.0);
    ui->termValueL->setMinimum(mn);
    ui->termValueL->setMaximum(1.0);
    ui->termValueM->setMinimum(mn);
    ui->termValueM->setMaximum(1.0);
    ui->termValueU->setMinimum(mn);
    ui->termValueU->setMaximum(1.0);

    ui->termValue->setValue(fcm->terms[currentTermId]->value);
    ui->termValueL->setValue(fcm->terms[currentTermId]->fuzzyValue.l);
    ui->termValueM->setValue(fcm->terms[currentTermId]->fuzzyValue.m);
    ui->termValueU->setValue(fcm->terms[currentTermId]->fuzzyValue.u);
    ui->termNotes->setMarkdownText(fcm->terms[currentTermId]->description);
    ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(fcm->terms[currentTermId]->color.name()));
    updateFuzzyValuePlot();
}

void MainWindow::termNotesChanged() {
    if (ui->treeWidgetTerms->currentItem() && ui->treeWidgetTerms->currentItem()->parent()) {
        fcm->terms[currentTermId]->description = ui->termNotes->markdownText();
    }
}

void MainWindow::autoConfigureTermColor() {
    if (!ui->autoColorConfiguration->isChecked()) {
        return;
    }
    double meanTermValue = (fcm->terms[currentTermId]->value + fcm->terms[currentTermId]->fuzzyValue.defuzzify()) / 2;
    if (fcm->terms[currentTermId]->type == ElementType::Node) {
        fcm->terms[currentTermId]->color = ColorValueAdapter().getColor(meanTermValue, 0, 1);
    } else {
        fcm->terms[currentTermId]->color = ColorValueAdapter().getColor(meanTermValue, -1, 1);
    }
    ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(fcm->terms[currentTermId]->color.name()));
}

void MainWindow::autoConfigureNumericValue() {
    if (!ui->autoNumericConfiguration->isChecked()) {
        return;
    }
    QSignalBlocker b1(ui->termValue);
    fcm->terms[currentTermId]->value = fcm->terms[currentTermId]->fuzzyValue.defuzzify();
    ui->termValue->setValue(fcm->terms[currentTermId]->fuzzyValue.defuzzify());
}

void MainWindow::autoConfigureFuzzyValue() {
    if (!ui->autoFuzzyConfiguration->isChecked()) {
        return;
    }
    QSignalBlocker b1(ui->termValueL);
    QSignalBlocker b2(ui->termValueM);
    QSignalBlocker b3(ui->termValueU);
    fcm->terms[currentTermId]->fuzzyValue.l = std::max(fcm->terms[currentTermId]->value - 0.2, fcm->terms[currentTermId]->type == ElementType::Edge ? -1.0 : 0.0);
    fcm->terms[currentTermId]->fuzzyValue.m = fcm->terms[currentTermId]->value;
    fcm->terms[currentTermId]->fuzzyValue.u = std::min(fcm->terms[currentTermId]->value + 0.2, 1.0);
    ui->termValueL->setValue(fcm->terms[currentTermId]->fuzzyValue.l);
    ui->termValueM->setValue(fcm->terms[currentTermId]->fuzzyValue.m);
    ui->termValueU->setValue(fcm->terms[currentTermId]->fuzzyValue.u);
}

void MainWindow::popagateTermUpdate() {
    staticAnalysisPresenter->refreshUI(false);
    if (activeSimulation) {
        presenter->moveStep(0);
    }
}

void MainWindow::onTermValueChanged(double value) {
    fcm->terms[currentTermId]->value = value;
    autoConfigureFuzzyValue();
    autoConfigureTermColor();
    updateFuzzyValuePlot();
    creationPresenter->updateTerm(currentTermId);
    popagateTermUpdate();
}

void MainWindow::onTermValueLChanged(double value) {
    if (ui->termValueM->value() < value) {
        ui->termValueM->setValue(value);
        fcm->terms[currentTermId]->fuzzyValue.m = value;
    }
    if (ui->termValueU->value() < value) {
        ui->termValueU->setValue(value);
        fcm->terms[currentTermId]->fuzzyValue.u = value;
    }
    fcm->terms[currentTermId]->fuzzyValue.l = value;
    updateFuzzyValuePlot();
    autoConfigureNumericValue();
    autoConfigureTermColor();
    creationPresenter->updateTerm(currentTermId);
    popagateTermUpdate();
}

void MainWindow::onTermValueMChanged(double value) {
    if (ui->termValueL->value() > value) {
        ui->termValueL->setValue(value);
        fcm->terms[currentTermId]->fuzzyValue.l = value;
    }
    if (ui->termValueU->value() < value) {
        ui->termValueU->setValue(value);
        fcm->terms[currentTermId]->fuzzyValue.u = value;
    }
    fcm->terms[currentTermId]->fuzzyValue.m = value;
    updateFuzzyValuePlot();
    autoConfigureNumericValue();
    autoConfigureTermColor();
    creationPresenter->updateTerm(currentTermId);
    popagateTermUpdate();
}

void MainWindow::onTermValueUChanged(double value) {
    if (ui->termValueL->value() > value) {
        ui->termValueL->setValue(value);
        fcm->terms[currentTermId]->fuzzyValue.l = value;
    }
    if (ui->termValueM->value() > value) {
        ui->termValueM->setValue(value);
        fcm->terms[currentTermId]->fuzzyValue.m = value;
    }
    fcm->terms[currentTermId]->fuzzyValue.u = value;
    updateFuzzyValuePlot();
    autoConfigureNumericValue();
    autoConfigureTermColor();
    creationPresenter->updateTerm(currentTermId);
    popagateTermUpdate();
}

void MainWindow::updateFuzzyValuePlot() {
    ui->fuzzyValuePlot->graph(0)->setData(QVector<double>{ui->termValueL->value(), ui->termValueM->value(), ui->termValueU->value()}, QVector<double>{0, 1, 0});
    ui->fuzzyValuePlot->replot();
}

void MainWindow::onItemChanged(QTreeWidgetItem  *item, int column) {
    auto id = item->data(0, Qt::UserRole).toUuid();
    fcm->terms[id]->name = item->text(0);
    creationPresenter->updateTerm(id);
}

void MainWindow::onPredictToStaticChanged(bool checked) {
    ui->doubleSpinBoxThreshold->setEnabled(checked);
    ui->spinBoxMetricSteps->setEnabled(checked);
    ui->spinBoxFixedSteps->setEnabled(!checked);

    ui->doubleSpinBoxThresholdSensitivity->setEnabled(checked);
    ui->spinBoxMetricStepsSensitivity->setEnabled(checked);
    ui->spinBoxFixedStepsSensitivity->setEnabled(!checked);
}

Experiment MainWindow::createExperiment() {
    Experiment experiment;
    for (auto& [id, term] : fcm->terms) {
        experiment.terms[id] = std::make_shared<Term>(*term);
        experiment.terms[id]->dbId = -1;
    }
    for (const auto& [id, concept] : fcm->concepts) {
        experiment.concepts[id] = std::make_shared<Concept>(*concept);
        experiment.concepts[id]->dbId = -1;
        experiment.concepts[id]->term = experiment.terms[fcm->concepts[id]->term->id];
        experiment.concepts[id]->predictedValues = {};
        experiment.concepts[id]->sensitivity = {};
    }
    for (const auto& [id, weight] : fcm->weights) {
        experiment.weights[id] = std::make_shared<Weight>(*weight);
        experiment.weights[id]->dbId = -1;
        experiment.weights[id]->term = experiment.terms[fcm->weights[id]->term->id];
        experiment.weights[id]->predictedValues = {};
        experiment.weights[id]->sensitivity = {};
    }
    experiment.predictionParameters = getPredictionParameters();
    experiment.timestamp = QDateTime::currentDateTime();
    fcm->experiments.push_back(experiment);
    addExperiment(experiment);
    autosave();
    return experiment;
}

void MainWindow::loadExperiment() {
    auto* button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    int row = button->property("row").toInt();
    if (row < 0 || row >= static_cast<int>(fcm->experiments.size())) {
        return;
    }

    bool canSaveCurrentState = true;
    for (const auto& [_, concept] : fcm->concepts) {
        if (!concept->term) {
            canSaveCurrentState = false;
            break;
        }
    }
    if (canSaveCurrentState) {
        for (const auto& [_, weight] : fcm->weights) {
            if (!weight->term) {
                canSaveCurrentState = false;
                break;
            }
        }
    }

    if (canSaveCurrentState) {
        auto saveCurrentStateWindow = SaveCurrentStateWindow(this);
        if (saveCurrentStateWindow.exec() != QDialog::Accepted) {
            return;
        }
        bool saveCurrentState = saveCurrentStateWindow.saveCurrentState();

        if (saveCurrentState) {
            createExperiment();
        }
    }

    fcm->terms.clear();
    fcm->concepts.clear();
    fcm->weights.clear();
    fcm->predictionParameters = fcm->experiments[row].predictionParameters;
    for (const auto& [id, term] : fcm->experiments[row].terms) {
        fcm->terms[id] = std::make_shared<Term>(*term);
        fcm->terms[id]->dbId = -1;
        if (fcm->experiments.back().terms.find(id) != fcm->experiments.back().terms.end()) {
            fcm->terms[id]->description = fcm->experiments.back().terms[id]->description;
        }
    }
    for (const auto& [id, concept] : fcm->experiments[row].concepts) {
        fcm->concepts[id] = std::make_shared<Concept>(*concept);
        fcm->concepts[id]->term = fcm->terms[concept->term->id];
        fcm->concepts[id]->dbId = -1;
        if (fcm->experiments.back().concepts.find(id) != fcm->experiments.back().concepts.end()) {
            fcm->concepts[id]->description = fcm->experiments.back().concepts[id]->description;
        }
    }
    for (const auto& [id, weight] : fcm->experiments[row].weights) {
        fcm->weights[id] = std::make_shared<Weight>(*weight);
        fcm->weights[id]->term = fcm->terms[weight->term->id];
        fcm->weights[id]->dbId = -1;
        if (fcm->experiments.back().weights.find(id) != fcm->experiments.back().weights.end()) {
            fcm->weights[id]->description = fcm->experiments.back().weights[id]->description;
        }
    }

    loadFCM(fcm);
}

void MainWindow::onDeleteExperiment() {
    auto* button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    int row = button->property("row").toInt();
    if (row < 0 || row >= static_cast<int>(fcm->experiments.size())) {
        return;
    }

    if (fcm->experiments[row].dbId != -1) {
        fcm->deletedExperimentsIds.push_back(fcm->experiments[row].dbId);
    }

    fcm->experiments.erase(fcm->experiments.begin() + row);
    ui->experimantsTable->model()->removeRows(0, ui->experimantsTable->model()->rowCount());

    for (const auto& experiment : fcm->experiments) {
        addExperiment(experiment);
    }
    autosave();
}

void MainWindow::updateFCM() {
    fcm->name = ui->modelName->text();
    fcm->description = ui->modelNotes->markdownText();
    fcm->predictionParameters = getPredictionParameters();
}

PredictionParameters MainWindow::getPredictionParameters() {
    return PredictionParameters{
        ui->comboBoxAlgorithm->currentData(Qt::UserRole).toString(),
        ui->useFuzzyValues->isChecked(),
        ui->comboBoxActivation->currentData(Qt::UserRole).toString(),
        ui->comboBoxMetric->currentData(Qt::UserRole).toString(),
        ui->checkBoxPredictToStatic->isChecked(),
        ui->doubleSpinBoxThreshold->value(),
        ui->spinBoxMetricSteps->value(),
        ui->spinBoxFixedSteps->value()
    };
}

void MainWindow::saveAs() {
    updateFCM();

    const auto modelsNames = savingManager->getModelsNames();
    SaveAsWindow saveAsWindow(modelsNames, fcm->name, MainWindow::tr("Save FCM"), this);

    if (saveAsWindow.exec() == QDialog::Accepted) {
        QString newName = saveAsWindow.savingModelName();
        fcm->name = newName;
        savingManager->saveAs(*fcm);
        ui->modelName->setText(newName);
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
    qDeleteAll(conceptsGroup->takeChildren());
    qDeleteAll(weightsGroup->takeChildren());

    fcm = newFCM;

    if (activeSimulation) {
        resetPredictionScene();
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
    presenter = std::make_shared<SimulationPresenter>(creationPresenter);
    connect(&*presenter, &SimulationPresenter::updateProgress, this, &MainWindow::updateProgress);
    connect(&*presenter, &SimulationPresenter::finished, this, &MainWindow::simulationFinished);
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

    ui->graphicsViewGraph->resetTransform();
    ui->graphicsViewPredict->resetTransform();
    ui->graphicsViewSensitivity->resetTransform();
    updateGraphScaleLabel(1.0);
    updatePredictScaleLabel(1.0);
    updateSensitivityScaleLabel(1.0);

    delete staticAnalysisPresenter;
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, creationPresenter, fcm);

    ui->experimantsTable->model()->removeRows(0, ui->experimantsTable->model()->rowCount());

    for (const auto& experiment : fcm->experiments) {
        addExperiment(experiment);
    }

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

    ui->actionAutoSave->setChecked(fcm->autosaveOn);
}

void MainWindow::open() {
    const auto modelsNames = savingManager->getModelsNames();

    LoadModelWindow* loadModelWindow = new LoadModelWindow(modelsNames, MainWindow::tr("Open FCM"), this);

    if (loadModelWindow->exec() != QDialog::Accepted) {
        return;
    }
    QString modelName = loadModelWindow->selectedModelName();
    if (modelName.isEmpty()) {
        return;
    }

    auto model = savingManager->getFCM(modelName);
    if (!model) {
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

    const auto templatesNames = templatesManager->getTemplatesNames();
    SaveAsWindow saveAsWindow(templatesNames, fcm->name, MainWindow::tr("Save FCM Template"), this);

    if (saveAsWindow.exec() == QDialog::Accepted) {
        fcm->name = saveAsWindow.savingModelName();
        templatesManager->createTemplate(*fcm);
        ui->modelName->setText(fcm->name);
    }
}

void MainWindow::openTemplate() {
    const auto templatesNames = templatesManager->getTemplatesNames();
    LoadModelWindow* loadModelWindow = new LoadModelWindow(templatesNames, MainWindow::tr("Open FCM Template"), this);

    if (loadModelWindow->exec() != QDialog::Accepted) {
        return;
    }
    QString templateName = loadModelWindow->selectedModelName();
    if (templateName.isEmpty()) {
        return;
    }

    auto model = templatesManager->getFCM(templateName);
    if (!model) {
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
    QString fileName = QFileDialog::getSaveFileName(
        this,
        MainWindow::tr("Save FCM Model"),
        "",
        "JSON files (*.json)"
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".json"))
        fileName += ".json";

    updateFCM();
    if (!JsonRepository::exportToJson(*fcm, fileName))
    {
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
    actions.push_back(ui->menuModels->addAction(fcm->name));
    actions[currentModelIdx]->setData(currentModelIdx);
    connect(actions[currentModelIdx], &QAction::triggered, this, &MainWindow::switchModel);
}

void MainWindow::switchModel() {
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }
    size_t index = static_cast<size_t>(action->data().toULongLong());
    if (index >= fcms.size()) {
        return;
    }
    fcm = fcms[index];
    loadFCM(fcm);
}

void MainWindow::nameChanged(QString newName) {
    actions[currentModelIdx]->setText(newName);
    fcm->name = newName;
    autosave();
}

void MainWindow::createNewModel() {
    fcm = std::make_shared<FCM>();
    fcm->name = MainWindow::tr("New model");
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
    const auto templatesNames = templatesManager->getTemplatesNames();

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
        conceptsGroup->setText(0, tr("Concepts terms"));
        weightsGroup->setText(0, tr("Weights terms"));
        ui->plotSensitivity->xAxis->setLabel(tr("max change"));
        ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
        ui->plotSensitivity->replot();

        if (auto* experimentsModel = qobject_cast<QStandardItemModel*>(ui->experimantsTable->model())) {
            experimentsModel->setHorizontalHeaderLabels({
                tr("Algorithm"),
                tr("Value type"),
                tr("Activation function"),
                tr("Metric"),
                tr("Predict to static"),
                tr("Threshold"),
                tr("Steps less threshold"),
                tr("Fixed steps"),
                tr("Timestamp"),
                "",
                ""
            });
        }

        ui->plotSensitivity->xAxis->setLabel(tr("max change"));
        ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
        ui->labelScaleGraph->setText(QString(MainWindow::tr("Scale: %1%")).arg(graphScale*100, 0, 'f', 2));
        ui->labelScalePredict->setText(QString(MainWindow::tr("Scale: %1%")).arg(predictScale*100, 0, 'f', 2));
        ui->labelScaleSensitivity->setText(QString(MainWindow::tr("Scale: %1%")).arg(sensitivityScale*100, 0, 'f', 2));
        ui->pushButtonMode->setText(editMode == EditMode::EditValues ? MainWindow::tr("Mode: Edit values") : MainWindow::tr("Mode: Create"));
        if (paused) {
             ui->pushButtonPause->setText(MainWindow::tr("Resume"));
        } else {
            ui->pushButtonPause->setText(MainWindow::tr("Pause"));
        }
        ui->labelMetricValue->setText(QString(MainWindow::tr("Metric value: %1")).arg(currentMetricValue, 0, 'f', 4));
        if (sensitivityPlotShown) {
            ui->showSensitivityPlot->setText(MainWindow::tr("Elements Sensitivity"));
        } else {
            ui->showSensitivityPlot->setText(MainWindow::tr("FCM Sensitivity"));
        }

        auto* experimentsModel = ui->experimantsTable->model();
        for (int row = 0; row < experimentsModel->rowCount(); ++row) {
            if (auto* loadButton = qobject_cast<QPushButton*>(ui->experimantsTable->indexWidget(ui->experimantsTable->model()->index(row, 9)))) {
                loadButton->setText(tr("Load"));
            }
            if (auto* deleteButton = qobject_cast<QPushButton*>(ui->experimantsTable->indexWidget(ui->experimantsTable->model()->index(row, 10)))) {
                deleteButton->setText(tr("Delete"));
            }

            QModelIndex idx = ui->experimantsTable->model()->index(row, 0);
            ui->experimantsTable->model()->setData(idx, MainWindow::tr(fcm->experiments[row].predictionParameters.algorithm.toUtf8().constData()));

            experimentsModel->setData(experimentsModel->index(row, 1), fcm->experiments[row].predictionParameters.useFuzzyValues ? tr("fuzzy") : tr("numeric"));

            idx = ui->experimantsTable->model()->index(row, 2);
            ui->experimantsTable->model()->setData(idx, MainWindow::tr(fcm->experiments[row].predictionParameters.activationFunction.toUtf8().constData()));
            experimentsModel->setData(experimentsModel->index(row, 4), fcm->experiments[row].predictionParameters.predictToStatic ? tr("yes") : tr("no"));
        }
    }

    QMainWindow::changeEvent(event);
}
