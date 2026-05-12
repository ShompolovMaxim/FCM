#include "sensitivity_presenter.h"

#include "model_setup_presenter.h"
#include "ui_main_window.h"

#include "ui/graph_editor/graph_scene.h"

#include <QCoreApplication>
#include <QMessageBox>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

SensitivityPresenter::SensitivityPresenter(
    Ui::MainWindow* ui,
    std::shared_ptr<FCM>& fcm,
    std::shared_ptr<ModelSetupPresenter> modelSetupPresenter,
    std::shared_ptr<SimulationPresenter> simulationPresenter,
    std::shared_ptr<CreationPresenter> creationPresenter,
    QWidget* parentWidget,
    QObject* parent
) : ui(ui), parentWidget(parentWidget), modelSetupPresenter(modelSetupPresenter), simulationPresenter(simulationPresenter), creationPresenter(creationPresenter), fcm(fcm), QObject{parent} {
    connect(ui->graphicsViewSensitivity, &GraphView::scaleChanged, this, &SensitivityPresenter::updateSensitivityScaleLabel);
    connect(ui->pushButtonAnalizeSensitivity, &QPushButton::clicked, this, &SensitivityPresenter::analize);
    connect(ui->pushButtonResetSensitivity, &QPushButton::clicked, this, &SensitivityPresenter::resetSensitivity);
    connect(ui->showSensitivityPlot, &QPushButton::clicked, this, &SensitivityPresenter::showSensitivityPlot);
}

SensitivityAnalysisParameters SensitivityPresenter::getSensitivityParameters() {
    return {
        ui->doubleSpinBoxMaxChange->value(),
        ui->changeConcepts->isChecked(),
        ui->changeWeights->isChecked(),
        10,
        1000,
        ui->sensitivityMeasureMetric->currentData(Qt::UserRole).toString()
    };
}

bool SensitivityPresenter::isActive() const {
    return sensitivityScenePresenter && sensitivityScenePresenter->isActive();
}

void SensitivityPresenter::reconfigure() {
    if (isActive()) {
        resetSensitivity();
    }

    sensitivityScenePresenter.reset();

    ui->progressBarSensitivity->setValue(0);
    ui->stackedWidgetSensitivity->setCurrentIndex(0);
    ui->doubleSpinBoxMaxChange->setValue(0.1);
    ui->changeConcepts->setChecked(true);
    ui->changeWeights->setChecked(false);
    ui->plotSensitivity->graph(0)->data()->clear();
    ui->plotSensitivity->replot();
    retranslateUi();
}

void SensitivityPresenter::retranslateUi() {
    ui->labelScaleSensitivity->setText(QString(mainWindowTr("Scale: %1%")).arg(sensitivityScale*100, 0, 'f', 2));
    ui->plotSensitivity->xAxis->setLabel(mainWindowTr("max change"));
    ui->plotSensitivity->yAxis->setLabel(mainWindowTr("sensitivity"));
    ui->plotSensitivity->replot();
    if (ui->stackedWidgetSensitivity->currentIndex() == 0) {
        ui->showSensitivityPlot->setText(mainWindowTr("FCM Sensitivity"));
    } else {
        ui->showSensitivityPlot->setText(mainWindowTr("Elements Sensitivity"));
    }
}

void SensitivityPresenter::analize() {
    if (!modelSetupPresenter->checkElementsHaveValues()) {
        return;
    }

    sensitivityScenePresenter = std::make_shared<SensitivityScenePresenter>(ui->plotSensitivity, creationPresenter);
    connect(sensitivityScenePresenter.get(), &SensitivityScenePresenter::updateProgress, this, &SensitivityPresenter::updateSensitivityProgress);
    sensitivityScenePresenter->activate();

    ui->pushButtonAnalizeSensitivity->setEnabled(false);
    ui->pushButtonResetSensitivity->setEnabled(true);
    ui->doubleSpinBoxMaxChange->setEnabled(false);
    ui->changeConcepts->setEnabled(false);
    ui->changeWeights->setEnabled(false);
    ui->sensitivityMeasureMetric->setEnabled(false);

    auto* sensitivityScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy(sensitivityScenePresenter, ElementWindowMode::SensitivityAnalysis);
    auto* oldSensitivityScene = ui->graphicsViewGraph->scene();
    ui->graphicsViewSensitivity->setScene(sensitivityScene);
    if (oldSensitivityScene != ui->graphicsViewGraph->scene()) {
        delete oldSensitivityScene;
    }

    sensitivityScenePresenter->setRuntimeContext(fcm, sensitivityScene->getFCM(), sensitivityScene);
    sensitivityScenePresenter->analize(modelSetupPresenter->getPredictionParameters(), getSensitivityParameters());
}

void SensitivityPresenter::changeActivationFunctionSensitivity(int index) {
    ui->fuzzinessDegreeSensitivity->setEnabled(index == 3 || index == 4);
}

void SensitivityPresenter::resetSensitivity() {
    if (!isActive()) {
        return;
    }

    ui->pushButtonAnalizeSensitivity->setEnabled(true);
    ui->pushButtonResetSensitivity->setEnabled(false);
    ui->doubleSpinBoxMaxChange->setEnabled(true);
    ui->changeConcepts->setEnabled(true);
    ui->changeWeights->setEnabled(true);
    ui->sensitivityMeasureMetric->setEnabled(true);
    sensitivityScenePresenter->reset();
    ui->progressBarSensitivity->setValue(0);
    auto* sensitivityScene = ui->graphicsViewSensitivity->scene();
    ui->graphicsViewSensitivity->setScene(ui->graphicsViewGraph->scene());
    delete sensitivityScene;
    ui->plotSensitivity->graph(0)->data()->clear();
    ui->plotSensitivity->replot();
}

void SensitivityPresenter::showSensitivityPlot() {
    int index = ui->stackedWidgetSensitivity->currentIndex();
    ui->stackedWidgetSensitivity->setCurrentIndex(index == 0 ? 1 : 0);
    retranslateUi();
}

void SensitivityPresenter::updateSensitivityProgress(double progress) {
    ui->progressBarSensitivity->setValue(static_cast<int>(progress * 100));
}

void SensitivityPresenter::updateSensitivityScaleLabel(double newScale) {
    ui->labelScaleSensitivity->setText(QString(mainWindowTr("Scale: %1%")).arg(newScale*100, 0, 'f', 2));
    sensitivityScale = newScale;
}
