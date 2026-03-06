#include "weight_window.h"
#include "ui_weight_window.h"

#include <QPushButton>

WeightWindow::WeightWindow(const std::map<size_t, Term>& terms, Weight currentWeight, QWidget *parent)
    : terms(terms), currentWeight(currentWeight), QDialog(parent), ui(new Ui::WeightWindow)
{
    ui->setupUi(this);

    if (currentWeight.name.isEmpty()) {
        setWindowTitle("Create weight");
    } else {
        setWindowTitle(currentWeight.name);
        ui->nameField->setText(currentWeight.name);
    }
    ui->notesField->setPlainText(currentWeight.description);

    QStringList termsNames = {};
    for (const auto& term : terms) {
        termsNames.append(term.second.name);
    }
    termsNames.sort();
    ui->valueField->addItems(termsNames);

    auto currentTerm = fuzzifier->fuzzify(terms, currentWeight.value);
    size_t currentTermInd = 0;
    for (size_t i = 0; i < termsNames.size(); ++i) {
        if (termsNames[i] == currentTerm.name) {
            currentTermInd = i;
        }
    }
    ui->valueField->setCurrentIndex(currentTermInd);

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &WeightWindow::onApplyClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &WeightWindow::onOkClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::close);
}

WeightWindow::~WeightWindow()
{
    delete ui;
}

void WeightWindow::onApplyClicked()
{
    updateCurrentWeight();
    emit applied(currentWeight);
    setWindowTitle(currentWeight.name);
}

void WeightWindow::onOkClicked()
{
    updateCurrentWeight();
    emit applied(currentWeight);
    close();
}

void WeightWindow::updateCurrentWeight() {
    currentWeight.name = ui->nameField->text();
    currentWeight.description = ui->notesField->toPlainText();

    Term t;
    for (const auto& term : terms) {
        if (term.second.name == ui->valueField->currentText()) {
            currentWeight.value = term.second.value;
            break;
        }
    }
}
