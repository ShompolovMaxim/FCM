#include "significant_spin_box.h"

#include <QLocale>

SignificantSpinBox::SignificantSpinBox(QWidget *parent) : QDoubleSpinBox(parent) {}

QString SignificantSpinBox::textFromValue(double value) const {
    return QLocale().toString(value, 'g', 7);
}

double SignificantSpinBox::valueFromText(const QString &text) const {
    return QLocale().toDouble(text);
}
