#include "save_as_window.h"
#include "ui_save_as_window.h"

#include <QMessageBox>

SaveAsWindow::SaveAsWindow(QStringList modelsNames, QString modelName, const QString &windowTitle, QWidget *parent) : modelsNames(modelsNames), QDialog(parent), ui(new Ui::SaveAsWindow) {
    ui->setupUi(this);
    setWindowTitle(windowTitle);
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

void SaveAsWindow::accept() {
    if (modelsNames.contains(ui->savingName->text())) {
        QMessageBox::critical(this, tr("Error"), tr("This name is already taken"));
        return;
    }
    QDialog::accept();
}

void SaveAsWindow::onModelDoubleClicked(QListWidgetItem* item) {
    if (item) {
        ui->savingName->setText(item->text());
    }
}
