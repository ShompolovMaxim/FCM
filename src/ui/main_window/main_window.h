#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <memory>

#include "model/entities/fcm.h"

#include "presenter/creation_presenter.h"
#include "presenter/sensitivity_presenter.h"
#include "presenter/simulation_presenter.h"
#include "presenter/static_analysis_presenter.h"

#include "repository/saving_manager.h"
#include "repository/templates_manager.h"

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

    SensitivityAnalysisParameters getSensitivityParameters();
    void analize();
    void showSensitivityPlot();
    void updateSensitivityProgress(double progress);

    void onCreateTerm();
    void onDeleteTerm();
    void onChooseTermColor();
    void onCurrentItemChanged(QTreeWidgetItem  *current, QTreeWidgetItem *previous);
    void onTermValueChanged(double value);
    void onTermValueLChanged(double value);
    void onTermValueMChanged(double value);
    void onTermValueUChanged(double value);
    void onItemChanged(QTreeWidgetItem  *item, int column);

    void onPredictToStaticChanged(bool checked);
    void onDeleteExperiment();

    void saveAs();
    void save();
    void open();
    void autosaveChange(bool flag);
    void autosave();
    void saveAsTemplate();
    void openTemplate();
    void onExportPng();
    void onExportJson();
    void onImportJson();

    void changeModelSettingsVisibility(bool checked);
    void changeGraphVisibility(bool checked);
    void changeAdjacencyMatrixVisibility(bool checked);
    void changeStaticAnalysisVisibility(bool checked);
    void changeSimulationVisibility(bool checked);
    void changeExperimentsVisibility(bool checked);
    void changeSensitivityAnalysisVisibility(bool checked);

    void nameChanged(QString newName);
    void createNewModel();
    void switchModel();

    void joinModels();

    void setEnglish();
    void setRussian();

protected:
    void changeEvent(QEvent *event) override;

private:
    void simulationFinished();

    void updateFCM();
    PredictionParameters getPredictionParameters();
    void addExperiment(const Experiment& experiment);

    void updateFuzzyValuePlot();

    void loadFCM(std::shared_ptr<FCM> newFCM);

    void addFCM(std::shared_ptr<FCM> fcm);

    Ui::MainWindow *ui;
    std::shared_ptr<SimulationPresenter> presenter;
    StaticAnalysisPresenter* staticAnalysisPresenter;
    std::shared_ptr<SensitivityPresenter> sensitivityPresenter;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<FCM> fcm;
    QUuid currentTermId;
    QTreeWidgetItem* conceptsGroup;
    QTreeWidgetItem* weightsGroup;

    std::shared_ptr<TemplatesManager> templatesManager;
    std::shared_ptr<SavingManager> savingManager;

    bool sensitivityPlotShown = false;

    QSettings settings = QSettings("HSE", "FCM");

    std::vector<std::shared_ptr<FCM>> fcms;
    std::vector<QAction*> actions;
    size_t currentModelIdx;

    QTranslator translatorRus;

    double graphScale = 1;
    double predictScale = 1;
    EditMode editMode = EditMode::Create;
    bool paused = false;
    double currentMetricValue = 0;
};
#endif // MAIN_WINDOW_H
