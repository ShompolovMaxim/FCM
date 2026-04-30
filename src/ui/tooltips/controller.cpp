#include "controller.h"

ToolTipController::ToolTipController(QObject *parent) : QObject(parent), enabled(true) {}

void ToolTipController::setEnabled(bool on) {
    enabled = on;
}

bool ToolTipController::isEnabled() const {
    return enabled;
}

bool ToolTipController::eventFilter(QObject *obj, QEvent *event) {
    if (!enabled && event->type() == QEvent::ToolTip) {
        return true;
    }
    return QObject::eventFilter(obj, event);
}
