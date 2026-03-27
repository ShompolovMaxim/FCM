#include "weight_window.h"
#include "ui_weight_window.h"

#include <QPushButton>

WeightWindow::WeightWindow(const std::map<size_t, std::shared_ptr<Term>>& terms, std::shared_ptr<Weight> currentWeight, std::vector<double> predictedValues, QWidget *parent)
    : terms(terms), currentWeight(currentWeight), QDialog(parent), ui(new Ui::WeightWindow)
{
    ui->setupUi(this);

    if (currentWeight->name.isEmpty()) {
        setWindowTitle("Create weight");
        creation = true;
    } else {
        setWindowTitle(currentWeight->name);
        ui->nameField->setText(currentWeight->name);
        creation = false;
    }
    ui->notesField->setPlainText(currentWeight->description);

    updateTermsList();

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &WeightWindow::onApplyClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &WeightWindow::onOkClicked);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &WeightWindow::onCancelClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &WeightWindow::onDelete);

    ui->plot->addGraph();
    ui->plot->yAxis->setRange(0, 1.1);
    ui->plot->xAxis->setLabel("step");
    ui->plot->yAxis->setLabel("weight value");
    ui->plot->addGraph();
    setPredictedValues(predictedValues);
}

WeightWindow::~WeightWindow() {
    delete ui;
}

void WeightWindow::setPredictedValues(std::vector<double> predictedValues) {
    QVector<double> x;
    for (size_t i = 0; i < predictedValues.size(); ++i) {
        x.push_back(i + 1);
    }

    ui->plot->graph(0)->setData(x, QVector<double>(predictedValues.begin(), predictedValues.end()));
    ui->plot->graph(0)->rescaleKeyAxis();

    auto textTicker = QSharedPointer<QCPAxisTickerText>::create();

    for (const auto& [_, term] : terms) {
        textTicker->addTick(term->value, term->name);
    }
    ui->plot->yAxis->setTicker(textTicker);
    ui->plot->yAxis->setTickLabelRotation(0);
    ui->plot->yAxis->setSubTicks(false);

    ui->plot->replot();
}

void WeightWindow::updateTermsList() {
    ui->valueField->clear();
    ui->valueField->addItem("", QVariant());

    std::vector<std::pair<QString, size_t>> sortedTerms;
    for (const auto& [id, term] : terms) {
        if (term->type == ElementType::Edge) {
            sortedTerms.emplace_back(term->name, id);
        }
    }
    std::sort(sortedTerms.begin(), sortedTerms.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });

    for (const auto& [name, id] : sortedTerms) {
        ui->valueField->addItem(name, QVariant::fromValue(id));
    }

    if (currentWeight->term) {
        size_t termId = currentWeight->term->id;
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
    creation = false;
    setWindowTitle(currentWeight->name);
}

void WeightWindow::onOkClicked() {
    updateCurrentWeight();
    emit applied(currentWeight);
    close();
}

void WeightWindow::onCancelClicked() {
    if (creation) {
        emit deleted(currentWeight->id);
    }
    close();
}

void WeightWindow::closeEvent(QCloseEvent* event) {
    onCancelClicked();
}

void WeightWindow::onDelete() {
    emit deleted(currentWeight->id);
    creation = false;
    close();
}

void WeightWindow::updateCurrentWeight() {
    currentWeight->name = ui->nameField->text();
    currentWeight->description = ui->notesField->toPlainText();

    QVariant data = ui->valueField->currentData();
    if (data.isValid()) {
        size_t id = data.toULongLong();
        auto it = terms.find(id);
        if (it != terms.end()) {
            currentWeight->term = it->second;
            return;
        }
    }
    currentWeight->term = nullptr;
}
