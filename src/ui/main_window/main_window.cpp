#include "main_window.h"
#include "ui_main_window.h"

#include "ui/graph_editor/graph_scene.h"
#include "ui/load_model_window/load_model_window.h"

#include "repository/json_repository.h"
#include "repository/migration_manager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("models.db");
    if (!db.open()) {
        qFatal("Cannot open database");
    }
    MigrationManager::migrate(db);
    modelsRepository = std::make_shared<ModelsRepository>(db);

    fcm = std::make_shared<FCM>();

    auto* scene = new GraphScene(fcm);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);

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
    QObject::connect(&presenter, &SimulationPresenter::updateProgress, this, &MainWindow::updateProgress);
    QObject::connect(&presenter, &SimulationPresenter::finished, this, &MainWindow::simulationFinished);
    QObject::connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, this, &MainWindow::onPredictToStaticChanged);

    ui->listWidgetTerms->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidgetTerms, &QListWidget::customContextMenuRequested, this, &MainWindow::onListWidgetContextMenu);
    ui->listWidgetTerms->setDragEnabled(true);
    ui->listWidgetTerms->setAcceptDrops(true);
    ui->listWidgetTerms->setDropIndicatorShown(true);
    ui->listWidgetTerms->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->listWidgetTerms, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);

    connect(ui->termValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueChanged);
    connect(ui->termValueL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueLChanged);
    connect(ui->termValueM, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueMChanged);
    connect(ui->termValueU, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueUChanged);
    connect(ui->listWidgetTerms, &QListWidget::itemChanged, this, &MainWindow::onItemChanged);

    QStandardItemModel* experimentsModel = new QStandardItemModel();
    experimentsModel->setHorizontalHeaderLabels({"Algorithm", "Activation function", "Metric", "Predict to static", "Threshold", "Steps less threshold", "Fixed steps", "Timestamp", ""});
    ui->experimantsTable->setModel(experimentsModel);
    ui->experimantsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->experimantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionExportPNG, &QAction::triggered, this, &MainWindow::onExportPng);
    connect(ui->actionExportJSON, &QAction::triggered, this, &MainWindow::onExportJson);
    connect(ui->actionImport, &QAction::triggered, this, &MainWindow::onImportJson);

    ui->fuzzyValuePlot->xAxis->setRange(-1.1, 1.1);
    ui->fuzzyValuePlot->yAxis->setRange(0, 1);
    ui->fuzzyValuePlot->xAxis->setLabel("x");
    ui->fuzzyValuePlot->yAxis->setLabel("μ(x)");
    ui->fuzzyValuePlot->addGraph();

    staticAnalysisPresenter = new StaticAnalysisPresenter(ui->staticAnalysis, fcm);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index)
            {
                if (ui->tabWidget->widget(index) == ui->staticAnalysis)
                    staticAnalysisPresenter->update();
            });
}

MainWindow::~MainWindow()
{
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
    experiment.terms = fcm->terms;
    experiment.concepts = fcm->concepts;
    experiment.weights = fcm->weights;
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
    for (QGraphicsItem* item : predictScene->items()) {
        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            nodes.append(n);
        }
    }

    ui->progressBarPredict->setMaximum(predictionParameters.fixedSteps);

    presenter.simulate(predictionParameters, simulationParameters, nodes);
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
        presenter.pause();
    } else {
        ui->pushButtonPause->setText("Pause");
        presenter.resume();
    }
}

void MainWindow::speedUp() {
    presenter.speedUp();
    ui->doubleSpinBoxStepsPerSecond->setValue(presenter.getStepsPerSecond());
}

void MainWindow::slowDown() {
    presenter.slowDown();
    ui->doubleSpinBoxStepsPerSecond->setValue(presenter.getStepsPerSecond());
}

void MainWindow::stepForward() {
    if (!presenter.moveStep(ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(this, "Error", "Value of step is not calculated or step out of range!");
    }
}

void MainWindow::stepBack() {
    if (!presenter.moveStep(-ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(this, "Error", "Value of step is not calculated or step out of range!");
    }
}

void MainWindow::simulationFinished() {
    ui->pushButtonPredict->setEnabled(true);
    ui->doubleSpinBoxStepsPerSecond->setEnabled(true);
}

void MainWindow::updateProgress(size_t value) {
    ui->progressBarPredict->setValue(value+1);
}

void MainWindow::onListWidgetContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->listWidgetTerms->itemAt(pos);

    QMenu menu(this);

    QAction *addAction = menu.addAction("Add");
    QAction *deleteAction = menu.addAction("Delete");

    QAction *selectedAction = menu.exec(ui->listWidgetTerms->mapToGlobal(pos));

    if (selectedAction == addAction) {
        QListWidgetItem *newItem = new QListWidgetItem("New term");
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
        terms[newItem];
        terms[newItem].name = "New term";
        terms[newItem].id = ++fcm->termsCounter;
        fcm->terms[terms[newItem].id] = terms[newItem];

        ui->listWidgetTerms->addItem(newItem);

        ui->listWidgetTerms->setCurrentItem(newItem);
        ui->listWidgetTerms->editItem(newItem);
    }
    if (selectedAction == deleteAction && item) {
        delete item;
    }
}

void MainWindow::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->termValue->setEnabled(true);
    ui->termValueL->setEnabled(true);
    ui->termValueM->setEnabled(true);
    ui->termValueU->setEnabled(true);
    currentTerm = current;

    QSignalBlocker b1(ui->termValue);
    QSignalBlocker b2(ui->termValueL);
    QSignalBlocker b3(ui->termValueM);
    QSignalBlocker b4(ui->termValueU);

    ui->termValue->setValue(terms[current].value);
    ui->termValueL->setValue(terms[current].fuzzyValueL);
    ui->termValueM->setValue(terms[current].fuzzyValueM);
    ui->termValueU->setValue(terms[current].fuzzyValueU);
    updateFuzzyValuePlot();
}

void MainWindow::onTermValueChanged(double value) {
    terms[currentTerm].value = value;
    fcm->terms[terms[currentTerm].id].value = value;
}

void MainWindow::onTermValueLChanged(double value) {
    if (ui->termValueM->value() < value) {
        ui->termValueM->setValue(value);
        fcm->terms[terms[currentTerm].id].fuzzyValueM = value;
    }
    if (ui->termValueU->value() < value) {
        ui->termValueU->setValue(value);
        fcm->terms[terms[currentTerm].id].fuzzyValueU = value;
    }
    terms[currentTerm].fuzzyValueL = value;
    fcm->terms[terms[currentTerm].id].fuzzyValueL = value;
    updateFuzzyValuePlot();
}

void MainWindow::onTermValueMChanged(double value) {
    if (ui->termValueL->value() > value) {
        ui->termValueL->setValue(value);
        fcm->terms[terms[currentTerm].id].fuzzyValueL = value;
    }
    if (ui->termValueU->value() < value) {
        ui->termValueU->setValue(value);
        fcm->terms[terms[currentTerm].id].fuzzyValueU = value;
    }
    terms[currentTerm].fuzzyValueM = value;
    fcm->terms[terms[currentTerm].id].fuzzyValueM = value;
    updateFuzzyValuePlot();
}

void MainWindow::onTermValueUChanged(double value) {
    if (ui->termValueL->value() > value) {
        ui->termValueL->setValue(value);
        fcm->terms[terms[currentTerm].id].fuzzyValueL = value;
    }
    if (ui->termValueM->value() > value) {
        ui->termValueM->setValue(value);
        fcm->terms[terms[currentTerm].id].fuzzyValueM = value;
    }
    terms[currentTerm].fuzzyValueU = value;
    fcm->terms[terms[currentTerm].id].fuzzyValueU = value;
    updateFuzzyValuePlot();
}

void MainWindow::updateFuzzyValuePlot() {
    ui->fuzzyValuePlot->graph(0)->setData(QVector<double>{ui->termValueL->value(), ui->termValueM->value(), ui->termValueU->value()}, QVector<double>{0, 1, 0});
    ui->fuzzyValuePlot->replot();
}

void MainWindow::onItemChanged(QListWidgetItem *item) {
    terms[item].name = item->text();
    fcm->terms[terms[item].id].name = item->text();
}

void MainWindow::onPredictToStaticChanged(bool checked) {
    ui->comboBoxMetric->setEnabled(checked);
    ui->doubleSpinBoxThreshold->setEnabled(checked);
    ui->spinBoxMetricSteps->setEnabled(checked);
    ui->spinBoxFixedSteps->setEnabled(!checked);
}

void MainWindow::updateFCM() {
    fcm->name = ui->modelName->text();
    fcm->description = ui->modelNotes->toPlainText();
    fcm->predictionParameters = getPredictionParameters();
}

PredictionParameters MainWindow::getPredictionParameters() {
    return PredictionParameters{
        ui->comboBoxAlgorithm->currentText(),
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
    modelsRepository->createModel(*fcm);
}

void MainWindow::open() {
    const auto modelsNames = modelsRepository->getModelsNames();

    LoadModelWindow* loadModelWindow = new LoadModelWindow(modelsNames, this);

    if (loadModelWindow->exec() != QDialog::Accepted) {
        return;
    }
    QString modelName = loadModelWindow->selectedModelName();
    if (modelName.isEmpty()) {
        return;
    }

    auto model = modelsRepository->getModel(modelName);
    if (!model) {
        return;
    }
    fcm = std::make_shared<FCM>(*model);
    ui->modelName->setText(fcm->name);
    ui->modelNotes->setPlainText(fcm->description);

    ui->listWidgetTerms->clear();
    terms.clear();
    for (auto& [id, term] : fcm->terms) {
        QListWidgetItem* item = new QListWidgetItem(term.name);
        ui->listWidgetTerms->addItem(item);
        terms[item] = term;
    }
    if (!fcm->terms.empty()) {
        ui->listWidgetTerms->setCurrentRow(0);
    }

    auto newScene = new GraphScene(fcm);
    QObject::connect(ui->pushButtonMode, &QPushButton::clicked, newScene, &GraphScene::switchMode);
    QObject::connect(newScene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    auto oldSceneCreate = ui->graphicsViewGraph->scene();
    auto oldScenePredict = ui->graphicsViewPredict->scene();
    ui->graphicsViewGraph->setScene(newScene);
    ui->graphicsViewPredict->setScene(newScene);
    if (oldSceneCreate != oldScenePredict) {
        delete oldScenePredict;
    }
    delete oldSceneCreate;

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

    if (fileName.isEmpty())
        return;

    auto model = JsonRepository::importFromJson(fileName);

    if (!fcm)
    {
        QMessageBox::warning(this, "Error", "Failed to load file.");
    }

    fcm = std::make_shared<FCM>(*model);
    ui->modelName->setText(fcm->name);
    ui->modelNotes->setPlainText(fcm->description);

    ui->listWidgetTerms->clear();
    terms.clear();
    for (auto& [id, term] : fcm->terms) {
        QListWidgetItem* item = new QListWidgetItem(term.name);
        ui->listWidgetTerms->addItem(item);
        terms[item] = term;
    }
    if (!fcm->terms.empty()) {
        ui->listWidgetTerms->setCurrentRow(0);
    }

    auto newScene = new GraphScene(fcm);
    QObject::connect(ui->pushButtonMode, &QPushButton::clicked, newScene, &GraphScene::switchMode);
    QObject::connect(newScene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    auto oldSceneCreate = ui->graphicsViewGraph->scene();
    auto oldScenePredict = ui->graphicsViewPredict->scene();
    ui->graphicsViewGraph->setScene(newScene);
    ui->graphicsViewPredict->setScene(newScene);
    if (oldSceneCreate != oldScenePredict) {
        delete oldScenePredict;
    }
    delete oldSceneCreate;

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
