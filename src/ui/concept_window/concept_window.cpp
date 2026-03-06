#include "concept_window.h"
#include "ui_concept_window.h"

#include "lib/qcustomplot/qcustomplot.h"

#include <QPushButton>

ConceptWindow::ConceptWindow(const std::map<size_t, Term>& terms, Concept currentConcept, std::vector<double> predictedValues, QWidget *parent)
    : terms(terms), currentConcept(currentConcept), QDialog(parent), ui(new Ui::ConceptWindow)
{
    ui->setupUi(this);

    if (currentConcept.name.isEmpty()) {
        setWindowTitle("Create concept");
    } else {
        setWindowTitle(currentConcept.name);
        ui->nameField->setText(currentConcept.name);
    }
    ui->notesField->setPlainText(currentConcept.description);
    ui->startStepField->setValue(currentConcept.startStep);

    QStringList termsNames = {};
    for (const auto& term : terms) {
        termsNames.append(term.second.name);
    }
    termsNames.sort();
    ui->valueField->addItems(termsNames);

    auto currentTerm = fuzzifier->fuzzify(terms, currentConcept.value);
    size_t currentTermInd = 0;
    for (size_t i = 0; i < termsNames.size(); ++i) {
        if (termsNames[i] == currentTerm.name) {
            currentTermInd = i;
        }
    }
    ui->valueField->setCurrentIndex(currentTermInd);

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ConceptWindow::onApplyClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ConceptWindow::onOkClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::close);

    ui->plot->addGraph();
    ui->plot->yAxis->setRange(0, 1.1);
    ui->plot->xAxis->setLabel("step");
    ui->plot->yAxis->setLabel("concept value");
    ui->plot->addGraph();
    setPredictedValues(predictedValues);
}

void ConceptWindow::setPredictedValues(std::vector<double> predictedValues) {
    QVector<double> x;
    for (size_t i = 0; i < predictedValues.size(); ++i) {
        x.push_back(i + 1);
    }

    ui->plot->graph(0)->setData(x, QVector<double>(predictedValues.begin(), predictedValues.end()));
    ui->plot->graph(0)->rescaleKeyAxis();

    auto textTicker = QSharedPointer<QCPAxisTickerText>::create();

    for (const auto& [_, term] : terms)
    {
        textTicker->addTick(term.value, term.name);
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

ConceptWindow::~ConceptWindow()
{
    delete ui;
}

void ConceptWindow::onApplyClicked()
{
    updateCurrentConcept();
    emit applied(currentConcept);
    setWindowTitle(currentConcept.name);
}

void ConceptWindow::onOkClicked()
{
    updateCurrentConcept();
    emit applied(currentConcept);
    close();
}

void ConceptWindow::updateCurrentConcept() {
    currentConcept.name = ui->nameField->text();
    currentConcept.description = ui->notesField->toPlainText();

    Term t;
    for (const auto& term : terms) {
        if (term.second.name == ui->valueField->currentText()) {
            currentConcept.value = term.second.value;
            break;
        }
    }

    currentConcept.startStep = ui->startStepField->value();
}
