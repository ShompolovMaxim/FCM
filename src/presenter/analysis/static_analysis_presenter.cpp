#include "static_analysis_presenter.h"

#include "model/color_value_adapter/linear_approximation_adapter.h"
#include "ui_main_window.h"
#include "ui/graph_editor/graph_scene.h"
#include "ui/graph_editor/graph_view.h"

#include <QSignalBlocker>
#include <algorithm>

StaticAnalysisPresenter::StaticAnalysisPresenter(Ui::MainWindow* ui, std::shared_ptr<CreationPresenter> presenter, std::shared_ptr<FCM> fcm)
    : ui(ui), presenter(presenter), fcm(fcm), graphScene(dynamic_cast<GraphScene*>(ui->graphicsView->scene())), analyzer(fcm), fuzzyAnalyzer(fcm) {
    updateGraphConceptList();

    analyzer.init();
    fuzzyAnalyzer.init();

    connect(presenter.get(), &CreationPresenter::conceptCreated, this, &StaticAnalysisPresenter::onConceptCreated, Qt::QueuedConnection);
    connect(presenter.get(), &CreationPresenter::conceptUpdated, this, &StaticAnalysisPresenter::onConceptUpdated, Qt::QueuedConnection);
    connect(presenter.get(), &CreationPresenter::conceptDeleted, this, &StaticAnalysisPresenter::onConceptDeleted, Qt::QueuedConnection);
    connect(presenter.get(), &CreationPresenter::weightCreated, this, &StaticAnalysisPresenter::onWeightCreated, Qt::QueuedConnection);
    connect(presenter.get(), &CreationPresenter::weightUpdated, this, &StaticAnalysisPresenter::onWeightUpdated, Qt::QueuedConnection);
    connect(presenter.get(), &CreationPresenter::weightDeleted, this, &StaticAnalysisPresenter::onWeightDeleted, Qt::QueuedConnection);

    connect(ui->graphConcept, &QComboBox::currentIndexChanged, this, &StaticAnalysisPresenter::recalculateInfluence);
    connect(ui->influenceDirection, &QComboBox::currentIndexChanged, this, &StaticAnalysisPresenter::recalculateInfluence);
    connect(ui->influenceSteps, &QSpinBox::valueChanged, this, &StaticAnalysisPresenter::recalculateInfluence);
    connect(ui->useFuzzyValuesStatic, &QCheckBox::checkStateChanged, this, &StaticAnalysisPresenter::useFuzzyValuesChanged);

    refreshUI();
}

void StaticAnalysisPresenter::reconfigure(std::shared_ptr<FCM> newFcm) {
    fcm = std::move(newFcm);
    graphScene = dynamic_cast<GraphScene*>(ui->graphicsView->scene());
    analyzer = StaticAnalyzer(fcm);
    fuzzyAnalyzer = FuzzyStaticAnalyzer(fcm);
    updateGraphConceptList();
    analyzer.init();
    fuzzyAnalyzer.init();

    refreshUI();
}

void StaticAnalysisPresenter::refreshFromModelSetup() {
    refreshUI(false);
}

void StaticAnalysisPresenter::useFuzzyValuesChanged() {
    recalculateInfluence();
    refreshUI(true);
}

void StaticAnalysisPresenter::onConceptCreated(std::shared_ptr<Concept> c) {
    updateGraphConceptList();
    analyzer.onConceptCreated(c);
    fuzzyAnalyzer.onConceptCreated(c);
    refreshUI();
}

void StaticAnalysisPresenter::onConceptUpdated(std::shared_ptr<Concept>) {
    updateGraphConceptList();
}

void StaticAnalysisPresenter::onConceptDeleted(QUuid id) {
    QVariant currentData = ui->graphConcept->currentData();
    if (currentData.isValid() && currentData.toUuid() == id) {
        ui->graphConcept->setCurrentIndex(0);
    }

    updateGraphConceptList();
    analyzer.onConceptDeleted(id);
    fuzzyAnalyzer.onConceptDeleted(id);
    refreshUI();
}

void StaticAnalysisPresenter::onWeightCreated(std::shared_ptr<Weight> w) {
    analyzer.onWeightCreated(w);
    fuzzyAnalyzer.onWeightCreated(w);
    refreshUI();
}

void StaticAnalysisPresenter::onWeightUpdated(std::shared_ptr<Weight> w) {
    analyzer.onWeightUpdated(w);
    fuzzyAnalyzer.onWeightUpdated(w);
    refreshUI();
}

void StaticAnalysisPresenter::onWeightDeleted(QUuid id) {
    analyzer.onWeightDeleted(id);
    fuzzyAnalyzer.onWeightDeleted(id);
    refreshUI();
}

void StaticAnalysisPresenter::recalculateInfluence() {
    if (ui->graphConcept->count() == 0) {
        return;
    }
    if (ui->graphConcept->currentIndex()) {
        auto conceptId = ui->graphConcept->currentData().toUuid();
        auto steps = ui->influenceSteps->value();
        auto influenceFrom = ui->influenceDirection->currentData(Qt::UserRole).toString() == "from";
        analyzer.updateInfluence(conceptId, steps, influenceFrom);
        fuzzyAnalyzer.updateInfluence(conceptId, steps, influenceFrom);
    }
    refreshUI(false);
}

void StaticAnalysisPresenter::refreshUI(bool changeTable) {
    const auto& result = ui->useFuzzyValuesStatic->isChecked() ? fuzzyAnalyzer.getNumericResult() : analyzer.getResult();

    ui->densityLabel->setText(tr("FCM density: ") + QString::number(result.density));
    ui->complexityLabel->setText(tr("FCM complexity: ") + QString::number(result.complexity));
    ui->hierarchyLabel->setText(tr("FCM hierarchy index: ") + QString::number(result.hierarchyIndex));

    if (changeTable) {
        ui->factorsStatsTable->setRowCount(result.factors.size());
        ui->factorsStatsTable->setColumnCount(4);
        ui->factorsStatsTable->setHorizontalHeaderLabels({tr("Concept name"), tr("Out"), tr("In"), tr("Centrality")});

        int row = 0;
        for (const auto& [idc, f] : result.factors) {
            ui->factorsStatsTable->setItem(row, 0, new QTableWidgetItem(f.conceptName));
            ui->factorsStatsTable->setItem(row, 1, new QTableWidgetItem(QString::number(f.outDegree)));
            ui->factorsStatsTable->setItem(row, 2, new QTableWidgetItem(QString::number(f.inDegree)));
            ui->factorsStatsTable->setItem(row, 3, new QTableWidgetItem(QString::number(f.centrality)));
            row++;
        }

        ui->factorsStatsTable->setWordWrap(true);
        ui->factorsStatsTable->resizeRowsToContents();
    }

    auto colorValueAdapter = LinearApproximationColorValueAdapter(fcm->terms);
    for (auto [id, concept] : fcm->concepts) {
        if (ui->graphConcept->count() > 1 && ui->graphConcept->currentIndex()) {
            graphScene->setConceptColor(id, colorValueAdapter.getColor(std::min(std::max(result.factors.at(id).influence, -1.0), 1.0), -1, 1, false, ui->useFuzzyValuesStatic->isChecked()),
                                        id == ui->graphConcept->currentData().toUuid());
        } else {
            graphScene->setConceptColor(id, colorValueAdapter.getColor(0, -1, 1, false, ui->useFuzzyValuesStatic->isChecked()), false);
        }
    }
}

void StaticAnalysisPresenter::updateGraphConceptList() {
    QSignalBlocker blocker(ui->graphConcept);
    ui->graphConcept->clear();

    QList<QPair<QString, QUuid>> conceptItems;
    for (const auto& [id, concept] : fcm->concepts) {
        conceptItems.append({concept->name, id});
    }

    std::sort(conceptItems.begin(), conceptItems.end(),
              [](const QPair<QString, QUuid>& a, const QPair<QString, QUuid>& b) {
                  return a.first < b.first;
              });

    ui->graphConcept->addItem("", QVariant());
    for (const auto& item : conceptItems) {
        ui->graphConcept->addItem(item.first, QVariant::fromValue(item.second));
    }

    QVariant currentData = ui->graphConcept->currentData();
    if (currentData.isValid()) {
        for (int i = 0; i < ui->graphConcept->count(); ++i) {
            if (ui->graphConcept->itemData(i) == currentData) {
                ui->graphConcept->setCurrentIndex(i);
                break;
            }
        }
    } else {
        ui->graphConcept->setCurrentIndex(0);
    }
}

