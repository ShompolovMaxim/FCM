#pragma once

#include <QDoubleSpinBox>

class SignificantSpinBox : public QDoubleSpinBox {
    Q_OBJECT

public:
    explicit SignificantSpinBox(QWidget *parent = nullptr);

protected:
    QString textFromValue(double value) const override;
    double valueFromText(const QString &text) const override;
};
