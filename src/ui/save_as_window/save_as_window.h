#pragma once

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class SaveAsWindow;
}

class SaveAsWindow : public QDialog {
    Q_OBJECT

public:
    explicit SaveAsWindow(QStringList modelsNames, QString modelName, QWidget *parent = nullptr);
    ~SaveAsWindow();

    QString savingModelName() const;

private slots:
    void onModelDoubleClicked(QListWidgetItem* item);

private:
    Ui::SaveAsWindow *ui;
};
