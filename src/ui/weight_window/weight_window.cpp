#include "weight_window.h"
#include "ui_weight_window.h"

#include <QPushButton>

WeightWindow::WeightWindow(const std::map<QUuid, std::shared_ptr<Term>>& terms, std::shared_ptr<Weight> currentWeight, ElementWindowMode mode, QWidget *parent)
    : terms(terms), currentWeight(currentWeight), QDialog(parent), mode(mode), ui(new Ui::WeightWindow) {
    ui->setupUi(this);

    if (mode == ElementWindowMode::CreateElement) {
        setWindowTitle(tr("Create weight"));
    } else {
        if (currentWeight->name.isEmpty()) {
            setWindowTitle(tr("Update weight"));
        } else {
            setWindowTitle(currentWeight->name);
        }
        ui->nameField->setText(currentWeight->name);
    }
    ui->notesField->setMarkdownText(currentWeight->description);

    updateTermsList();

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &WeightWindow::onApplyClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &WeightWindow::onOkClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &WeightWindow::onCancelClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &WeightWindow::onDelete);

    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->yAxis->setRange(-1.1, 1.1);
    ui->plot->xAxis->setLabel(tr("step"));
    ui->plot->yAxis->setLabel(tr("weight value"));
    ui->plot->graph(0)->setPen(QPen(Qt::red));
    ui->plot->graph(1)->setPen(QPen(QColorConstants::Svg::orange));
    ui->plot->graph(2)->setPen(QPen(Qt::green));

    ui->plotSensitivity->addGraph();
    ui->plotSensitivity->yAxis->setRange(-0.1, 1.1);
    ui->plotSensitivity->xAxis->setLabel(tr("change"));
    ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));

    setPredictedValues();

    if (mode == ElementWindowMode::PredictionResults || mode == ElementWindowMode::PredictionResultsDisabled) {
        ui->tabWidget->setCurrentIndex(1);
    }
    if (mode == ElementWindowMode::SensitivityAnalysis || mode == ElementWindowMode::SensitivityAnalysisDisabled) {
        ui->tabWidget->setCurrentIndex(2);
    }
    if (mode == ElementWindowMode::PredictionResultsDisabled || mode == ElementWindowMode::SensitivityAnalysisDisabled) {
        ui->nameField->setEnabled(false);
        ui->valueField->setEnabled(false);
        ui->notesField->setEnabled(false);
        ui->buttonBox->setEnabled(false);
        ui->deleteButton->setEnabled(false);
    }
    if (mode == ElementWindowMode::PredictionResultsDisabled) {
        ui->tabWidget->setTabEnabled(2, false);
    }
    if (mode == ElementWindowMode::SensitivityAnalysisDisabled) {
        ui->tabWidget->setTabEnabled(1, false);
    }
}

WeightWindow::~WeightWindow() {
    delete ui;
}

void WeightWindow::setPredictedValues() {
    if (std::holds_alternative<std::vector<double>>(currentWeight->predictedValues)) {
        setNumericPredictedValues();
    } else {
        setFuzzyPredictedValues();
    }
    setSensitivity();
}

void WeightWindow::setNumericPredictedValues() {
    auto predictedValues = std::get<std::vector<double>>(currentWeight->predictedValues);
    QVector<double> x;
    for (size_t i = 0; i < predictedValues.size(); ++i) {
        x.push_back(i + 1);
    }

    ui->plot->graph(0)->setData(x, QVector<double>(predictedValues.begin(), predictedValues.end()));
    ui->plot->graph(0)->rescaleKeyAxis();

    auto textTicker = QSharedPointer<QCPAxisTickerText>::create();

    for (const auto& [_, term] : terms) {
        if (term->type == ElementType::Edge) {
            textTicker->addTick(term->value, term->name);
        }
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

void WeightWindow::setFuzzyPredictedValues() {
    auto predictedValues = std::get<std::vector<TriangularFuzzyValue>>(currentWeight->predictedValues);
    QVector<double> x;
    QVector<double> l;
    QVector<double> m;
    QVector<double> u;
    for (size_t i = 0; i < predictedValues.size(); ++i) {
        x.push_back(i + 1);
        l.push_back(predictedValues[i].l);
        m.push_back(predictedValues[i].m);
        u.push_back(predictedValues[i].u);
    }

    ui->plot->graph(0)->setData(x, l);
    ui->plot->graph(0)->rescaleKeyAxis();
    ui->plot->graph(1)->setData(x, m);
    ui->plot->graph(1)->rescaleKeyAxis();
    ui->plot->graph(2)->setData(x, u);
    ui->plot->graph(2)->rescaleKeyAxis();

    auto textTicker = QSharedPointer<QCPAxisTickerText>::create();

    for (const auto& [_, term] : terms) {
        if (term->type == ElementType::Edge) {
            textTicker->addTick(term->value, term->name);
        }
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

void WeightWindow::setSensitivity() {
    QVector<double> x;
    QVector<double> y;
    for (const auto& [change, sensitivity] : currentWeight->sensitivity) {
        x.push_back(change);
        y.push_back(sensitivity);
    }
    ui->plotSensitivity->graph(0)->setData(x, y);
    ui->plotSensitivity->graph(0)->rescaleKeyAxis();
    ui->plotSensitivity->graph(0)->rescaleValueAxis();
    ui->plotSensitivity->replot();
}

void WeightWindow::updateTermsList() {
    ui->valueField->clear();
    ui->valueField->addItem("", QVariant());

    std::vector<Term> sortedTerms;
    for (const auto& [id, term] : terms) {
        if (term->type == ElementType::Edge) {
            sortedTerms.push_back(*term);
        }
    }
    std::sort(sortedTerms.begin(), sortedTerms.end(), [](const auto& a, const auto& b){
        return a.value + a.fuzzyValue.defuzzify() < b.value + b.fuzzyValue.defuzzify();
    });

    for (const auto& term : sortedTerms) {
        ui->valueField->addItem(term.name, QVariant::fromValue(term.id));
    }

    if (currentWeight->term) {
        QUuid termId = currentWeight->term->id;
        int index = ui->valueField->findData(QVariant::fromValue(termId));
        if (index >= 0) ui->valueField->setCurrentIndex(index);
        else ui->valueField->setCurrentIndex(0);
    } else {
        ui->valueField->setCurrentIndex(0);
    }
}

void WeightWindow::onApplyClicked() {
    updateCurrentWeight();
    emit applied(currentWeight);
    if (currentWeight->name.isEmpty()) {
        setWindowTitle(tr("Update weight"));
    } else {
        setWindowTitle(currentWeight->name);
    }
    if (mode == ElementWindowMode::CreateElement) {
        mode = ElementWindowMode::UpdateElement;
    }
}

void WeightWindow::onOkClicked() {
    updateCurrentWeight();
    emit applied(currentWeight);
    close();
}

void WeightWindow::onCancelClicked() {
    if (mode == ElementWindowMode::CreateElement) {
        emit deleted(currentWeight->id);
    }
    close();
}

void WeightWindow::closeEvent(QCloseEvent* event) {
    onCancelClicked();
}

void WeightWindow::onDelete() {
    emit deleted(currentWeight->id);
    close();
}

void WeightWindow::updateCurrentWeight() {
    currentWeight->name = ui->nameField->text();
    currentWeight->description = ui->notesField->markdownText();

    QVariant data = ui->valueField->currentData();
    if (data.isValid()) {
        QUuid id = data.toUuid();
        auto it = terms.find(id);
        if (it != terms.end()) {
            currentWeight->term = it->second;
            return;
        }
    }
    currentWeight->term = nullptr;
}

void WeightWindow::retranslate() {
    ui->retranslateUi(this);
    if (currentWeight->name.isEmpty()) {
        setWindowTitle(tr("Create weight"));
    } else {
        if (currentWeight->name.isEmpty()) {
            setWindowTitle(tr("Update weight"));
        } else {
            setWindowTitle(currentWeight->name);
        }
    }
    ui->plot->xAxis->setLabel(tr("step"));
    ui->plot->yAxis->setLabel(tr("weight value"));
    ui->plotSensitivity->xAxis->setLabel(tr("change"));
    ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
    ui->plot->replot();
    ui->plotSensitivity->replot();
}

