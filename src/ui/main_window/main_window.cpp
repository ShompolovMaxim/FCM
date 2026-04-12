#include "main_window.h"
#include "ui_main_window.h"

#include "ui/join_window/join_window.h"
#include "ui/graph_editor/graph_scene.h"
#include "ui/save_as_window/save_as_window.h"
#include "ui/load_model_window/load_model_window.h"

#include "repository/json_repository.h"
#include "repository/migration_manager.h"

#include "model/join/models_joiner.h"

#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    auto lang = settings.value("language", "").toString();
    translatorRus.load("FCM_ru_RU.qm");
    if (lang.isEmpty()) {
        ui->actionEnglish->setChecked(true);
    } else {
        ui->actionRussian->setChecked(true);
        qApp->installTranslator(&translatorRus);
        ui->retranslateUi(this);
    }
    connect(ui->actionRussian, &QAction::triggered, this, &MainWindow::setRussian);
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::setEnglish);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("models.db");
    if (!db.open()) {
        qFatal("Cannot open database");
    }
    MigrationManager::migrate(db);
    savingManager = std::make_shared<SavingManager>(ModelsRepository(db));
    templatesManager = std::make_shared<TemplatesManager>(TemplatesRepository(db));

    fcm = std::make_shared<FCM>();

    creationPresenter = std::make_shared<CreationPresenter>(fcm, nullptr);
    connect(creationPresenter.get(), &CreationPresenter::autosave, this, &MainWindow::autosave);
    ui->adjacencyTableView->setPresenter(creationPresenter);
    presenter = std::make_shared<SimulationPresenter>(creationPresenter, nullptr);

    auto* scene = new GraphScene(fcm, creationPresenter);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);
    ui->graphicsViewSensitivity->setScene(scene);

    auto* staticAnalysisScene = new GraphScene(fcm, creationPresenter);
    staticAnalysisScene->setMode(EditMode::EditValues);
    staticAnalysisScene->blockConceptCreationColorEdit(true);
    ui->staticAnalysis->findChild<GraphView*>("graphicsView")->setScene(staticAnalysisScene);

    QObject::connect(ui->pushButtonMode, &QPushButton::clicked, scene, &GraphScene::switchMode);
    QObject::connect(scene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    QObject::connect(ui->graphicsViewGraph, &GraphView::scaleChanged, this, &MainWindow::updateGraphScaleLabel);
    QObject::connect(ui->graphicsViewPredict, &GraphView::scaleChanged, this, &MainWindow::updatePredictScaleLabel);
    QObject::connect(ui->pushButtonScaleGraph, &QPushButton::clicked, ui->graphicsViewGraph, &GraphView::resetScale);
    QObject::connect(ui->pushButtonScalePredict, &QPushButton::clicked, ui->graphicsViewPredict, &GraphView::resetScale);
    QObject::connect(ui->pushButtonPredict, &QPushButton::clicked, this, &MainWindow::predict);
    QObject::connect(ui->pushButtonReset, &QPushButton::clicked, this, &MainWindow::resetPredictionScene);
    QObject::connect(ui->pushButtonPause, &QPushButton::clicked, this, &MainWindow::pauseResumePrediction);
    QObject::connect(ui->pushButtonSpeedUp, &QPushButton::clicked, this, &MainWindow::speedUp);
    QObject::connect(ui->pushButtonSlowDown, &QPushButton::clicked, this, &MainWindow::slowDown);
    QObject::connect(ui->pushButtonForward, &QPushButton::clicked, this, &MainWindow::stepForward);
    QObject::connect(ui->pushButtonBack, &QPushButton::clicked, this, &MainWindow::stepBack);
    QObject::connect(&*presenter, &SimulationPresenter::updateProgress, this, &MainWindow::updateProgress);
    QObject::connect(&*presenter, &SimulationPresenter::finished, this, &MainWindow::simulationFinished);
    QObject::connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, this, &MainWindow::onPredictToStaticChanged);

    connect(ui->createTermButton, &QPushButton::clicked, this, &MainWindow::onCreateTerm);
    connect(ui->deleteTermButton, &QPushButton::clicked, this, &MainWindow::onDeleteTerm);
    connect(ui->termColorButton, &QPushButton::clicked, this, &MainWindow::onChooseTermColor);
    connect(ui->termValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueChanged);
    connect(ui->termValueL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueLChanged);
    connect(ui->termValueM, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueMChanged);
    connect(ui->termValueU, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueUChanged);

    conceptsGroup = new QTreeWidgetItem(ui->treeWidgetTerms);
    conceptsGroup->setText(0, tr("Concepts terms"));
    weightsGroup = new QTreeWidgetItem(ui->treeWidgetTerms);
    weightsGroup->setText(0, tr("Weights terms"));
    connect(ui->treeWidgetTerms, &QTreeWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);
    connect(ui->treeWidgetTerms, &QTreeWidget::itemChanged, this, &MainWindow::onItemChanged);

    QStandardItemModel* experimentsModel = new QStandardItemModel();
    experimentsModel->setHorizontalHeaderLabels({tr("Algorithm"), tr("Activation function"), tr("Metric"), tr("Predict to static"), tr("Threshold"), tr("Steps less threshold"), tr("Fixed steps"), tr("Timestamp"), "", ""});
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
    connect(ui->showSensitivityPlot, &QPushButton::clicked, this, &MainWindow::showSensitivityPlot);
    ui->plotSensitivity->hide();
    ui->plotSensitivity->addGraph();
    ui->plotSensitivity->yAxis->setRange(-0.1, 1.1);
    ui->plotSensitivity->xAxis->setLabel(tr("max change"));
    ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));

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

    QObject::connect(ui->modelName, &QLineEdit::textChanged, this, &MainWindow::nameChanged);
    addFCM(fcm);
    ui->modelName->setText(tr("New model"));
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::createNewModel);

    connect(ui->actionJoinFCM, &QAction::triggered, this, &MainWindow::joinModels);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateGraphScaleLabel(double newScale) {
    ui->labelScaleGraph->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    graphScale = newScale;
}

void MainWindow::updatePredictScaleLabel(double newScale) {
    ui->labelScalePredict->setText(QString(MainWindow::tr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    predictScale = newScale;
}

void MainWindow::updateModeButtonText(EditMode newMode) {
    ui->pushButtonMode->setText(newMode == EditMode::EditValues ? MainWindow::tr("Mode: Edit values") : MainWindow::tr("Mode: Create"));
    editMode = newMode;

}

void MainWindow::addExperiment(const Experiment& experiment) {
    auto experimentsModel = ui->experimantsTable->model();
    int row = experimentsModel->rowCount();
    experimentsModel->insertRow(row);
    experimentsModel->setData(experimentsModel->index(row, 0), experiment.predictionParameters.algorithm);
    experimentsModel->setData(experimentsModel->index(row, 1), experiment.predictionParameters.activationFunction);
    experimentsModel->setData(experimentsModel->index(row, 2), experiment.predictionParameters.metric);
    experimentsModel->setData(experimentsModel->index(row, 3), experiment.predictionParameters.predictToStatic);
    experimentsModel->setData(experimentsModel->index(row, 4), experiment.predictionParameters.threshold);
    experimentsModel->setData(experimentsModel->index(row, 5), experiment.predictionParameters.stepsLessThreshold);
    experimentsModel->setData(experimentsModel->index(row, 6), experiment.predictionParameters.fixedSteps);
    experimentsModel->setData(experimentsModel->index(row, 7), experiment.timestamp);
    experimentsModel->setData(experimentsModel->index(row, 8), "");
    experimentsModel->setData(experimentsModel->index(row, 9), "");
    QPushButton* btn = new QPushButton(tr("Load"), ui->experimantsTable);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 8), btn);
    QPushButton* deleteButton = new QPushButton(tr("Delete"), ui->experimantsTable);
    deleteButton->setProperty("row", row);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 9), deleteButton);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteExperiment);
    QObject::connect(btn, &QPushButton::clicked, [row]() {
        qDebug() << "Нажата кнопка в строке" << row;
    });

}

void MainWindow::predict() {
    ui->pushButtonPredict->setEnabled(false);
    ui->doubleSpinBoxStepsPerSecond->setEnabled(false);

    auto predictionParameters = getPredictionParameters();

    auto simulationParameters = SimulationParameters{
        ui->checkBoxRealTime->isChecked(),
        ui->doubleSpinBoxStepsPerSecond->value()
    };

    Experiment experiment;
    for (auto& [id, term] : fcm->terms) {
        experiment.terms[id] = std::make_shared<Term>(*term);
        experiment.terms[id]->dbId = -1;
    }
    for (const auto& [id, concept] : fcm->concepts) {
        experiment.concepts[id] = std::make_shared<Concept>(*concept);
        experiment.concepts[id]->dbId = -1;
        experiment.concepts[id]->term = experiment.terms[fcm->concepts[id]->term->id];
    }
    for (const auto& [id, weight] : fcm->weights) {
        experiment.weights[id] = std::make_shared<Weight>(*weight);
        experiment.weights[id]->term = experiment.terms[fcm->weights[id]->term->id];
    }
    experiment.predictionParameters = predictionParameters;
    experiment.timestamp = QDateTime::currentDateTime();
    fcm->experiments.push_back(experiment);
    addExperiment(experiment);
    autosave();

    auto* predictScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy();
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

    presenter->simulate(predictionParameters, simulationParameters, nodes, edges, fcm);
}

void MainWindow::resetPredictionScene() {
    simulationFinished();
    auto* predictionScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewPredict->setScene(ui->graphicsViewGraph->scene());
    delete predictionScene;
}

void MainWindow::pauseResumePrediction() {
    if (ui->pushButtonPause->text() == "Pause") {
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

void MainWindow::simulationFinished() {
    ui->pushButtonPredict->setEnabled(true);
    ui->doubleSpinBoxStepsPerSecond->setEnabled(true);
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
    auto* sensitivityScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy();
    auto* oldSensitivityScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewSensitivity->setScene(sensitivityScene);
    if (oldSensitivityScene != ui->graphicsViewGraph->scene()) {
        delete oldSensitivityScene;
    }

    sensitivityPresenter = std::make_shared<SensitivityPresenter>(sensitivityScene, ui->plotSensitivity, creationPresenter, nullptr);
    connect(sensitivityPresenter.get(), &SensitivityPresenter::updateProgress, this, &MainWindow::updateSensitivityProgress);
    sensitivityPresenter->analize(getPredictionParameters(), getSensitivityParameters(), fcm);
}

void MainWindow::showSensitivityPlot() {
    if (sensitivityPlotShown) {
        ui->plotSensitivity->hide();
        ui->graphicsViewSensitivity->show();
        ui->showSensitivityPlot->setText(MainWindow::tr("FCM Sensitivity"));
    } else {
        ui->graphicsViewSensitivity->hide();
        ui->plotSensitivity->show();
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
    }
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    ui->createTermButton->setEnabled(current);

    QSignalBlocker b1(ui->termValue);
    QSignalBlocker b2(ui->termValueL);
    QSignalBlocker b3(ui->termValueM);
    QSignalBlocker b4(ui->termValueU);

    if (!current || !current->parent()) {
        ui->termValue->setValue(0);
        ui->termValueL->setValue(0);
        ui->termValueM->setValue(0);
        ui->termValueU->setValue(0);
        ui->termValue->setEnabled(false);
        ui->termValueL->setEnabled(false);
        ui->termValueM->setEnabled(false);
        ui->termValueU->setEnabled(false);
        ui->deleteTermButton->setEnabled(false);
        ui->termColorButton->setEnabled(false);
        ui->termColorButton->setStyleSheet("");
        updateFuzzyValuePlot();
        return;
    }

    ui->termValue->setEnabled(true);
    ui->termValueL->setEnabled(true);
    ui->termValueM->setEnabled(true);
    ui->termValueU->setEnabled(true);
    ui->deleteTermButton->setEnabled(true);
    ui->termColorButton->setEnabled(true);
    currentTermId = current->data(0, Qt::UserRole).toUuid();

    ui->termValue->setValue(fcm->terms[currentTermId]->value);
    ui->termValueL->setValue(fcm->terms[currentTermId]->fuzzyValue.l);
    ui->termValueM->setValue(fcm->terms[currentTermId]->fuzzyValue.m);
    ui->termValueU->setValue(fcm->terms[currentTermId]->fuzzyValue.u);
    ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(fcm->terms[currentTermId]->color.name()));
    updateFuzzyValuePlot();
}

void MainWindow::onTermValueChanged(double value) {
    fcm->terms[currentTermId]->value = value;
    if (ui->autoColorConfiguration->checkState()) {
        if (fcm->terms[currentTermId]->type == ElementType::Node) {
            fcm->terms[currentTermId]->color = ColorValueAdapter().getColor(value, 0, 1);
        } else {
            fcm->terms[currentTermId]->color = ColorValueAdapter().getColor(value, -1, 1);
        }
        ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(fcm->terms[currentTermId]->color.name()));
    }
    creationPresenter->updateTerm(currentTermId);
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
    creationPresenter->updateTerm(currentTermId);
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
    creationPresenter->updateTerm(currentTermId);
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
    creationPresenter->updateTerm(currentTermId);
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
}

void MainWindow::updateFCM() {
    fcm->name = ui->modelName->text();
    fcm->description = ui->modelNotes->toPlainText();
    fcm->predictionParameters = getPredictionParameters();
}

PredictionParameters MainWindow::getPredictionParameters() {
    return PredictionParameters{
        ui->comboBoxAlgorithm->currentText(),
        ui->useFuzzyValues->isChecked(),
        ui->comboBoxActivation->currentText(),
        ui->comboBoxMetric->currentText(),
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
    fcm = newFCM;

    ui->modelName->setText(fcm->name);
    ui->modelNotes->setPlainText(fcm->description);

    qDeleteAll(conceptsGroup->takeChildren());
    qDeleteAll(weightsGroup->takeChildren());

    QTreeWidgetItem* firstItem = nullptr;

    for (auto& [id, term] : fcm->terms) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, term->name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(id));
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if (term->type == ElementType::Node) {
            conceptsGroup->addChild(item);
        } else {
            weightsGroup->addChild(item);
        }

        if (!firstItem) firstItem = item;
    }

    ui->treeWidgetTerms->expandAll();

    creationPresenter = std::make_shared<CreationPresenter>(fcm);
    connect(creationPresenter.get(), &CreationPresenter::autosave, this, &MainWindow::autosave);
    auto newScene = new GraphScene(fcm, creationPresenter);
    QObject::connect(ui->pushButtonMode, &QPushButton::clicked, newScene, &GraphScene::switchMode);
    QObject::connect(newScene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
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
    auto* newStaticAnalysisScene = new GraphScene(fcm, creationPresenter);
    newStaticAnalysisScene->blockConceptCreationColorEdit(true);
    newStaticAnalysisScene->setMode(EditMode::EditValues);
    ui->staticAnalysis->findChild<GraphView*>("graphicsView")->setScene(newStaticAnalysisScene);
    delete oldStaticAnalysisScene;

    delete staticAnalysisPresenter;
    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, creationPresenter, fcm);

    ui->experimantsTable->model()->removeRows(0, ui->experimantsTable->model()->rowCount());

    for (const auto& experiment : fcm->experiments) {
        addExperiment(experiment);
    }

    ui->comboBoxAlgorithm->setCurrentText(fcm->predictionParameters.algorithm);
    ui->useFuzzyValues->setChecked(fcm->predictionParameters.useFuzzyValues);
    ui->comboBoxActivation->setCurrentText(fcm->predictionParameters.activationFunction);
    ui->comboBoxMetric->setCurrentText(fcm->predictionParameters.metric);
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

    loadFCM(std::make_shared<FCM>(*model));
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
        QMessageBox::warning(this, MainWindow::tr("Error"), MainWindow::tr("Unable to save PNG to the selected file!"));
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
        QMessageBox::warning(this, MainWindow::tr("Error"), MainWindow::tr("Failed to save file."));
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
        QMessageBox::warning(this, MainWindow::tr("Error"), MainWindow::tr("Failed to load file."));
    }

    loadFCM(std::make_shared<FCM>(*model));
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
        if (fcm->dbId == -1) {
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
                break;
            }
        }
        if (modelName == joinWindow->getTermsModel()) {
            baseFCM = joinFCMs.back();
        }
    }

    for (const auto& modelName : joinWindow->getModelsToJoin().value(JoinGroupType::Saved)) {
        joinFCMs.push_back(std::make_shared<FCM>(*savingManager->getFCM(modelName)));
        if (modelName == joinWindow->getTermsModel()) {
            baseFCM = joinFCMs.back();
        }
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
    creationPresenter->retranslateElementsWindows();
    settings.setValue("language", "");
}

void MainWindow::setRussian() {
    QSignalBlocker b1(ui->actionEnglish);
    QSignalBlocker b2(ui->actionRussian);
    ui->actionRussian->setChecked(true);
    ui->actionEnglish->setChecked(false);
    qApp->installTranslator(&translatorRus);
    creationPresenter->retranslateElementsWindows();
    settings.setValue("language", "RU");
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

        for (int row = 0; row < ui->experimantsTable->model()->rowCount(); ++row) {
            if (auto* loadButton = qobject_cast<QPushButton*>(ui->experimantsTable->indexWidget(ui->experimantsTable->model()->index(row, 8)))) {
                loadButton->setText(tr("Load"));
            }
            if (auto* deleteButton = qobject_cast<QPushButton*>(ui->experimantsTable->indexWidget(ui->experimantsTable->model()->index(row, 9)))) {
                deleteButton->setText(tr("Delete"));
            }
        }
    }

    QMainWindow::changeEvent(event);
}
