#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <memory>

#include "model/entities/fcm.h"

#include "presenter/creation_presenter.h"
#include "presenter/model_setup_presenter.h"
#include "presenter/sensitivity_presenter.h"
#include "presenter/simulation_presenter.h"
#include "presenter/simulation_scene_presenter.h"
#include "presenter/static_analysis_presenter.h"

#include "repository/saving_manager.h"
#include "repository/templates_manager.h"

#include "ui/graph_editor/edit_mode.h"
#include "ui/help/help_window.h"
#include "ui/tooltips/controller.h"

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

signals:
    void modelDeletionFinished(const QString &modelName, bool success);

public slots:
    void updateGraphScaleLabel(double newScale);
    void updateModeButtonText(EditMode newMode);

    void onCurrentTabChanged(int index);

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

    void changeShowTooltips(bool checked);
    void showHelp();

    void nameChanged(QString newName);
    void createNewModel();
    void switchModel();
    void changeActivationFunctionSensitivity(int index);

    void joinModels();

    void setEnglish();
    void setRussian();

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void cancelPendingWeightCreation();

    bool modelHasUnsavedChanges(std::shared_ptr<FCM> model);
    bool closeModel(size_t index);
    void closeOtherModels(size_t index);
    void rebuildModelsMenu();
    void deleteSavedModel(const QString &modelName);
    void deleteSavedTemplate(const QString &templateName);

    void recreatePresenters();

    void updateFCM();
    void loadFCM(std::shared_ptr<FCM> newFCM);

    void addFCM(std::shared_ptr<FCM> fcm);

    Ui::MainWindow *ui;
    std::shared_ptr<SimulationScenePresenter> simulationScenePresenter;
    StaticAnalysisPresenter* staticAnalysisPresenter;
    std::shared_ptr<SensitivityPresenter> sensitivityPresenter;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter;
    std::shared_ptr<SimulationPresenter> simulationPresenter;
    std::shared_ptr<FCM> fcm;

    std::shared_ptr<TemplatesManager> templatesManager;
    std::shared_ptr<SavingManager> savingManager;

    QSettings settings = QSettings("HSE", "FCM");

    std::vector<std::shared_ptr<FCM>> fcms;
    size_t currentModelIdx;

    QTranslator translatorRus;
    QTranslator translatorDefaultRus;
    QTranslator translatorWidgetsRus;

    double graphScale = 1;
    EditMode editMode = EditMode::Create;

    ToolTipController toolTipController = ToolTipController();

    HelpWindow *helpWindow = nullptr;
};
#endif // MAIN_WINDOW_H
