#include "static_analysis_presenter.h"

StaticAnalysisPresenter::StaticAnalysisPresenter(
    QWidget* tab,
    std::shared_ptr<FCM> fcm)
    : tab(tab), fcm(fcm)
{
    densityLabel = tab->findChild<QLabel*>("densityLabel");
    complexityLabel = tab->findChild<QLabel*>("complexityLabel");
    hierarchyLabel = tab->findChild<QLabel*>("hierarchyLabel");

    table = tab->findChild<QTableWidget*>("factorsStatsTable");
}

void StaticAnalysisPresenter::update()
{
    if (!fcm) {
        return;
    }

    auto result = StaticAnalyzer::analyze(*fcm);

    densityLabel->setText("FCM density: " + QString::number(result.density));
    complexityLabel->setText("FCM complexity: " + QString::number(result.complexity));
    hierarchyLabel->setText("FCM hierarchy index: " + QString::number(result.hierarchyIndex));

    table->clear();

    table->setRowCount(result.factors.size());
    table->setColumnCount(4);

    table->setHorizontalHeaderLabels({"Concept name", "Out degree", "In degree", "Centrality"});

    int row = 0;

    for (const auto& f : result.factors) {
        table->setItem(row,0, new QTableWidgetItem(QString::number(f.conceptId)));
        table->setItem(row,1, new QTableWidgetItem(QString::number(f.outDegree)));
        table->setItem(row,2, new QTableWidgetItem(QString::number(f.inDegree)));
        table->setItem(row,3, new QTableWidgetItem(QString::number(f.centrality)));

        row++;
    }
}
