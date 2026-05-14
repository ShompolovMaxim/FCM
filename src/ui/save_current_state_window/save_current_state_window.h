#pragma once

#include <QDialog>

namespace Ui {
class SaveCurrentStateWindow;
}

class SaveCurrentStateWindow : public QDialog {
    Q_OBJECT

public:
    explicit SaveCurrentStateWindow(QWidget *parent = nullptr);
    ~SaveCurrentStateWindow();

    bool saveCurrentState();

private:
    Ui::SaveCurrentStateWindow *ui;
};

