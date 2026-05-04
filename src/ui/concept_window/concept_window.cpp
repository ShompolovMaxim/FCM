#include "concept_window.h"
#include "ui_concept_window.h"

#include <QPushButton>

ConceptWindow::ConceptWindow(const std::map<QUuid, std::shared_ptr<Term>>& terms, std::shared_ptr<Concept> currentConcept, ElementWindowMode mode, QWidget *parent)
    : terms(terms), currentConcept(currentConcept), QDialog(parent), mode(mode), ui(new Ui::ConceptWindow) {
    ui->setupUi(this);

    if (mode == ElementWindowMode::CreateElement) {
        setWindowTitle(tr("Create concept"));
    } else {
        if (currentConcept->name.isEmpty()) {
            setWindowTitle(tr("Update concept"));
        } else {
            setWindowTitle(currentConcept->name);
        }
        ui->nameField->setText(currentConcept->name);
    }
    ui->notesField->setMarkdownText(currentConcept->description);
    ui->startStepField->setValue(currentConcept->startStep);

    updateTermsList();

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ConceptWindow::onApplyClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ConceptWindow::onOkClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ConceptWindow::onCancelClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &ConceptWindow::onDelete);

    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->yAxis->setRange(-0.1, 1.1);
    ui->plot->xAxis->setLabel(tr("step"));
    ui->plot->yAxis->setLabel(tr("concept value"));
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
        ui->startStepField->setEnabled(false);
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

void ConceptWindow::setPredictedValues() {
    if (std::holds_alternative<std::vector<double>>(currentConcept->predictedValues)) {
        setNumericPredictedValues();
    } else {
        setFuzzyPredictedValues();
    }
    setSensitivity();
}

void ConceptWindow::setNumericPredictedValues() {
    auto predictedValues = std::get<std::vector<double>>(currentConcept->predictedValues);
    QVector<double> x;
    for (size_t i = 0; i < predictedValues.size(); ++i) {
        x.push_back(i + 1);
    }

    ui->plot->graph(0)->setData(x, QVector<double>(predictedValues.begin(), predictedValues.end()));
    ui->plot->graph(0)->rescaleKeyAxis();

    auto textTicker = QSharedPointer<QCPAxisTickerText>::create();

    for (const auto& [_, term] : terms) {
        if (term->type == ElementType::Node) {
            textTicker->addTick(term->value, term->name);
        }
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

void ConceptWindow::setFuzzyPredictedValues() {
    auto predictedValues = std::get<std::vector<TriangularFuzzyValue>>(currentConcept->predictedValues);
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
        if (term->type == ElementType::Node) {
            textTicker->addTick(term->value, term->name);
        }
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

void ConceptWindow::setSensitivity() {
    QVector<double> x;
    QVector<double> y;
    for (const auto& [change, sensitivity] : currentConcept->sensitivity) {
        x.push_back(change);
        y.push_back(sensitivity);
    }
    ui->plotSensitivity->graph(0)->setData(x, y);
    ui->plotSensitivity->graph(0)->rescaleKeyAxis();
    ui->plotSensitivity->graph(0)->rescaleValueAxis();
    ui->plotSensitivity->replot();
}

void ConceptWindow::updateTermsList() {
    ui->valueField->clear();
    ui->valueField->addItem("", QVariant());

    std::vector<Term> sortedTerms;
    for (const auto& [id, term] : terms) {
        if (term->type == ElementType::Node) {
            sortedTerms.push_back(*term);
        }
    }
    std::sort(sortedTerms.begin(), sortedTerms.end(), [](const auto& a, const auto& b){
        return a.value + a.fuzzyValue.defuzzify() < b.value + b.fuzzyValue.defuzzify();
    });

    for (const auto& term : sortedTerms) {
        ui->valueField->addItem(term.name, QVariant::fromValue(term.id));
    }

    if (currentConcept->term) {
        QUuid termId = currentConcept->term->id;
        int index = ui->valueField->findData(QVariant::fromValue(termId));
        if (index >= 0) ui->valueField->setCurrentIndex(index);
        else ui->valueField->setCurrentIndex(0);
    } else {
        ui->valueField->setCurrentIndex(0);
    }
}

ConceptWindow::~ConceptWindow() {
    delete ui;
}

void ConceptWindow::onApplyClicked() {
    updateCurrentConcept();
    emit applied(currentConcept);
    if (currentConcept->name.isEmpty()) {
        setWindowTitle(tr("Update concept"));
    } else {
        setWindowTitle(currentConcept->name);
    }
    if (mode == ElementWindowMode::CreateElement) {
        mode = ElementWindowMode::UpdateElement;
    }
}

void ConceptWindow::onCancelClicked() {
    if (mode == ElementWindowMode::CreateElement) {
        emit deleted(currentConcept->id);
    }
    close();
}

void ConceptWindow::closeEvent(QCloseEvent* event) {
    onCancelClicked();
}

void ConceptWindow::onOkClicked()
{
    updateCurrentConcept();
    emit applied(currentConcept);
    close();
}

void ConceptWindow::onDelete() {
    emit deleted(currentConcept->id);
    close();
}

void ConceptWindow::updateCurrentConcept() {
    currentConcept->name = ui->nameField->text();
    currentConcept->description = ui->notesField->markdownText();
    currentConcept->startStep = ui->startStepField->value();

    QVariant data = ui->valueField->currentData();
    if (data.isValid()) {
        QUuid id = data.toUuid();
        auto it = terms.find(id);
        if (it != terms.end()) {
            currentConcept->term = it->second;
            return;
        }
    }
    currentConcept->term = nullptr;
}

void ConceptWindow::retranslate() {
    ui->retranslateUi(this);
    if (currentConcept->name.isEmpty()) {
        setWindowTitle(tr("Create concept"));
    } else {
        if (currentConcept->name.isEmpty()) {
            setWindowTitle(tr("Update concept"));
        } else {
            setWindowTitle(currentConcept->name);
        }
    }
    ui->plot->xAxis->setLabel(tr("step"));
    ui->plot->yAxis->setLabel(tr("concept value"));
    ui->plotSensitivity->xAxis->setLabel(tr("change"));
    ui->plotSensitivity->yAxis->setLabel(tr("sensitivity"));
    ui->plot->replot();
    ui->plotSensitivity->replot();
}
