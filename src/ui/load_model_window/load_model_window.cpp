#include "load_model_window.h"
#include "ui_load_model_window.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>

LoadModelWindow::LoadModelWindow(QStringList modelsNames, const QString &windowTitle, QWidget *parent) : QDialog(parent) , ui(new Ui::LoadModelWindow) {
    ui->setupUi(this);
    setWindowTitle(windowTitle);

    for (const auto& modelName : modelsNames) {
        addModelItem(modelName);
    }

    if (!modelsNames.isEmpty()) {
        ui->modelsNames->setCurrentRow(0);
    }

    updateButtonsState();
}

LoadModelWindow::~LoadModelWindow() {
    delete ui;
}

QString LoadModelWindow::selectedModelName() const
{
    auto item = ui->modelsNames->currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString{};
}

void LoadModelWindow::addModelItem(const QString &modelName) {
    auto* item = new QListWidgetItem(ui->modelsNames);
    item->setData(Qt::UserRole, modelName);

    auto* widget = new QWidget(ui->modelsNames);
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(8, 4, 8, 4);

    auto* label = new QLabel(modelName, widget);
    auto* deleteButton = new QPushButton(tr("Delete"), widget);
    deleteButton->setProperty("modelName", modelName);

    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(deleteButton);

    item->setSizeHint(widget->sizeHint());
    ui->modelsNames->setItemWidget(item, widget);

    connect(deleteButton, &QPushButton::clicked, this, &LoadModelWindow::onDeleteButtonClicked);
}

void LoadModelWindow::updateButtonsState() {
    auto* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setEnabled(ui->modelsNames->count() > 0);
    }
}

void LoadModelWindow::onModelDeleted(const QString &modelName, bool success) {
    if (!success) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to delete \"%1\".").arg(modelName));
        return;
    }

    removeModel(modelName);
}

void LoadModelWindow::onDeleteButtonClicked() {
    auto* deleteButton = qobject_cast<QPushButton*>(sender());
    if (!deleteButton) {
        return;
    }

    const QString modelName = deleteButton->property("modelName").toString();
    const auto reply = QMessageBox::question(
        this,
        tr("Delete"),
        tr("Delete \"%1\"?").arg(modelName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    emit deleteModelRequested(modelName);
}

void LoadModelWindow::removeModel(const QString &modelName) {
    for (int i = 0; i < ui->modelsNames->count(); ++i) {
        auto* item = ui->modelsNames->item(i);
        if (item->data(Qt::UserRole).toString() != modelName) {
            continue;
        }

        delete ui->modelsNames->takeItem(i);
        if (ui->modelsNames->count() > 0 && !ui->modelsNames->currentItem()) {
            ui->modelsNames->setCurrentRow(0);
        }
        updateButtonsState();
        return;
    }
}
