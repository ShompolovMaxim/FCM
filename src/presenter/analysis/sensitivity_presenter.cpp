#include "sensitivity_presenter.h"

#include "presenter/models/creation_presenter.h"
#include "ui_main_window.h"

#include "ui/graph_editor/graph_scene.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>

namespace {
QString mainWindowTr(const char* text) {
    return QCoreApplication::translate("MainWindow", text);
}
}

SensitivityPresenter::SensitivityPresenter(
    Ui::MainWindow* ui,
    std::shared_ptr<FCM>& fcm,
    std::shared_ptr<CreationPresenter> creationPresenter,
    QWidget* parentWidget,
    QObject* parent
) : ui(ui), parentWidget(parentWidget), creationPresenter(creationPresenter), fcm(fcm), QObject{parent} {
    ui->comboBoxAlgorithmSensitivity->setItemData(0, "const weights", Qt::UserRole);
    ui->comboBoxAlgorithmSensitivity->setItemData(1, "changing weights", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(0, "bivalent", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(1, "trivalent", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(2, "threshold-linear", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(3, "sigmoid", Qt::UserRole);
    ui->comboBoxActivationSensitivity->setItemData(4, "hyperbolic tangent", Qt::UserRole);
    ui->comboBoxMetricSensitivity->setItemData(0, "MSE", Qt::UserRole);
    ui->comboBoxMetricSensitivity->setItemData(1, "MAE", Qt::UserRole);
    ui->comboBoxMetricSensitivity->setItemData(2, "MAPE", Qt::UserRole);
    ui->sensitivityMeasureMetric->setItemData(0, "MSE", Qt::UserRole);
    ui->sensitivityMeasureMetric->setItemData(1, "MAE", Qt::UserRole);
    ui->sensitivityMeasureMetric->setItemData(2, "MAPE", Qt::UserRole);

    ui->plotSensitivity->addGraph();
    ui->plotSensitivity->yAxis->setRange(-0.1, 1.1);
    ui->plotSensitivity->xAxis->setLabel(tr("max change"));
    ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
    ui->plotSensitivity->setGeometry(ui->graphicsViewSensitivity->geometry());

    connect(ui->graphicsViewSensitivity, &GraphView::scaleChanged, this, &SensitivityPresenter::updateSensitivityScaleLabel);
    connect(ui->pushButtonAnalizeSensitivity, &QPushButton::clicked, this, &SensitivityPresenter::analize);
    connect(ui->pushButtonResetSensitivity, &QPushButton::clicked, this, &SensitivityPresenter::resetSensitivity);
    connect(ui->showSensitivityPlot, &QPushButton::clicked, this, &SensitivityPresenter::showSensitivityPlot);

    connect(ui->comboBoxAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxAlgorithmSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->useFuzzyValues, &QCheckBox::toggled, ui->useFuzzyValuesSensitivity, &QCheckBox::setChecked);
    connect(ui->comboBoxActivation, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxActivationSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->fuzzinessDegree, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->fuzzinessDegreeSensitivity, &QDoubleSpinBox::setValue);
    connect(ui->checkBoxPredictToStatic, &QCheckBox::toggled, ui->checkBoxPredictToStaticSensitivity, &QCheckBox::setChecked);
    connect(ui->comboBoxMetric, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->comboBoxMetricSensitivity, &QComboBox::setCurrentIndex);
    connect(ui->doubleSpinBoxThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->doubleSpinBoxThresholdSensitivity, &QDoubleSpinBox::setValue);
    connect(ui->spinBoxMetricSteps, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxMetricStepsSensitivity, &QSpinBox::setValue);
    connect(ui->spinBoxFixedSteps, QOverload<int>::of(&QSpinBox::valueChanged), ui->spinBoxFixedStepsSensitivity, &QSpinBox::setValue);
}

SensitivityAnalysisParameters SensitivityPresenter::getSensitivityParameters() {
    const QSettings settings("app.ini", QSettings::IniFormat);

    return {
        ui->doubleSpinBoxMaxChange->value(),
        ui->changeConcepts->isChecked(),
        ui->changeWeights->isChecked(),
        settings.value("sensitivity/changeSteps", 10).toInt(),
        settings.value("sensitivity/maxIterations", 1000).toInt(),
        ui->sensitivityMeasureMetric->currentData(Qt::UserRole).toString()
    };
}

bool SensitivityPresenter::isActive() const {
    return sensitivityScenePresenter && sensitivityScenePresenter->isActive();
}

void SensitivityPresenter::reconfigure() {
    const QSettings settings("app.ini", QSettings::IniFormat);

    if (isActive()) {
        resetSensitivity();
    }

    sensitivityScenePresenter.reset();

    ui->progressBarSensitivity->setValue(0);
    ui->stackedWidgetSensitivity->setCurrentIndex(0);
    ui->doubleSpinBoxMaxChange->setValue(settings.value("sensitivity/defaultMaxChange", 0.1).toDouble());
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
    QString errorMessage;
    if (!fcm->checkElementsHaveValues(&errorMessage)) {
        QMessageBox::critical(parentWidget, mainWindowTr("Error"), mainWindowTr(errorMessage.toUtf8().constData()));
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
    sensitivityScenePresenter->analize(fcm->predictionParameters, getSensitivityParameters());
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
