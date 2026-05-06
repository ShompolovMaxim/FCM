#include "load_model_window.h"
#include "ui_load_model_window.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSizePolicy>

LoadModelWindow::LoadModelWindow(QStringList modelsNames, const QString &windowTitle, QWidget *parent) : QDialog(parent) , ui(new Ui::LoadModelWindow) {
    ui->setupUi(this);
    setWindowTitle(windowTitle);

    modelsNames.sort(Qt::CaseInsensitive);
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

void LoadModelWindow::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    updateItemsSizeHints();
}

void LoadModelWindow::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    updateItemsSizeHints();
}

void LoadModelWindow::addModelItem(const QString &modelName) {
    auto* item = new QListWidgetItem(ui->modelsNames);
    item->setData(Qt::UserRole, modelName);

    auto* widget = new QWidget(ui->modelsNames);
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    auto* label = new QLabel(modelName, widget);
    label->setWordWrap(true);
    label->setObjectName("modelNameLabel");
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setMinimumWidth(0);
    auto* deleteButton = new QPushButton(tr("Delete"), widget);
    deleteButton->setObjectName("deleteButton");
    deleteButton->setProperty("modelName", modelName);
    deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(label);
    layout->addWidget(deleteButton);
    layout->setStretch(0, 1);
    layout->setStretch(1, 0);

    item->setSizeHint(widget->sizeHint());
    ui->modelsNames->setItemWidget(item, widget);

    connect(deleteButton, &QPushButton::clicked, this, &LoadModelWindow::onDeleteButtonClicked);
}

void LoadModelWindow::updateItemsSizeHints() {
    const int viewportWidth = ui->modelsNames->viewport()->width();
    if (viewportWidth <= 0) {
        return;
    }

    for (int i = 0; i < ui->modelsNames->count(); ++i) {
        auto *item = ui->modelsNames->item(i);
        auto *widget = ui->modelsNames->itemWidget(item);
        if (!widget) {
            continue;
        }

        auto *layout = qobject_cast<QHBoxLayout*>(widget->layout());
        auto *label = widget->findChild<QLabel*>("modelNameLabel");
        auto *deleteButton = widget->findChild<QPushButton*>("deleteButton");
        if (!layout || !label || !deleteButton) {
            item->setSizeHint(widget->sizeHint());
            continue;
        }

        const QMargins margins = layout->contentsMargins();
        const int spacing = layout->spacing();
        const int buttonWidth = deleteButton->sizeHint().width();
        const int availableLabelWidth = qMax(
            1,
            viewportWidth - margins.left() - margins.right() - spacing - buttonWidth
        );

        label->setFixedWidth(availableLabelWidth);
        const int labelHeight = label->heightForWidth(availableLabelWidth);
        const int contentHeight = qMax(labelHeight, deleteButton->sizeHint().height());
        const int totalHeight = margins.top() + contentHeight + margins.bottom();

        item->setSizeHint(QSize(viewportWidth, totalHeight));
    }
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
        updateItemsSizeHints();
        updateButtonsState();
        return;
    }
}
