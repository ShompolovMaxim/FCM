#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <memory>

#include "model/entities/fcm.h"

#include "presenter/creation_presenter.h"
#include "presenter/simulation_presenter.h"
#include "presenter/static_analysis_presenter.h"

#include "repository/models_repository.h"

#include "ui/graph_editor/edit_mode.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateGraphScaleLabel(double newScale);
    void updatePredictScaleLabel(double newScale);
    void updateModeButtonText(EditMode newMode);
    void predict();
    void resetPredictionScene();
    void pauseResumePrediction();
    void speedUp();
    void slowDown();
    void stepForward();
    void stepBack();
    void updateProgress(size_t value, size_t maxStep, double metricValue);

    void onCreateTerm();
    void onDeleteTerm();
    void onChooseTermColor();
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onTermValueChanged(double value);
    void onTermValueLChanged(double value);
    void onTermValueMChanged(double value);
    void onTermValueUChanged(double value);
    void onItemChanged(QListWidgetItem *item);

    void onPredictToStaticChanged(bool checked);

    void saveAs();
    void open();
    void onExportPng();
    void onExportJson();
    void onImportJson();

private:
    void simulationFinished();

    void updateFCM();
    PredictionParameters getPredictionParameters();
    void addExperiment(const Experiment& experiment);

    void updateFuzzyValuePlot();

    void loadFCM(const FCM& newFCM);

    Ui::MainWindow *ui;
    std::shared_ptr<SimulationPresenter> presenter;
    StaticAnalysisPresenter* staticAnalysisPresenter;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<FCM> fcm;
    size_t currentTermId;

    std::shared_ptr<ModelsRepository> modelsRepository;

};
#endif // MAIN_WINDOW_H
