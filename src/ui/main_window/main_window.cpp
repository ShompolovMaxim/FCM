#include "main_window.h"
#include "ui_main_window.h"
#include "ui/graph_editor/graph_scene.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    fcm = std::make_shared<FCM>();

    auto* scene = new GraphScene(fcm);
    ui->graphicsViewGraph->setScene(scene);
    ui->graphicsViewPredict->setScene(scene);

    QObject::connect(ui->pushButtonMode, &QPushButton::clicked, scene, &GraphScene::switchMode);
    QObject::connect(scene, &GraphScene::modeChanged, this, &MainWindow::updateModeButtonText);
    QObject::connect(ui->graphicsViewGraph, &GraphView::scaleChanged, this, &MainWindow::updateGraphScaleLabel);
    QObject::connect(ui->graphicsViewPredict, &GraphView::scaleChanged, this, &MainWindow::updatePredictScaleLabel);
    QObject::connect(ui->pushButtonScaleGraph, &QPushButton::clicked, ui->graphicsViewGraph, &GraphView::resetScale);
    QObject::connect(ui->pushButtonScalePredict, &QPushButton::clicked, ui->graphicsViewPredict, &GraphView::resetScale);
    QObject::connect(ui->pushButtonPredict, &QPushButton::clicked, this, &MainWindow::predict);
    QObject::connect(ui->pushButtonReset, &QPushButton::clicked, this, &MainWindow::resetPredictionScene);

    ui->listWidgetTerms->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidgetTerms, &QListWidget::customContextMenuRequested, this, &MainWindow::onListWidgetContextMenu);
    ui->listWidgetTerms->setDragEnabled(true);
    ui->listWidgetTerms->setAcceptDrops(true);
    ui->listWidgetTerms->setDropIndicatorShown(true);
    ui->listWidgetTerms->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->listWidgetTerms, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);

    connect(ui->termValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueChanged);
    connect(ui->termValueL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueLChanged);
    connect(ui->termValueM, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueMChanged);
    connect(ui->termValueU, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onTermValueUChanged);
    connect(ui->listWidgetTerms, &QListWidget::itemChanged, this, &MainWindow::onItemChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateGraphScaleLabel(double newScale) {
    ui->labelScaleGraph->setText(QString("Scale: %1%").arg(newScale*100, 0, 'f', 2));
}

void MainWindow::updatePredictScaleLabel(double newScale) {
    ui->labelScalePredict->setText(QString("Scale: %1%").arg(newScale*100, 0, 'f', 2));
}

void MainWindow::updateModeButtonText(EditMode newMode) {
    ui->pushButtonMode->setText(newMode == EditMode::EditValues ? "Mode: Edit values" : "Mode: Create");
}

void MainWindow::predict() {
    auto predictionParameters = PredictionParameters{
        ui->comboBoxAlgorithm->currentText(),
        ui->comboBoxActivation->currentText(),
        ui->comboBoxMetric->currentText(),
        ui->checkBoxPredictToStatic->isChecked(),
        ui->doubleSpinBoxThreshold->value(),
        ui->spinBoxMetricSteps->value(),
        ui->spinBoxFixedSteps->value()
    };

    auto simulationParameters = SimulationParameters{
        ui->checkBoxRealTime->isChecked(),
        ui->doubleSpinBoxStepsPerSecond->value()
    };

    auto* predictScene = dynamic_cast<GraphScene*>(ui->graphicsViewGraph->scene())->copy();
    auto* oldPredictScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewPredict->setScene(predictScene);
    if (oldPredictScene != ui->graphicsViewGraph->scene()) {
        delete oldPredictScene;
    }

    QList<NodeItem*> nodes;
    for (QGraphicsItem* item : predictScene->items()) {
        if (auto n = qgraphicsitem_cast<NodeItem*>(item)) {
            nodes.append(n);
        }
    }

    presenter.simulate(predictionParameters, simulationParameters, nodes);
}

void MainWindow::resetPredictionScene() {
    auto* predictionScene = ui->graphicsViewPredict->scene();
    ui->graphicsViewPredict->setScene(ui->graphicsViewGraph->scene());
    delete predictionScene;
}

void MainWindow::onListWidgetContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->listWidgetTerms->itemAt(pos);

    QMenu menu(this);

    QAction *addAction = menu.addAction("Добавить");
    QAction *deleteAction = menu.addAction("Удалить");

    QAction *selectedAction = menu.exec(ui->listWidgetTerms->mapToGlobal(pos));

    if (selectedAction == addAction) {
        QListWidgetItem *newItem = new QListWidgetItem("Новый элемент");
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

        ui->listWidgetTerms->addItem(newItem);

        ui->listWidgetTerms->setCurrentItem(newItem);
        ui->listWidgetTerms->editItem(newItem);

        terms[newItem];
        terms[newItem].name = "Новый элемент";
        terms[newItem].id = ++fcm->termsCounter;
        fcm->terms[terms[newItem].id] = terms[newItem];

    }
    if (selectedAction == deleteAction && item) {
        delete item;
    }
}

void MainWindow::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->termValue->setEnabled(true);
    ui->termValueL->setEnabled(true);
    ui->termValueM->setEnabled(true);
    ui->termValueU->setEnabled(true);
    currentTerm = current;

    ui->termValue->setValue(terms[current].value);
    ui->termValueL->setValue(terms[current].fuzzyValueL);
    ui->termValueM->setValue(terms[current].fuzzyValueM);
    ui->termValueU->setValue(terms[current].fuzzyValueU);
}

void MainWindow::onTermValueChanged(double value) {
    terms[currentTerm].value = value;
    fcm->terms[terms[currentTerm].id].value = value;
}

void MainWindow::onTermValueLChanged(double value) {
    terms[currentTerm].fuzzyValueL = value;
    fcm->terms[terms[currentTerm].id].fuzzyValueL = value;
}

void MainWindow::onTermValueMChanged(double value) {
    terms[currentTerm].fuzzyValueM = value;
    fcm->terms[terms[currentTerm].id].fuzzyValueM = value;
}

void MainWindow::onTermValueUChanged(double value) {
    terms[currentTerm].fuzzyValueU = value;
    fcm->terms[terms[currentTerm].id].fuzzyValueU = value;
}

void MainWindow::onItemChanged(QListWidgetItem *item)
{
    terms[item].name = item->text();
    fcm->terms[terms[item].id].name = item->text();
}

