#pragma once

#include "presenter/settings/toollip_controller.h"

#include <QObject>
#include <QSettings>
#include <QTranslator>
#include <QWidget>

namespace Ui {
class MainWindow;
}

class SettingsPresenter : public QObject {
    Q_OBJECT
public:
    explicit SettingsPresenter(
        Ui::MainWindow* ui,
        QObject *parent = nullptr
    );
    void applyInitialLanguage();

    void changeModelSettingsVisibility(bool checked);
    void changeGraphVisibility(bool checked);
    void changeAdjacencyMatrixVisibility(bool checked);
    void changeStaticAnalysisVisibility(bool checked);
    void changeSimulationVisibility(bool checked);
    void changeExperimentsVisibility(bool checked);
    void changeSensitivityAnalysisVisibility(bool checked);

    void changeShowTooltips(bool checked);

    void setEnglish();
    void setRussian();

signals:

private:
    Ui::MainWindow* ui;

    QSettings settings{"app.ini", QSettings::IniFormat};

    QTranslator translatorRus;
    QTranslator translatorDefaultRus;
    QTranslator translatorWidgetsRus;

    ToolTipController toolTipController;
};

