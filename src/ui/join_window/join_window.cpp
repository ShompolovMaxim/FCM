#include "join_window.h"
#include "ui_join_window.h"

#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>

JoinWindow::JoinWindow(const QList<QString>& unsavedModelsNames, const QList<QString>& savedModelsNames, const QList<QString>& templatesNames, QWidget *parent)
    : unsavedModelsNames(unsavedModelsNames), savedModelsNames(savedModelsNames), templatesNames(templatesNames), QDialog(parent), ui(new Ui::JoinWindow) {
    ui->setupUi(this);

    ui->joinMode->setItemData(0, "numeric", Qt::UserRole);
    ui->joinMode->setItemData(1, "fuzzy", Qt::UserRole);
    ui->joinMode->setItemData(2, "gibrid", Qt::UserRole);

    backButton = new QPushButton(tr("Back"));
    ui->buttonBox->addButton(backButton, QDialogButtonBox::ActionRole);
    okButton = new QPushButton("OK");
    ui->buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    nextButton = new QPushButton(tr("Next"));
    ui->buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);

    connect(ui->filterLine, &QLineEdit::textChanged, this, &JoinWindow::filterTree);
    connect(backButton, &QPushButton::clicked, this, &JoinWindow::back);
    connect(nextButton, &QPushButton::clicked, this, &JoinWindow::next);
    connect(okButton, &QPushButton::clicked, this, &JoinWindow::onOkClicked);

    back();
}

JoinWindow::~JoinWindow() {
    delete ui;
}

void JoinWindow::populateTree(const QList<QList<QString>>& data, const QList<QString>& headers) {
    ui->modelList->clear();

    QButtonGroup* group = nullptr;
    if (selectionMode == SelectionMode::Single) {
        group = new QButtonGroup(this);
        group->setExclusive(true);
    }

    int count = qMin(data.size(), headers.size());

    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem* parent = new QTreeWidgetItem(ui->modelList);
        parent->setText(0, headers[i]);

        JoinGroupType groupType = JoinGroupType::Unsaved;
        if (headers[i] == tr("Saved models")) {
            groupType = JoinGroupType::Saved;
        }

        parent->setData(0, Qt::UserRole, static_cast<int>(groupType));

        for (const QString& text : data[i]) {
            QTreeWidgetItem* item = new QTreeWidgetItem(parent);

            if (selectionMode == SelectionMode::Single) {
                QRadioButton* radio = new QRadioButton(text);
                ui->modelList->setItemWidget(item, 0, radio);
                group->addButton(radio);
            } else {
                item->setText(0, text);

                if (selectedNames.contains(groupType) &&
                    selectedNames[groupType].contains(text)) {
                    item->setCheckState(0, Qt::Checked);
                } else {
                    item->setCheckState(0, Qt::Unchecked);
                }
            }
        }
    }

    ui->modelList->expandAll();
}

void JoinWindow::filterTree(const QString &text) {
    for (int i = 0; i < ui->modelList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* parent = ui->modelList->topLevelItem(i);
        bool parentVisible = false;

        for (int j = 0; j < parent->childCount(); ++j) {
            QTreeWidgetItem* child = parent->child(j);
            bool match = child->text(0).contains(text, Qt::CaseInsensitive);
            child->setHidden(!match);
            if (match) {
                parentVisible = true;
            }
        }

        parent->setHidden(!parentVisible);
    }
}

QString JoinWindow::getSelectedName() const {
    for (int i = 0; i < ui->modelList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* group = ui->modelList->topLevelItem(i);

        for (int j = 0; j < group->childCount(); ++j) {
            QTreeWidgetItem* item = group->child(j);

            if (auto radio = qobject_cast<QRadioButton*>(
                    ui->modelList->itemWidget(item, 0))) {

                if (radio->isChecked()) {
                    return radio->text();
                }
            }
        }
    }

    return {};
}

QMap<JoinGroupType, QList<QString>> JoinWindow::getSelectedNames() const {
    QMap<JoinGroupType, QList<QString>> result;

    for (int i = 0; i < ui->modelList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* group = ui->modelList->topLevelItem(i);

        JoinGroupType type = static_cast<JoinGroupType>(
            group->data(0, Qt::UserRole).toInt()
            );

        QList<QString> groupList;

        for (int j = 0; j < group->childCount(); ++j) {
            QTreeWidgetItem* item = group->child(j);

            if (item->checkState(0) == Qt::Checked) {
                groupList.append(item->text(0));
            }
        }

        if (!groupList.isEmpty()) {
            result[type] = groupList;
        }
    }

    return result;
}

void JoinWindow::next() {
    backButton->show();
    okButton->show();
    nextButton->hide();
    ui->modelListLabel->setText(tr("Choose model which terms will be used:"));
    selectionMode = SelectionMode::Single;
    selectedNames = getSelectedNames();
    populateTree({selectedNames[JoinGroupType::Unsaved], selectedNames[JoinGroupType::Saved], templatesNames}, {tr("Unsaved models"), tr("Saved models"), tr("Templates")});
    filterTree(ui->filterLine->text());
}

void JoinWindow::back() {
    okButton->hide();
    backButton->hide();
    nextButton->show();
    ui->modelListLabel->setText(tr("Choose models to join:"));
    selectionMode = SelectionMode::Multiple;
    populateTree({unsavedModelsNames, savedModelsNames}, {tr("Unsaved models"), tr("Saved models")});
    filterTree(ui->filterLine->text());
}

void JoinWindow::onOkClicked() {
    termsModel = getSelectedName();

    if (termsModel.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a terms model before proceeding."));
        return;
    }

    accept();
}

const QMap<JoinGroupType, QList<QString>>& JoinWindow::getModelsToJoin() const {
    return selectedNames;
}

QString JoinWindow::getTermsModel() const {
    return termsModel;
}

QString JoinWindow::getResultName() const {
    return ui->resultName->text();
}

JoinMode JoinWindow::getJoinMode() const {
    return joinModeFromString(ui->joinMode->currentData(Qt::UserRole).toString());
}
