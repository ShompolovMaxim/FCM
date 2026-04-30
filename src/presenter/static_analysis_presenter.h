#pragma once

#include <memory>

#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

#include "model/static_analysis/static_analyzer.h"
#include "ui/graph_editor/graph_scene.h"
#include "creation_presenter.h"

class StaticAnalysisPresenter : public QObject {
    Q_OBJECT
public:
    StaticAnalysisPresenter(QWidget* tab, std::shared_ptr<CreationPresenter> presenter, std::shared_ptr<FCM> fcm);

    void refreshUI(bool changeTable = true);

private slots:
    void onConceptCreated(std::shared_ptr<Concept>);
    void onConceptUpdated(std::shared_ptr<Concept>);
    void onConceptDeleted(QUuid);

    void onWeightCreated(std::shared_ptr<Weight>);
    void onWeightUpdated(std::shared_ptr<Weight>);
    void onWeightDeleted(QUuid);

    void recalculateInfluence();

private:
    void updateGraphConceptList();

    QWidget* tab;
    std::shared_ptr<CreationPresenter> presenter;
    std::shared_ptr<FCM> fcm;

    QLabel* densityLabel;
    QLabel* complexityLabel;
    QLabel* hierarchyLabel;
    QTableWidget* table;
    QComboBox* graphConcept;
    QComboBox* influenceDirection;
    QSpinBox* influenceSteps;
    GraphScene* graphScene;

    StaticAnalyzer analyzer;

    bool influenceCalculated = false;
};
