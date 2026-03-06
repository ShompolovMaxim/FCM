#include "load_model_window.h"
#include "ui_load_model_window.h"

LoadModelWindow::LoadModelWindow(QStringList modelsNames, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoadModelWindow)
{
    ui->setupUi(this);

    ui->modelsNames->addItems(modelsNames);

    if (!modelsNames.isEmpty()) {
        ui->modelsNames->setCurrentRow(0);
    }
}

LoadModelWindow::~LoadModelWindow()
{
    delete ui;
}

QString LoadModelWindow::selectedModelName() const
{
    auto item = ui->modelsNames->currentItem();
    return item ? item->text() : QString{};
}
