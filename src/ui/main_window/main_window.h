#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTranslator>
#include <memory>

#include "model/entities/fcm.h"
#include "presenter/models/creation_presenter.h"
#include "presenter/models/model_setup_presenter.h"
#include "presenter/models/models_join_presenter.h"
#include "presenter/models/models_switching_presenter.h"
#include "presenter/saving/saving_export_presenter.h"
#include "presenter/analysis/sensitivity_presenter.h"
#include "presenter/simulation/simulation_presenter.h"
#include "presenter/analysis/static_analysis_presenter.h"

#include "ui/help/help_window.h"
#include "ui/tooltips/controller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindowUiTest;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void changeModelSettingsVisibility(bool checked);
    void changeGraphVisibility(bool checked);
    void changeAdjacencyMatrixVisibility(bool checked);
    void changeStaticAnalysisVisibility(bool checked);
    void changeSimulationVisibility(bool checked);
    void changeExperimentsVisibility(bool checked);
    void changeSensitivityAnalysisVisibility(bool checked);

    void changeShowTooltips(bool checked);
    void showHelp();

    void setEnglish();
    void setRussian();

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    friend class MainWindowUiTest;

    Ui::MainWindow *ui;
    StaticAnalysisPresenter* staticAnalysisPresenter;
    std::shared_ptr<ModelsJoinPresenter> modelsJoinPresenter;
    std::shared_ptr<ModelsSwitchingPresenter> modelsSwitchingPresenter;
    std::shared_ptr<SavingExportPresenter> savingExportPresenter;
    std::shared_ptr<SensitivityPresenter> sensitivityPresenter;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter;
    std::shared_ptr<SimulationPresenter> simulationPresenter;

    QSettings settings{"app.ini", QSettings::IniFormat};

    QTranslator translatorRus;
    QTranslator translatorDefaultRus;
    QTranslator translatorWidgetsRus;

    ToolTipController toolTipController = ToolTipController();

    HelpWindow *helpWindow = nullptr;
};
#endif // MAIN_WINDOW_H
