#pragma once

#include <memory>
#include <QWidget>
#include <QLabel>
#include <QTableWidget>

#include "model/static_analysis/static_analyzer.h"

class StaticAnalysisPresenter
{
public:
    StaticAnalysisPresenter(
        QWidget* tab,
        std::shared_ptr<FCM> fcm);

    void update();

private:

    QWidget* tab;
    std::shared_ptr<FCM> fcm;

    QLabel* densityLabel;
    QLabel* complexityLabel;
    QLabel* hierarchyLabel;

    QTableWidget* table;
};
