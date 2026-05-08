#include "model_setup_presenter.h"

#include "common/logger.h"
#include "model/entities/fcm.h"
#include "presenter/creation_presenter.h"
#include "presenter/simulation_presenter.h"
#include "presenter/static_analysis_presenter.h"
#include "ui/graph_editor/color_value_adapter/color_value_adapter.h"
#include "ui_main_window.h"

#include <QColorDialog>
#include <QCoreApplication>
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
    conceptsGroup = ui->treeWidgetTerms->topLevelItemCount() > 0
        ? ui->treeWidgetTerms->topLevelItem(0)
        : new QTreeWidgetItem(ui->treeWidgetTerms);
    weightsGroup = ui->treeWidgetTerms->topLevelItemCount() > 1
        ? ui->treeWidgetTerms->topLevelItem(1)
        : new QTreeWidgetItem(ui->treeWidgetTerms);

    conceptsGroup->setText(0, mainWindowTr("Concepts terms"));
    weightsGroup->setText(0, mainWindowTr("Weights terms"));

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
        fcm->terms[id]->color = ColorValueAdapter().getColor(fcm->terms[id]->value, 0, 1);
    } else {
        fcm->terms[id]->color = ColorValueAdapter().getColor(fcm->terms[id]->value, -1, 1);
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
        fcm->terms[currentTermId]->color = ColorValueAdapter().getColor(meanTermValue, 0, 1);
    } else {
        fcm->terms[currentTermId]->color = ColorValueAdapter().getColor(meanTermValue, -1, 1);
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
    if (presenter->isActive()) {
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
            QMessageBox::critical(parentWidget, tr("Error"), tr("There already is a term of this type with such a name"));
            return;
        }
    }
    fcm->terms[id]->name = item->text(0);
    creationPresenter->updateTerm(id);
}
