#include "simulation_presenter.h"

#include "presenter/models/creation_presenter.h"
#include "presenter/simulation/simulation_scene_presenter.h"
#include "ui_main_window.h"

#include "ui/graph_editor/graph_scene.h"
#include "ui/save_current_state_window/save_current_state_window.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

SimulationPresenter::SimulationPresenter(Ui::MainWindow* ui, std::shared_ptr<FCM>& fcm, std::shared_ptr<CreationPresenter> creationPresenter, QWidget* parentWidget, QObject *parent)
    : ui(ui), parentWidget(parentWidget), creationPresenter(std::move(creationPresenter)), fcm(fcm), QObject{parent} {
    ui->comboBoxAlgorithm->setItemData(0, "const weights", Qt::UserRole);
    ui->comboBoxAlgorithm->setItemData(1, "changing weights", Qt::UserRole);
    ui->comboBoxActivation->setItemData(0, "bivalent", Qt::UserRole);
    ui->comboBoxActivation->setItemData(1, "trivalent", Qt::UserRole);
    ui->comboBoxActivation->setItemData(2, "threshold-linear", Qt::UserRole);
    ui->comboBoxActivation->setItemData(3, "sigmoid", Qt::UserRole);
    ui->comboBoxActivation->setItemData(4, "hyperbolic tangent", Qt::UserRole);
    ui->comboBoxMetric->setItemData(0, "MSE", Qt::UserRole);
    ui->comboBoxMetric->setItemData(1, "MAE", Qt::UserRole);
    ui->comboBoxMetric->setItemData(2, "MAPE", Qt::UserRole);

    QStandardItemModel* experimentsModel = new QStandardItemModel();
    experimentsModel->setHorizontalHeaderLabels({mainWindowTr("Algorithm"), mainWindowTr("Value type"), mainWindowTr("Activation function"), mainWindowTr("Metric"), mainWindowTr("Predict to static"), mainWindowTr("Threshold"), mainWindowTr("Steps less threshold"), mainWindowTr("Fixed steps"), mainWindowTr("Timestamp"), "", ""});
    ui->experimantsTable->setModel(experimentsModel);
    ui->experimantsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->experimantsTable->setSortingEnabled(true);
    ui->experimantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->graphicsViewPredict, &GraphView::scaleChanged, this, &SimulationPresenter::updatePredictScaleLabel);
    connect(ui->pushButtonPredict, &QPushButton::clicked, this, &SimulationPresenter::predict);
    connect(ui->pushButtonReset, &QPushButton::clicked, this, &SimulationPresenter::resetPredictionScene);
    connect(ui->pushButtonPause, &QPushButton::clicked, this, &SimulationPresenter::pauseResumePrediction);
    connect(ui->pushButtonSpeedUp, &QPushButton::clicked, this, &SimulationPresenter::speedUp);
    connect(ui->pushButtonSlowDown, &QPushButton::clicked, this, &SimulationPresenter::slowDown);
    connect(ui->pushButtonForward, &QPushButton::clicked, this, &SimulationPresenter::stepForward);
    connect(ui->pushButtonBack, &QPushButton::clicked, this, &SimulationPresenter::stepBack);
    connect(ui->pushButtonFinish, &QPushButton::clicked, this, &SimulationPresenter::finishSimulation);
    connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, this, &SimulationPresenter::onPredictToStaticChanged);
    connect(ui->comboBoxAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SimulationPresenter::autosave);
    connect(ui->useFuzzyValues, &QCheckBox::toggled, this, &SimulationPresenter::autosave);
    connect(ui->comboBoxActivation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SimulationPresenter::changeActivationFunction);
    connect(ui->comboBoxMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SimulationPresenter::autosave);
    connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, this, &SimulationPresenter::autosave);
    connect(ui->doubleSpinBoxThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SimulationPresenter::autosave);
    connect(ui->spinBoxMetricSteps, QOverload<int>::of(&QSpinBox::valueChanged), this, &SimulationPresenter::autosave);
    connect(ui->spinBoxFixedSteps, QOverload<int>::of(&QSpinBox::valueChanged), this, &SimulationPresenter::autosave);

    connect(ui->comboBoxAlgorithmSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxAlgorithm, &QComboBox::setCurrentIndex);
    connect(ui->useFuzzyValuesSensitivity, &QCheckBox::toggled, ui->useFuzzyValues, &QCheckBox::setChecked);
    connect(ui->comboBoxActivationSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxActivation, &QComboBox::setCurrentIndex);
    connect(ui->fuzzinessDegreeSensitivity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->fuzzinessDegree, &QDoubleSpinBox::setValue);
    connect(ui->checkBoxPredictToStaticSensitivity, &QCheckBox::toggled, ui->checkBoxPredictToStatic, &QCheckBox::setChecked);
    connect(ui->comboBoxMetricSensitivity, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxMetric, &QComboBox::setCurrentIndex);
    connect(ui->doubleSpinBoxThresholdSensitivity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->doubleSpinBoxThreshold, &QDoubleSpinBox::setValue);
    connect(ui->spinBoxMetricStepsSensitivity, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxMetricSteps, &QSpinBox::setValue);
    connect(ui->spinBoxFixedStepsSensitivity, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxFixedSteps, &QSpinBox::setValue);

    recreateScenePresenter();
}

void SimulationPresenter::recreateScenePresenter() {
    simulationScenePresenter = std::make_shared<SimulationScenePresenter>(creationPresenter, nullptr);
    connect(simulationScenePresenter.get(), &SimulationScenePresenter::updateProgress, this, &SimulationPresenter::updateProgress);
    connect(simulationScenePresenter.get(), &SimulationScenePresenter::finished, this, &SimulationPresenter::simulationFinished);
}

void SimulationPresenter::reconfigure() {
    if (isActive()) {
        resetPredictionScene();
    }

    recreateScenePresenter();

    currentMetricValue = 0;
    paused = false;
    ui->pushButtonPause->setText(mainWindowTr("Pause"));
    ui->progressBarPredict->setValue(0);
    ui->labelMetricValue->setText(QString(mainWindowTr("Metric value: %1")).arg(currentMetricValue, 0, 'f', 4));

    ui->experimantsTable->model()->removeRows(0, ui->experimantsTable->model()->rowCount());
    for (const auto& experiment : fcm->experiments) {
        addExperiment(experiment);
    }
}

void SimulationPresenter::changeActivationFunction(int index) {
    ui->fuzzinessDegree->setEnabled(index == 3 || index == 4);
    emit autosave();
}

bool SimulationPresenter::isActive() const {
    return simulationScenePresenter && simulationScenePresenter->isActive();
}

bool SimulationPresenter::moveStep(int delta) {
    return simulationScenePresenter && simulationScenePresenter->moveStep(delta);
}

void SimulationPresenter::refreshFromModelSetup() {
    if (isActive()) {
        moveStep(0);
    }
}

void SimulationPresenter::retranslateUi() {
    ui->labelScalePredict->setText(QString(mainWindowTr("Scale: %1%")).arg(predictScale*100, 0, 'f', 2));
    ui->pushButtonPause->setText(paused ? mainWindowTr("Resume") : mainWindowTr("Pause"));
    ui->labelMetricValue->setText(QString(mainWindowTr("Metric value: %1")).arg(currentMetricValue, 0, 'f', 4));

    if (auto* experimentsModel = qobject_cast<QStandardItemModel*>(ui->experimantsTable->model())) {
        experimentsModel->setHorizontalHeaderLabels({
            mainWindowTr("Algorithm"),
            mainWindowTr("Value type"),
            mainWindowTr("Activation function"),
            mainWindowTr("Metric"),
            mainWindowTr("Predict to static"),
            mainWindowTr("Threshold"),
            mainWindowTr("Steps less threshold"),
            mainWindowTr("Fixed steps"),
            mainWindowTr("Timestamp"),
            "",
            ""
        });

        for (int row = 0; row < experimentsModel->rowCount(); ++row) {
            if (auto* loadButton = qobject_cast<QPushButton*>(ui->experimantsTable->indexWidget(experimentsModel->index(row, 9)))) {
                loadButton->setText(mainWindowTr("Load"));
            }
            if (auto* deleteButton = qobject_cast<QPushButton*>(ui->experimantsTable->indexWidget(experimentsModel->index(row, 10)))) {
                deleteButton->setText(mainWindowTr("Delete"));
            }

            QModelIndex idx = experimentsModel->index(row, 0);
            experimentsModel->setData(idx, mainWindowTr(fcm->experiments[row].predictionParameters.algorithm.toUtf8().constData()));
            experimentsModel->setData(experimentsModel->index(row, 1), fcm->experiments[row].predictionParameters.useFuzzyValues ? mainWindowTr("fuzzy") : mainWindowTr("numeric"));

            idx = experimentsModel->index(row, 2);
            auto activationFunctionText = mainWindowTr(fcm->experiments[row].predictionParameters.activationFunction.toUtf8().constData());
            if (fcm->experiments[row].predictionParameters.activationFunction == "sigmoid" || fcm->experiments[row].predictionParameters.activationFunction == "hyperbolic tangent") {
                activationFunctionText += "\n" + mainWindowTr("fuzziness degree") + " = " + QString::number(fcm->experiments[row].predictionParameters.fuzzinessDegree);
            }
            experimentsModel->setData(idx, activationFunctionText);
            experimentsModel->setData(experimentsModel->index(row, 4), fcm->experiments[row].predictionParameters.predictToStatic ? mainWindowTr("yes") : mainWindowTr("no"));
        }

        ui->experimantsTable->setWordWrap(true);
        ui->experimantsTable->resizeRowsToContents();
    }
}

void SimulationPresenter::updatePredictScaleLabel(double newScale) {
    ui->labelScalePredict->setText(QString(mainWindowTr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    predictScale = newScale;
}

void SimulationPresenter::simulationFinished() {
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

void SimulationPresenter::predict() {
    QString errorMessage;
    if (!fcm->checkElementsHaveValues(&errorMessage)) {
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr(errorMessage.toUtf8().constData()));
        return;
    }

    simulationScenePresenter->activate();

    const auto predictionParameters = fcm->predictionParameters;

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

    auto* predictScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy(simulationScenePresenter, ElementWindowMode::PredictionResults);
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

    simulationScenePresenter->setRuntimeContext(fcm, predictScene->getFCM(), predictScene);
    simulationScenePresenter->simulate(predictionParameters, simulationParameters, nodes, edges);
}

Experiment SimulationPresenter::createExperiment() {
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
    experiment.predictionParameters = fcm->predictionParameters;
    experiment.timestamp = QDateTime::currentDateTime();
    fcm->experiments.push_back(experiment);
    addExperiment(experiment);
    emit autosave();
    return experiment;
}

void SimulationPresenter::addExperiment(const Experiment& experiment) {
    auto experimentsModel = ui->experimantsTable->model();
    int row = experimentsModel->rowCount();
    experimentsModel->insertRow(row);
    experimentsModel->setData(experimentsModel->index(row, 0), mainWindowTr(experiment.predictionParameters.algorithm.toUtf8().constData()));
    experimentsModel->setData(experimentsModel->index(row, 1), experiment.predictionParameters.useFuzzyValues ? mainWindowTr("fuzzy") : mainWindowTr("numeric"));
    auto activationFunctionText = mainWindowTr(fcm->experiments[row].predictionParameters.activationFunction.toUtf8().constData());
    if (fcm->experiments[row].predictionParameters.activationFunction == "sigmoid" || fcm->experiments[row].predictionParameters.activationFunction == "hyperbolic tangent") {
        activationFunctionText += "\n" + mainWindowTr("fuzziness degree") + " = " + QString::number(fcm->experiments[row].predictionParameters.fuzzinessDegree);
    }
    experimentsModel->setData(experimentsModel->index(row, 2), activationFunctionText);
    experimentsModel->setData(experimentsModel->index(row, 3), mainWindowTr(experiment.predictionParameters.metric.toUtf8().constData()));
    experimentsModel->setData(experimentsModel->index(row, 4), experiment.predictionParameters.predictToStatic ? mainWindowTr("yes") : mainWindowTr("no"));
    experimentsModel->setData(experimentsModel->index(row, 5), experiment.predictionParameters.threshold);
    experimentsModel->setData(experimentsModel->index(row, 6), experiment.predictionParameters.stepsLessThreshold);
    experimentsModel->setData(experimentsModel->index(row, 7), experiment.predictionParameters.fixedSteps);
    experimentsModel->setData(experimentsModel->index(row, 8), experiment.timestamp);
    experimentsModel->setData(experimentsModel->index(row, 9), "");
    experimentsModel->setData(experimentsModel->index(row, 10), "");
    for (int column = 0; column < experimentsModel->columnCount(); ++column) {
        experimentsModel->setData(experimentsModel->index(row, column), Qt::AlignCenter, Qt::TextAlignmentRole);
    }
    QPushButton* btn = new QPushButton(mainWindowTr("Load"), ui->experimantsTable);
    btn->setProperty("row", row);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 9), btn);
    QPushButton* deleteButton = new QPushButton(mainWindowTr("Delete"), ui->experimantsTable);
    deleteButton->setProperty("row", row);
    ui->experimantsTable->setIndexWidget(experimentsModel->index(row, 10), deleteButton);
    connect(deleteButton, &QPushButton::clicked, this, &SimulationPresenter::onDeleteExperiment);
    connect(btn, &QPushButton::clicked, this, &SimulationPresenter::loadExperiment);
    ui->experimantsTable->setWordWrap(true);
    ui->experimantsTable->resizeRowsToContents();
}

void SimulationPresenter::loadExperiment() {
    auto* button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }

    int row = button->property("row").toInt();
    if (row < 0 || row >= static_cast<int>(fcm->experiments.size())) {
        return;
    }

    const bool canSaveCurrentState = fcm->checkElementsHaveValues();

    if (canSaveCurrentState) {
        auto saveCurrentStateWindow = SaveCurrentStateWindow(parentWidget);
        if (saveCurrentStateWindow.exec() != QDialog::Accepted) {
            return;
        }
        if (saveCurrentStateWindow.saveCurrentState()) {
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
        fcm->concepts[id]->term = concept->term ? fcm->terms[concept->term->id] : nullptr;
        fcm->concepts[id]->dbId = -1;
        if (fcm->experiments.back().concepts.find(id) != fcm->experiments.back().concepts.end()) {
            fcm->concepts[id]->description = fcm->experiments.back().concepts[id]->description;
        }
    }
    for (const auto& [id, weight] : fcm->experiments[row].weights) {
        fcm->weights[id] = std::make_shared<Weight>(*weight);
        fcm->weights[id]->term = weight->term ? fcm->terms[weight->term->id] : nullptr;
        fcm->weights[id]->dbId = -1;
        if (fcm->experiments.back().weights.find(id) != fcm->experiments.back().weights.end()) {
            fcm->weights[id]->description = fcm->experiments.back().weights[id]->description;
        }
    }

    emit loadFCMRequested(fcm);
}

void SimulationPresenter::onDeleteExperiment() {
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
    emit autosave();
}

void SimulationPresenter::resetPredictionScene() {
    if (!isActive()) {
        return;
    }
    simulationScenePresenter->reset();
    paused = false;
    ui->pushButtonPause->setText(mainWindowTr("Pause"));
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
    auto* predictionScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewPredict->setScene(ui->graphicsViewGraph->scene());
    delete predictionScene;
}

void SimulationPresenter::pauseResumePrediction() {
    if (!paused) {
        ui->pushButtonPause->setText(mainWindowTr("Resume"));
        simulationScenePresenter->pause();
        paused = true;
    } else {
        ui->pushButtonPause->setText(mainWindowTr("Pause"));
        simulationScenePresenter->resume();
        paused = false;
    }
}

void SimulationPresenter::speedUp() {
    simulationScenePresenter->speedUp();
    ui->doubleSpinBoxStepsPerSecond->setValue(simulationScenePresenter->getStepsPerSecond());
}

void SimulationPresenter::slowDown() {
    simulationScenePresenter->slowDown();
    ui->doubleSpinBoxStepsPerSecond->setValue(simulationScenePresenter->getStepsPerSecond());
}

void SimulationPresenter::stepForward() {
    if (!moveStep(ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Value of step is not calculated or step out of range!"));
    }
}

void SimulationPresenter::stepBack() {
    if (!moveStep(-ui->spinBoxMoveSteps->value())) {
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Value of step is not calculated or step out of range!"));
    }
}

void SimulationPresenter::finishSimulation() {
    ui->pushButtonBack->setEnabled(false);
    ui->pushButtonForward->setEnabled(false);
    ui->pushButtonSlowDown->setEnabled(false);
    ui->pushButtonPause->setEnabled(false);
    ui->pushButtonSpeedUp->setEnabled(false);
    ui->pushButtonFinish->setEnabled(false);
    simulationScenePresenter->finish();
}

void SimulationPresenter::updateProgress(size_t value, size_t maxStep, double metricValue) {
    ui->progressBarPredict->setMaximum(maxStep);
    ui->progressBarPredict->setValue(value);
    ui->labelMetricValue->setText(QString(mainWindowTr("Metric value: %1")).arg(metricValue, 0, 'f', 4));
    currentMetricValue = metricValue;
}

void SimulationPresenter::onPredictToStaticChanged(bool checked) {
    ui->doubleSpinBoxThreshold->setEnabled(checked);
    ui->spinBoxMetricSteps->setEnabled(checked);
    ui->spinBoxFixedSteps->setEnabled(!checked);

    ui->doubleSpinBoxThresholdSensitivity->setEnabled(checked);
    ui->spinBoxMetricStepsSensitivity->setEnabled(checked);
    ui->spinBoxFixedStepsSensitivity->setEnabled(!checked);
}
