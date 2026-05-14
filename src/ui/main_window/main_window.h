#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <memory>

#include "model/entities/fcm.h"
#include "presenter/models/creation_presenter.h"
#include "presenter/models/model_setup_presenter.h"
#include "presenter/models/models_join_presenter.h"
#include "presenter/models/models_switching_presenter.h"
#include "presenter/settings/settings_presenter.h"
#include "presenter/saving/saving_export_presenter.h"
#include "presenter/analysis/sensitivity_presenter.h"
#include "presenter/simulation/simulation_presenter.h"
#include "presenter/analysis/static_analysis_presenter.h"

#include "ui/help/help_window.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindowUiTest;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showHelp();

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
    std::shared_ptr<SettingsPresenter> settingsPresenter;
    std::shared_ptr<SavingExportPresenter> savingExportPresenter;
    std::shared_ptr<SensitivityPresenter> sensitivityPresenter;
    std::shared_ptr<CreationPresenter> creationPresenter;
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter;
    std::shared_ptr<SimulationPresenter> simulationPresenter;

    HelpWindow *helpWindow = nullptr;
};
#endif // MAIN_WINDOW_H
