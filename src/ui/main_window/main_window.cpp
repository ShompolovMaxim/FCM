#include "main_window.h"
#include "ui_main_window.h"

#include "ui/graph_editor/graph_scene.h"
#include "ui/save_as_window/save_as_window.h"
#include "ui/load_model_window/load_model_window.h"

#include "repository/json_repository.h"
#include "repository/migration_manager.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("models.db");
    if (!db.open()) {
        qFatal("Cannot open database");
    }
    MigrationManager::migrate(db);
    savingManager = std::make_shared<SavingManager>(ModelsRepository(db));

    fcm = std::make_shared<FCM>();

    creationPresenter = std::make_shared<CreationPresenter>(fcm, nullptr);
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
    conceptsGroup->setText(0, "Concepts terms");
    weightsGroup = new QTreeWidgetItem(ui->treeWidgetTerms);
    weightsGroup->setText(0, "Weights terms");
    connect(ui->treeWidgetTerms, &QTreeWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);
    connect(ui->treeWidgetTerms, &QTreeWidget::itemChanged, this, &MainWindow::onItemChanged);

    QStandardItemModel* experimentsModel = new QStandardItemModel();
    experimentsModel->setHorizontalHeaderLabels({"Algorithm", "Activation function", "Metric", "Predict to static", "Threshold", "Steps less threshold", "Fixed steps", "Timestamp", ""});
    ui->experimantsTable->setModel(experimentsModel);
    ui->experimantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
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
    ui->plotSensitivity->xAxis->setLabel("max change");
    ui->plotSensitivity->yAxis->setLabel("sensitivity");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateGraphScaleLabel(double newScale) {
    ui->labelScaleGraph->setText(QString("Scale: %1%").arg(newScale*100, 0, 'f', 2));
}

void MainWindow::updatePredictScaleLabel(double newScale) {
    ui->labelScalePredict->setText(QString("Scale: %1%").arg(newScale*100, 0, 'f', 2));
}

void MainWindow::updateModeButtonText(EditMode newMode) {
    ui->pushButtonMode->setText(newMode == EditMode::EditValues ? "Mode: Edit values" : "Mode: Create");
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
    QPushButton* btn = new QPushButton("Load", ui->experimantsTable);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 8), btn);
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
        ui->pushButtonPause->setText("Resume");
        presenter->pause();
    } else {
        ui->pushButtonPause->setText("Pause");
        presenter->resume();
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
        QMessageBox::critical(this, "Error", "Value of step is not calculated or step out of range!");
    }
}

void MainWindow::stepBack() {
    if (!presenter->moveStep(-ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(this, "Error", "Value of step is not calculated or step out of range!");
    }
}

void MainWindow::simulationFinished() {
    ui->pushButtonPredict->setEnabled(true);
    ui->doubleSpinBoxStepsPerSecond->setEnabled(true);
}

void MainWindow::updateProgress(size_t value, size_t maxStep, double metricValue) {
    ui->progressBarPredict->setMaximum(maxStep);
    ui->progressBarPredict->setValue(value);
    ui->labelMetricValue->setText(QString("Metric value: %1").arg(metricValue, 0, 'f', 4));
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
        ui->showSensitivityPlot->setText("FCM Sensitivity");
    } else {
        ui->graphicsViewSensitivity->hide();
        ui->plotSensitivity->show();
        ui->showSensitivityPlot->setText("Elements Sensitivity");
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
    fcm->terms[id]->name = "New term";
    fcm->terms[id]->type = type;

    if (type == ElementType::Node) {
        fcm->terms[id]->color = ColorValueAdapter().getColor(fcm->terms[id]->value, 0, 1);
    } else {
        fcm->terms[id]->color = ColorValueAdapter().getColor(fcm->terms[id]->value, -1, 1);
    }


    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, "New term");
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
    QColor color = QColorDialog::getColor(Qt::white, this, QString("Choose term %1 color").arg(fcm->terms[currentTermId]->name));
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
    SaveAsWindow saveAsWindow(modelsNames, fcm->name, this);

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

void MainWindow::loadFCM(const FCM& newFCM) {
    *fcm = newFCM;

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

    for (const auto& experiment : fcm->experiments) {
        addExperiment(experiment);
    }

    ui->comboBoxAlgorithm->setCurrentText(fcm->predictionParameters.algorithm);
    ui->comboBoxActivation->setCurrentText(fcm->predictionParameters.activationFunction);
    ui->comboBoxMetric->setCurrentText(fcm->predictionParameters.metric);
    ui->checkBoxPredictToStatic->setChecked(fcm->predictionParameters.predictToStatic);
    ui->doubleSpinBoxThreshold->setValue(fcm->predictionParameters.threshold);
    ui->spinBoxMetricSteps->setValue(fcm->predictionParameters.stepsLessThreshold);
    ui->spinBoxFixedSteps->setValue(fcm->predictionParameters.fixedSteps);
}

void MainWindow::open() {
    const auto modelsNames = savingManager->getModelsNames();

    LoadModelWindow* loadModelWindow = new LoadModelWindow(modelsNames, this);

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

    loadFCM(*model);
}

void MainWindow::onExportPng()
{
    QString proposedName = "fcm.png";
    if (!ui->modelName->text().isEmpty()) {
        proposedName = ui->modelName->text() + ".png";
    }
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export as PNG"),
        proposedName,
        tr("PNG Images (*.png)")
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
        fileName += ".png";

    QPixmap pixmap = ui->graphicsViewGraph->grab();

    if (!pixmap.save(fileName, "PNG")) {
        QMessageBox::warning(this, tr("Error"), tr("Unable to save PNG to the selected file!"));
    }
}

void MainWindow::onExportJson() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save FCM Model",
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
        QMessageBox::warning(this, "Error", "Failed to save file.");
    }
}

void MainWindow::onImportJson() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open FCM Model",
        "",
        "JSON files (*.json)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    auto model = JsonRepository::importFromJson(fileName);

    if (!model) {
        QMessageBox::warning(this, "Error", "Failed to load file.");
    }

    loadFCM(*model);
}
