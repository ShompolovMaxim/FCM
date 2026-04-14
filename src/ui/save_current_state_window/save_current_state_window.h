#ifndef SAVE_CURRENT_STATE_WINDOWTEWINDOW_H
#define SAVE_CURRENT_STATE_WINDOWTEWINDOW_H

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

#endif // SAVE_CURRENT_STATE_WINDOWTEWINDOW_H
