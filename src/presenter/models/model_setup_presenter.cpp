#include "model_setup_presenter.h"

#include "common/logger.h"
#include "model/entities/fcm.h"
#include "model/prediction/prediction_parameters.h"
#include "presenter/models/creation_presenter.h"
#include "presenter/simulation/simulation_presenter.h"
#include "presenter/analysis/static_analysis_presenter.h"
#include "model/color_value_adapter/default_adapter.h"
#include "ui/graph_editor/graph_scene.h"
#include "ui_main_window.h"

#include <QColorDialog>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSignalBlocker>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

ModelSetupPresenter::ModelSetupPresenter(Ui::MainWindow* ui, std::shared_ptr<FCM>& fcm, std::shared_ptr<CreationPresenter>& creationPresenter, StaticAnalysisPresenter*& staticAnalysisPresenter, std::shared_ptr<SimulationPresenter>& presenter, QWidget* parentWidget, QObject* parent)
    : ui(ui),
      parentWidget(parentWidget),
      fcm(fcm),
      creationPresenter(creationPresenter),
      staticAnalysisPresenter(staticAnalysisPresenter),
      presenter(presenter),
      QObject(parent) {
    conceptsGroup = ui->treeWidgetTerms->topLevelItemCount() > 0 ? ui->treeWidgetTerms->topLevelItem(0) : new QTreeWidgetItem(ui->treeWidgetTerms);
    weightsGroup = ui->treeWidgetTerms->topLevelItemCount() > 1 ? ui->treeWidgetTerms->topLevelItem(1) : new QTreeWidgetItem(ui->treeWidgetTerms);

    conceptsGroup->setText(0, mainWindowTr("Concepts terms"));
    weightsGroup->setText(0, mainWindowTr("Weights terms"));

    ui->fuzzyValuePlot->xAxis->setRange(-1.1, 1.1);
    ui->fuzzyValuePlot->yAxis->setRange(0, 1);
    ui->fuzzyValuePlot->xAxis->setLabel("x");
    ui->fuzzyValuePlot->yAxis->setLabel("μ(x)");
    ui->fuzzyValuePlot->addGraph();

    connect(ui->modelNotes, &QTextEdit::textChanged, this, &ModelSetupPresenter::descriptionChanged);
    connect(ui->textEditNotesPredict, &QTextEdit::textChanged, this, &ModelSetupPresenter::descriptionChanged);
    connect(ui->textEditNotesSensitivity, &QTextEdit::textChanged, this, &ModelSetupPresenter::descriptionChanged);

    connect(ui->createTermButton, &QPushButton::clicked, this, &ModelSetupPresenter::onCreateTerm);
    connect(ui->deleteTermButton, &QPushButton::clicked, this, &ModelSetupPresenter::onDeleteTerm);
    connect(ui->termColorButton, &QPushButton::clicked, this, &ModelSetupPresenter::onChooseTermColor);
    connect(ui->termValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ModelSetupPresenter::onTermValueChanged);
    connect(ui->termValueL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ModelSetupPresenter::onTermValueLChanged);
    connect(ui->termValueM, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ModelSetupPresenter::onTermValueMChanged);
    connect(ui->termValueU, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ModelSetupPresenter::onTermValueUChanged);
    connect(ui->termNotes, &QTextEdit::textChanged, this, &ModelSetupPresenter::termNotesChanged);
    connect(ui->treeWidgetTerms, &QTreeWidget::currentItemChanged, this, &ModelSetupPresenter::onCurrentItemChanged);
    connect(ui->treeWidgetTerms, &QTreeWidget::itemChanged, this, &ModelSetupPresenter::onItemChanged);
}

bool ModelSetupPresenter::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape &&
        ui->tabWidget->currentWidget() == ui->graph &&
        creationPresenter &&
        creationPresenter->hasPendingWeightStart()) {
        cancelPendingWeightCreation();
        event->accept();
        return true;
    }

    return false;
}

void ModelSetupPresenter::reconfigure() {
    QSignalBlocker b1(ui->modelName);
    QSignalBlocker b2(ui->termValue);
    QSignalBlocker b3(ui->termValueL);
    QSignalBlocker b4(ui->termValueM);
    QSignalBlocker b5(ui->termValueU);
    QSignalBlocker b6(ui->termNotes);

    qDeleteAll(conceptsGroup->takeChildren());
    qDeleteAll(weightsGroup->takeChildren());

    ui->modelName->setText(fcm->name);
    ui->modelNotes->setMarkdownText(fcm->description);

    std::vector<std::pair<QUuid, std::shared_ptr<Term>>> sortedTerms(
        fcm->terms.begin(), fcm->terms.end()
    );

    std::sort(sortedTerms.begin(), sortedTerms.end(),
        [](const auto& a, const auto& b) {
            return a.second->value < b.second->value;
        });

    for (auto& [id, term] : sortedTerms) {
        auto* item = new QTreeWidgetItem();
        item->setText(0, term->name);
        item->setData(0, Qt::UserRole, QVariant::fromValue(id));
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if (term->type == ElementType::Node) {
            conceptsGroup->addChild(item);
        } else {
            weightsGroup->addChild(item);
        }
    }

    ui->treeWidgetTerms->expandAll();

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
}

void ModelSetupPresenter::retranslateUi() {
    QSignalBlocker blocker(ui->treeWidgetTerms);
    auto* currentConceptsGroup = ui->treeWidgetTerms->topLevelItem(0);
    auto* currentWeightsGroup = ui->treeWidgetTerms->topLevelItem(1);
    currentConceptsGroup->setText(0, mainWindowTr("Concepts terms"));
    currentWeightsGroup->setText(0, mainWindowTr("Weights terms"));
    updateGraphScaleLabel(graphScale);
    updateModeButtonText(editMode);
}

void ModelSetupPresenter::updateGraphScaleLabel(double newScale) {
    ui->labelScaleGraph->setText(QString(mainWindowTr("Scale: %1%")).arg(newScale * 100, 0, 'f', 2));
    graphScale = newScale;
}

void ModelSetupPresenter::updateModeButtonText(EditMode newMode) {
    ui->pushButtonMode->setText(newMode == EditMode::EditValues ? mainWindowTr("Mode: Edit values") : mainWindowTr("Mode: Create"));
    editMode = newMode;
}

PredictionParameters ModelSetupPresenter::getPredictionParameters() const {
    return PredictionParameters{
        ui->comboBoxAlgorithm->currentData(Qt::UserRole).toString(),
        ui->useFuzzyValues->isChecked(),
        ui->comboBoxActivation->currentData(Qt::UserRole).toString(),
        ui->comboBoxMetric->currentData(Qt::UserRole).toString(),
        ui->checkBoxPredictToStatic->isChecked(),
        ui->doubleSpinBoxThreshold->value(),
        ui->spinBoxMetricSteps->value(),
        ui->spinBoxFixedSteps->value(),
        ui->fuzzinessDegree->value()
    };
}

bool ModelSetupPresenter::checkElementsHaveValues() {
    for (const auto& [_, concept] : fcm->concepts) {
        if (!concept->term) {
            QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Not every concept has a value!"));
            return false;
        }
    }
    for (const auto& [_, weight] : fcm->weights) {
        if (!weight->term) {
            QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("Not every weight has a value!"));
            return false;
        }
    }
    return true;
}

void ModelSetupPresenter::updateFCM() {
    fcm->name = ui->modelName->text();
    fcm->description = ui->modelNotes->markdownText();
    fcm->predictionParameters = getPredictionParameters();
    fcm->autoConfigureTermsColors = ui->autoColorConfiguration->isChecked();
    fcm->autoConfigureNumericValues = ui->autoNumericConfiguration->isChecked();
    fcm->autoConfigureFuzzyValues = ui->autoFuzzyConfiguration->isChecked();
}

void ModelSetupPresenter::descriptionChanged() {
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

void ModelSetupPresenter::cancelPendingWeightCreation() {
    if (!creationPresenter || !creationPresenter->hasPendingWeightStart()) {
        return;
    }

    if (auto* scene = qobject_cast<GraphScene*>(ui->graphicsViewGraph->scene())) {
        scene->cancelPendingWeightCreation();
    }
}

void ModelSetupPresenter::onCreateTerm() {
    QTreeWidgetItem* currentItem = ui->treeWidgetTerms->currentItem();

    QTreeWidgetItem* targetGroup = conceptsGroup;
    ElementType type = ElementType::Node;

    if (currentItem) {
        if (currentItem == weightsGroup || (currentItem->parent() == weightsGroup)) {
            targetGroup = weightsGroup;
            type = ElementType::Edge;
        }
    }

    size_t counter = 1;
    QStringList termsNames;
    for (const auto& [_, term] : fcm->terms) {
        if (term->type == type) {
            termsNames.append(term->name);
        }
    }
    while (termsNames.contains(mainWindowTr("New term") + (counter - 1 ? " (" + QString::number(counter) + ")" : ""))) {
        ++counter;
    }

    auto id = QUuid::createUuid();

    fcm->terms[id] = std::make_shared<Term>();
    fcm->terms[id]->id = id;
    fcm->terms[id]->name = mainWindowTr("New term") + (counter - 1 ? " (" + QString::number(counter) + ")" : "");
    fcm->terms[id]->type = type;

    if (type == ElementType::Node) {
        fcm->terms[id]->color = DefaultColorValueAdapter().getColor(fcm->terms[id]->value, 0, 1);
    } else {
        fcm->terms[id]->color = DefaultColorValueAdapter().getColor(fcm->terms[id]->value, -1, 1);
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, fcm->terms[id]->name);
    item->setData(0, Qt::UserRole, QVariant::fromValue(id));
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    targetGroup->addChild(item);

    ui->treeWidgetTerms->setCurrentItem(item);
    ui->treeWidgetTerms->editItem(item, 0);

    creationPresenter->updateTerm(id);
    popagateTermUpdate();
}

void ModelSetupPresenter::onDeleteTerm() {
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

void ModelSetupPresenter::onChooseTermColor() {
    QColor color = QColorDialog::getColor(Qt::white, parentWidget, QString(mainWindowTr("Choose term %1 color")).arg(fcm->terms[currentTermId]->name));
    if (color.isValid()) {
        fcm->terms[currentTermId]->color = color;
        ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
        creationPresenter->updateTerm(currentTermId);
        popagateTermUpdate();
    }
}

void ModelSetupPresenter::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    ui->createTermButton->setEnabled(current);

    QSignalBlocker b1(ui->termValue);
    QSignalBlocker b2(ui->termValueL);
    QSignalBlocker b3(ui->termValueM);
    QSignalBlocker b4(ui->termValueU);
    QSignalBlocker b5(ui->termNotes);

    if (!current || !current->parent()) {
        currentTermId = QUuid();
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
    ui->termColorButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
        }

        QToolTip {
            background-color: #ffffe1;
            color: black;
            border: 1px solid black;
        }
    )").arg(fcm->terms[currentTermId]->color.name()));
    updateFuzzyValuePlot();
}

void ModelSetupPresenter::onCurrentTabChanged(int index) {
    if (ui->tabWidget->currentWidget() != ui->graph &&
        creationPresenter &&
        creationPresenter->hasPendingWeightStart()) {
        cancelPendingWeightCreation();
    }
}

void ModelSetupPresenter::termNotesChanged() {
    if (ui->treeWidgetTerms->currentItem() && ui->treeWidgetTerms->currentItem()->parent()) {
        fcm->terms[currentTermId]->description = ui->termNotes->markdownText();
    }
}

void ModelSetupPresenter::autoConfigureTermColor() {
    if (!ui->autoColorConfiguration->isChecked()) {
        return;
    }
    double meanTermValue = (fcm->terms[currentTermId]->value + fcm->terms[currentTermId]->fuzzyValue.defuzzify()) / 2;
    if (fcm->terms[currentTermId]->type == ElementType::Node) {
        fcm->terms[currentTermId]->color = DefaultColorValueAdapter().getColor(meanTermValue, 0, 1);
    } else {
        fcm->terms[currentTermId]->color = DefaultColorValueAdapter().getColor(meanTermValue, -1, 1);
    }
    ui->termColorButton->setStyleSheet(QString("background-color: %1").arg(fcm->terms[currentTermId]->color.name()));
}

void ModelSetupPresenter::autoConfigureNumericValue() {
    if (!ui->autoNumericConfiguration->isChecked()) {
        return;
    }
    QSignalBlocker b1(ui->termValue);
    fcm->terms[currentTermId]->value = fcm->terms[currentTermId]->fuzzyValue.defuzzify();
    ui->termValue->setValue(fcm->terms[currentTermId]->fuzzyValue.defuzzify());
}

void ModelSetupPresenter::autoConfigureFuzzyValue() {
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

void ModelSetupPresenter::popagateTermUpdate() {
    staticAnalysisPresenter->refreshUI(false);
    if (presenter && presenter->isActive()) {
        presenter->moveStep(0);
    }
}

void ModelSetupPresenter::onTermValueChanged(double value) {
    fcm->terms[currentTermId]->value = value;
    autoConfigureFuzzyValue();
    autoConfigureTermColor();
    updateFuzzyValuePlot();
    creationPresenter->updateTerm(currentTermId);
    popagateTermUpdate();
}

void ModelSetupPresenter::onTermValueLChanged(double value) {
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

void ModelSetupPresenter::onTermValueMChanged(double value) {
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

void ModelSetupPresenter::onTermValueUChanged(double value) {
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

void ModelSetupPresenter::updateFuzzyValuePlot() {
    ui->fuzzyValuePlot->graph(0)->setData(QVector<double>{ui->termValueL->value(), ui->termValueM->value(), ui->termValueU->value()}, QVector<double>{0, 1, 0});
    ui->fuzzyValuePlot->replot();
}

void ModelSetupPresenter::onItemChanged(QTreeWidgetItem  *item, int column) {
    auto id = item->data(0, Qt::UserRole).toUuid();
    for (const auto [termId, term] : fcm->terms) {
        if (termId != id && term->type == fcm->terms[id]->type && item->text(0) == term->name) {
            item->setText(0, fcm->terms[id]->name);
            QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr("There already is a term of this type with such a name"));
            return;
        }
    }
    fcm->terms[id]->name = item->text(0);
    creationPresenter->updateTerm(id);
}

