#include "save_as_window.h"
#include "ui_save_as_window.h"

#include <QMessageBox>

SaveAsWindow::SaveAsWindow(
    QStringList modelsNames,
    QStringList existingModelsNames,
    QString modelName,
    const QString &windowTitle,
    QWidget *parent
) : modelsNames(modelsNames), existingModelsNames(existingModelsNames), QDialog(parent), ui(new Ui::SaveAsWindow) {
    ui->setupUi(this);
    setWindowTitle(windowTitle);
    modelsNames.sort(Qt::CaseInsensitive);
    ui->modelsNames->setWordWrap(true);
    ui->modelsNames->setTextElideMode(Qt::ElideNone);
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
    if (existingModelsNames.contains(ui->savingName->text())) {
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
