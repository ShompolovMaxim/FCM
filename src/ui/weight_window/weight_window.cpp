#include "weight_window.h"
#include "ui_weight_window.h"

#include <QPushButton>

WeightWindow::WeightWindow(const std::map<size_t, std::shared_ptr<Term>>& terms, std::shared_ptr<Weight> currentWeight, QWidget *parent)
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
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->yAxis->setRange(-1.1, 1.1);
    ui->plot->xAxis->setLabel("step");
    ui->plot->yAxis->setLabel("weight value");
    ui->plot->graph(0)->setPen(QPen(Qt::red));
    ui->plot->graph(1)->setPen(QPen(QColorConstants::Svg::orange));
    ui->plot->graph(2)->setPen(QPen(Qt::green));

    ui->plotSensitivity->addGraph();
    ui->plotSensitivity->yAxis->setRange(-0.1, 1.1);
    ui->plotSensitivity->xAxis->setLabel("change");
    ui->plotSensitivity->yAxis->setLabel("sensitivity");

    setPredictedValues();
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

    for (const auto& [_, term] : terms)
    {
        textTicker->addTick(term->value, term->name);
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
        textTicker->addTick(term->value, term->name);
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
    ui->plotSensitivity->replot();
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
