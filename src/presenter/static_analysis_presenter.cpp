#include "static_analysis_presenter.h"

#include "ui/graph_editor/color_value_adapter/color_value_adapter.h"
#include "ui/graph_editor/graph_view.h"

#include <algorithm>

StaticAnalysisPresenter::StaticAnalysisPresenter(QWidget* tab, std::shared_ptr<CreationPresenter> presenter, std::shared_ptr<FCM> fcm)
    : tab(tab), presenter(presenter),fcm(fcm), analyzer(fcm) {
    densityLabel = tab->findChild<QLabel*>("densityLabel");
    complexityLabel = tab->findChild<QLabel*>("complexityLabel");
    hierarchyLabel = tab->findChild<QLabel*>("hierarchyLabel");
    table = tab->findChild<QTableWidget*>("factorsStatsTable");
    graphConcept = tab->findChild<QComboBox*>("graphConcept");
    influenceDirection = tab->findChild<QComboBox*>("influenceDirection");
    influenceSteps = tab->findChild<QSpinBox*>("influenceSteps");
    graphScene = dynamic_cast<GraphScene*>(tab->findChild<GraphView*>("graphicsView")->scene());

    updateGraphConceptList();

    analyzer.init();

    connect(presenter.get(), &CreationPresenter::conceptCreated, this, &StaticAnalysisPresenter::onConceptCreated);
    connect(presenter.get(), &CreationPresenter::conceptUpdated, this, &StaticAnalysisPresenter::onConceptUpdated);
    connect(presenter.get(), &CreationPresenter::conceptDeleted, this, &StaticAnalysisPresenter::onConceptDeleted);
    connect(presenter.get(), &CreationPresenter::weightCreated, this, &StaticAnalysisPresenter::onWeightCreated);
    connect(presenter.get(), &CreationPresenter::weightUpdated, this, &StaticAnalysisPresenter::onWeightUpdated);
    connect(presenter.get(), &CreationPresenter::weightDeleted, this, &StaticAnalysisPresenter::onWeightDeleted);

    connect(graphConcept, &QComboBox::currentIndexChanged, this, &StaticAnalysisPresenter::recalculateInfluence);
    connect(influenceDirection, &QComboBox::currentIndexChanged, this, &StaticAnalysisPresenter::recalculateInfluence);
    connect(influenceSteps, &QSpinBox::valueChanged, this, &StaticAnalysisPresenter::recalculateInfluence);

    refreshUI();
}

void StaticAnalysisPresenter::onConceptCreated(std::shared_ptr<Concept> c) {
    updateGraphConceptList();
    analyzer.onConceptCreated(c);
    refreshUI();
}

void StaticAnalysisPresenter::onConceptUpdated(std::shared_ptr<Concept>) {
    updateGraphConceptList();
}

void StaticAnalysisPresenter::onConceptDeleted(QUuid id) {
    QVariant currentData = graphConcept->currentData();
    if (currentData.isValid() && currentData.toUuid() == id) {
        graphConcept->setCurrentIndex(0);
    }

    updateGraphConceptList();
    analyzer.onConceptDeleted(id);
    refreshUI();
}

void StaticAnalysisPresenter::onWeightCreated(std::shared_ptr<Weight> w) {
    analyzer.onWeightCreated(w);
    refreshUI();
}

void StaticAnalysisPresenter::onWeightUpdated(std::shared_ptr<Weight> w) {
    analyzer.onWeightUpdated(w);
    refreshUI();
}

void StaticAnalysisPresenter::onWeightDeleted(QUuid id) {
    analyzer.onWeightDeleted(id);
    refreshUI();
}

void StaticAnalysisPresenter::recalculateInfluence() {
    if (graphConcept->count() == 0) {
        return;
    }
    if (graphConcept->currentIndex()) {
        auto conceptId =graphConcept->currentData().toUuid();
        auto steps = influenceSteps->value();
        auto influenceFrom = influenceDirection->currentData(Qt::UserRole).toString() == "from";
        analyzer.updateInfluence(conceptId, steps, influenceFrom);
    }
    refreshUI();
}

void StaticAnalysisPresenter::refreshUI() {
    const auto& result = analyzer.getResult();

    densityLabel->setText(tr("FCM density: ") + QString::number(result.density));
    complexityLabel->setText(tr("FCM complexity: ") + QString::number(result.complexity));
    hierarchyLabel->setText(tr("FCM hierarchy index: ") + QString::number(result.hierarchyIndex));

    table->setRowCount(result.factors.size());
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({tr("Concept name"), tr("Out"), tr("In"), tr("Centrality")});

    int row = 0;
    for (const auto& [idc, f] : result.factors) {
        table->setItem(row,0, new QTableWidgetItem(f.conceptName));
        table->setItem(row,1, new QTableWidgetItem(QString::number(f.outDegree)));
        table->setItem(row,2, new QTableWidgetItem(QString::number(f.inDegree)));
        table->setItem(row,3, new QTableWidgetItem(QString::number(f.centrality)));
        row++;
    }

    table->setWordWrap(true);
    table->resizeRowsToContents();

    auto colorValueAdapter = ColorValueAdapter();
    for (auto [id, concept] : fcm->concepts) {
        if (graphConcept->count() > 1 && graphConcept->currentIndex()) {
            graphScene->setConceptColor(id, colorValueAdapter.getColor(std::min(std::max(result.factors.at(id).influence, -1.0), 1.0), -1, 1),
                                        id == graphConcept->currentData().toUuid());
        } else {
            graphScene->setConceptColor(id, colorValueAdapter.getColor(0, -1, 1), false);
        }
    }
}

void StaticAnalysisPresenter::updateGraphConceptList() {
    graphConcept->clear();

    QList<QPair<QString, QUuid>> conceptItems;
    for (const auto& [id, concept] : fcm->concepts) {
        conceptItems.append({concept->name, id});
    }

    std::sort(conceptItems.begin(), conceptItems.end(),
              [](const QPair<QString, QUuid>& a, const QPair<QString, QUuid>& b) {
                  return a.first < b.first;
              });

    graphConcept->addItem("", QVariant());
    for (const auto& item : conceptItems) {
        graphConcept->addItem(item.first, QVariant::fromValue(item.second));
    }

    QVariant currentData = graphConcept->currentData();
    if (currentData.isValid()) {
        for (int i = 0; i < graphConcept->count(); ++i) {
            if (graphConcept->itemData(i) == currentData) {
                graphConcept->setCurrentIndex(i);
                break;
            }
        }
    } else {
        graphConcept->setCurrentIndex(0);
    }
}
