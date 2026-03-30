#include "concept_window.h"
#include "ui_concept_window.h"

#include <QPushButton>

ConceptWindow::ConceptWindow(const std::map<size_t, std::shared_ptr<Term>>& terms, std::shared_ptr<Concept> currentConcept, QWidget *parent)
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
    ui->plot->yAxis->setRange(-0.1, 1.1);
    ui->plot->xAxis->setLabel("step");
    ui->plot->yAxis->setLabel("concept value");
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::red));
    ui->plot->graph(1)->setPen(QPen(QColorConstants::Svg::orange));
    ui->plot->graph(2)->setPen(QPen(Qt::green));
    setPredictedValues();
}

void ConceptWindow::setPredictedValues() {
    if (std::holds_alternative<std::vector<double>>(currentConcept->predictedValues)) {
        setNumericPredictedValues();
    } else {
        setFuzzyPredictedValues();
    }
}

void ConceptWindow::updateTermsList() {
    ui->valueField->clear();
    ui->valueField->addItem("", QVariant());

    std::vector<std::pair<QString, size_t>> sortedTerms;
    for (const auto& [id, term] : terms) {
        if (term->type == ElementType::Node) {
            sortedTerms.emplace_back(term->name, id);
        }
    }
    std::sort(sortedTerms.begin(), sortedTerms.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });

    for (const auto& [name, id] : sortedTerms) {
        ui->valueField->addItem(name, QVariant::fromValue(id));
    }

    if (currentConcept->term) {
        size_t termId = currentConcept->term->id;
        int index = ui->valueField->findData(QVariant::fromValue(termId));
        if (index >= 0) ui->valueField->setCurrentIndex(index);
        else ui->valueField->setCurrentIndex(0);
    } else {
        ui->valueField->setCurrentIndex(0);
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

    QVariant data = ui->valueField->currentData();
    if (data.isValid()) {
        size_t id = data.toULongLong();
        auto it = terms.find(id);
        if (it != terms.end()) {
            currentConcept->term = it->second;
            return;
        }
    }
    currentConcept->term = nullptr;
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

    for (const auto& [_, term] : terms)
    {
        textTicker->addTick(term->value, term->name);
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
    ui->plot->graph(3)->rescaleKeyAxis();

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
