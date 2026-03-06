#ifndef WEIGHT_WINDOW_H
#define WEIGHT_WINDOW_H

#include "model/entities/term.h"
#include "model/entities/weight.h"
#include "model/fuzzy_logic/numeric_fuzzifier.h"

#include <QDialog>

namespace Ui {
class WeightWindow;
}

class WeightWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WeightWindow(const std::map<size_t, Term>& terms, Weight currentWeight, QWidget *parent = nullptr);
    ~WeightWindow();

signals:
    void applied(const Weight& value);

private slots:
    void onApplyClicked();
    void onOkClicked();

private:
    Ui::WeightWindow *ui;
    const std::map<size_t, Term>& terms;
    Weight currentWeight;
    std::shared_ptr<Fuzzifier> fuzzifier = std::make_shared<NumericFuzzifier>();

    void updateCurrentWeight();
};

#endif // WEIGHT_WINDOW_H
