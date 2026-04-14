#include "save_current_state_window.h"
#include "ui_save_current_state_window.h"

SaveCurrentStateWindow::SaveCurrentStateWindow(QWidget *parent) : QDialog(parent), ui(new Ui::SaveCurrentStateWindow) {
    ui->setupUi(this);
}

SaveCurrentStateWindow::~SaveCurrentStateWindow() {
    delete ui;
}

bool SaveCurrentStateWindow::saveCurrentState() {
    return ui->saveCurrentState->isChecked();
}
