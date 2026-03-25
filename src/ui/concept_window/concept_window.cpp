#include "concept_window.h"
#include "ui_concept_window.h"

#include <QPushButton>

ConceptWindow::ConceptWindow(const std::map<size_t, std::shared_ptr<Term>>& terms, std::shared_ptr<Concept> currentConcept, std::vector<double> predictedValues, QWidget *parent)
    : terms(terms), currentConcept(currentConcept), QDialog(parent), ui(new Ui::ConceptWindow)
{
    ui->setupUi(this);

    if (currentConcept->name.isEmpty()) {
        setWindowTitle("Create concept");
        creation = true;
    } else {
        setWindowTitle(currentConcept->name);
        ui->nameField->setText(currentConcept->name);
        creation = false;
    }
    ui->notesField->setPlainText(currentConcept->description);
    ui->startStepField->setValue(currentConcept->startStep);

    updateTermsList();

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ConceptWindow::onApplyClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ConceptWindow::onOkClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ConceptWindow::onCancelClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &ConceptWindow::onDelete);

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
        textTicker->addTick(term->value, term->name);
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

void ConceptWindow::updateTermsList() {
    QStringList termsNames = {};
    for (const auto& [id, term] : terms) {
        if (term->type == ElementType::Node) {
            termsNames.append(term->name);
        }
    }
    termsNames.sort();
    termsNames.prepend("");
    ui->valueField->clear();
    ui->valueField->addItems(termsNames);

    if (currentConcept->term) {
        ui->valueField->setCurrentText(currentConcept->term->name);
    } else {
        ui->valueField->setCurrentText("");
    }
}

ConceptWindow::~ConceptWindow()
{
    delete ui;
}

void ConceptWindow::onApplyClicked()
{
    updateCurrentConcept();
    emit applied(currentConcept);
    creation = false;
    setWindowTitle(currentConcept->name);
}

void ConceptWindow::onCancelClicked() {
    if (creation) {
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
    creation = false;
    close();
}

void ConceptWindow::updateCurrentConcept() {
    currentConcept->name = ui->nameField->text();
    currentConcept->description = ui->notesField->toPlainText();
    currentConcept->startStep = ui->startStepField->value();

    for (const auto& [id, term] : terms) {
        if (term->name == ui->valueField->currentText()) {
            currentConcept->term = term;
            return;
        }
    }
    currentConcept->term = nullptr;
}
