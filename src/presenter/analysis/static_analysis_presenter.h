#pragma once

#include <memory>

#include <QObject>

#include "model/static_analysis/static_analyzer.h"
#include "model/static_analysis/fuzzy_static_analyzer.h"
#include "presenter/models/creation_presenter.h"

class GraphScene;
namespace Ui {
class MainWindow;
}

class StaticAnalysisPresenter : public QObject {
    Q_OBJECT
public:
    StaticAnalysisPresenter(Ui::MainWindow* ui, std::shared_ptr<CreationPresenter> presenter, std::shared_ptr<FCM> fcm);
    void reconfigure(std::shared_ptr<FCM> newFcm);

    void refreshUI(bool changeTable = true);

public slots:
    void refreshFromModelSetup();

private slots:
    void onConceptCreated(std::shared_ptr<Concept>);
    void onConceptUpdated(std::shared_ptr<Concept>);
    void onConceptDeleted(QUuid);

    void onWeightCreated(std::shared_ptr<Weight>);
    void onWeightUpdated(std::shared_ptr<Weight>);
    void onWeightDeleted(QUuid);

    void recalculateInfluence();

    void useFuzzyValuesChanged();

private:
    void updateGraphConceptList();

    Ui::MainWindow* ui;
    std::shared_ptr<CreationPresenter> presenter;
    std::shared_ptr<FCM> fcm;
    GraphScene* graphScene;

    StaticAnalyzer analyzer;
    FuzzyStaticAnalyzer fuzzyAnalyzer;

    bool influenceCalculated = false;
};
