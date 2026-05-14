#include "settings_presenter.h"

#include "ui_main_window.h"

#include <QCoreApplication>
#include <QSignalBlocker>

SettingsPresenter::SettingsPresenter(
    Ui::MainWindow* ui,
    QObject *parent
) : ui(ui),
    QObject{parent} {
    const auto lang = settings.value("language", "").toString();
    translatorRus.load("FCM_ru_RU.qm");
    translatorDefaultRus.load("qtbase_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translatorWidgetsRus.load("qt_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (lang.isEmpty()) {
        ui->actionEnglish->setChecked(true);
    } else {
        ui->actionRussian->setChecked(true);
        qApp->installTranslator(&translatorRus);
        qApp->installTranslator(&translatorDefaultRus);
        qApp->installTranslator(&translatorWidgetsRus);
    }

    connect(ui->actionRussian, &QAction::triggered, this, &SettingsPresenter::setRussian);
    connect(ui->actionEnglish, &QAction::triggered, this, &SettingsPresenter::setEnglish);

    connect(ui->actionModelSettings, &QAction::toggled, this, &SettingsPresenter::changeModelSettingsVisibility);
    connect(ui->actionGraph, &QAction::toggled, this, &SettingsPresenter::changeGraphVisibility);
    connect(ui->actionAdjacencyMatrix, &QAction::toggled, this, &SettingsPresenter::changeAdjacencyMatrixVisibility);
    connect(ui->actionStaticAnalysis, &QAction::toggled, this, &SettingsPresenter::changeStaticAnalysisVisibility);
    connect(ui->actionSimulation, &QAction::toggled, this, &SettingsPresenter::changeSimulationVisibility);
    connect(ui->actionExperiments, &QAction::toggled, this, &SettingsPresenter::changeExperimentsVisibility);
    connect(ui->actionSensitivityAnalysis, &QAction::toggled, this, &SettingsPresenter::changeSensitivityAnalysisVisibility);

    ui->tabWidget->setTabVisible(0, settings.value("tabs/modelSettings", true).toBool());
    ui->tabWidget->setTabVisible(1, settings.value("tabs/graph", true).toBool());
    ui->tabWidget->setTabVisible(2, settings.value("tabs/adjMatrix", true).toBool());
    ui->tabWidget->setTabVisible(3, settings.value("tabs/staticAnalysis", true).toBool());
    ui->tabWidget->setTabVisible(4, settings.value("tabs/simulation", true).toBool());
    ui->tabWidget->setTabVisible(5, settings.value("tabs/experiments", true).toBool());
    ui->tabWidget->setTabVisible(6, settings.value("tabs/sensitivity", true).toBool());
    ui->actionModelSettings->setChecked(settings.value("tabs/modelSettings", true).toBool());
    ui->actionGraph->setChecked(settings.value("tabs/graph", true).toBool());
    ui->actionAdjacencyMatrix->setChecked(settings.value("tabs/adjMatrix", true).toBool());
    ui->actionStaticAnalysis->setChecked(settings.value("tabs/staticAnalysis", true).toBool());
    ui->actionSimulation->setChecked(settings.value("tabs/simulation", true).toBool());
    ui->actionExperiments->setChecked(settings.value("tabs/experiments", true).toBool());
    ui->actionSensitivityAnalysis->setChecked(settings.value("tabs/sensitivity", true).toBool());

    qApp->installEventFilter(&toolTipController);
    toolTipController.setEnabled(settings.value("tooltips", true).toBool());
    connect(ui->actionShowTooltips, &QAction::toggled, this, &SettingsPresenter::changeShowTooltips);
    ui->actionShowTooltips->setChecked(settings.value("tooltips", true).toBool());
}

void SettingsPresenter::changeModelSettingsVisibility(bool checked) {
    ui->tabWidget->setTabVisible(0, checked);
    settings.setValue("tabs/modelSettings", checked);
}

void SettingsPresenter::changeGraphVisibility(bool checked) {
    ui->tabWidget->setTabVisible(1, checked);
    settings.setValue("tabs/graph", checked);
}

void SettingsPresenter::changeAdjacencyMatrixVisibility(bool checked) {
    ui->tabWidget->setTabVisible(2, checked);
    settings.setValue("tabs/adjMatrix", checked);
}

void SettingsPresenter::changeStaticAnalysisVisibility(bool checked) {
    ui->tabWidget->setTabVisible(3, checked);
    settings.setValue("tabs/staticAnalysis", checked);
}

void SettingsPresenter::changeSimulationVisibility(bool checked) {
    ui->tabWidget->setTabVisible(4, checked);
    settings.setValue("tabs/simulation", checked);
}

void SettingsPresenter::changeExperimentsVisibility(bool checked) {
    ui->tabWidget->setTabVisible(5, checked);
    settings.setValue("tabs/experiments", checked);
}

void SettingsPresenter::changeSensitivityAnalysisVisibility(bool checked) {
    ui->tabWidget->setTabVisible(6, checked);
    settings.setValue("tabs/sensitivity", checked);
}

void SettingsPresenter::changeShowTooltips(bool checked) {
    toolTipController.setEnabled(checked);
    settings.setValue("tooltips", checked);
}

void SettingsPresenter::setEnglish() {
    QSignalBlocker b1(ui->actionRussian);
    QSignalBlocker b2(ui->actionEnglish);
    ui->actionEnglish->setChecked(true);
    ui->actionRussian->setChecked(false);
    qApp->removeTranslator(&translatorRus);
    qApp->removeTranslator(&translatorDefaultRus);
    qApp->removeTranslator(&translatorWidgetsRus);
    settings.setValue("language", "");
}

void SettingsPresenter::setRussian() {
    QSignalBlocker b1(ui->actionEnglish);
    QSignalBlocker b2(ui->actionRussian);
    ui->actionRussian->setChecked(true);
    ui->actionEnglish->setChecked(false);
    qApp->installTranslator(&translatorRus);
    qApp->installTranslator(&translatorDefaultRus);
    qApp->installTranslator(&translatorWidgetsRus);
    settings.setValue("language", "RU");
}
