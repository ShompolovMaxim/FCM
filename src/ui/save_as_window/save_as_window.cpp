#include "save_as_window.h"
#include "ui_save_as_window.h"

SaveAsWindow::SaveAsWindow(QStringList modelsNames, QString modelName, QWidget *parent) : QDialog(parent), ui(new Ui::SaveAsWindow) {
    ui->setupUi(this);
    ui->modelsNames->addItems(modelsNames);
    ui->savingName->setText(modelName);
    connect(ui->modelsNames, &QListWidget::itemDoubleClicked, this, &SaveAsWindow::onModelDoubleClicked);
}

SaveAsWindow::~SaveAsWindow() {
    delete ui;
}

QString SaveAsWindow::savingModelName() const {
    return ui->savingName->text();
}

void SaveAsWindow::onModelDoubleClicked(QListWidgetItem* item) {
    if (item) {
        ui->savingName->setText(item->text());
    }
}
