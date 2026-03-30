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
    explicit WeightWindow(const std::map<size_t, std::shared_ptr<Term>>& terms, std::shared_ptr<Weight> currentWeight, QWidget *parent = nullptr);
    ~WeightWindow();

    void setPredictedValues();

    void updateTermsList();

signals:
    void applied(const std::shared_ptr<Weight>& value);
    void deleted(size_t id);

private slots:
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();
    void closeEvent(QCloseEvent* event) override;
    void onDelete();

private:
    void updateCurrentWeight();

    void setNumericPredictedValues();
    void setFuzzyPredictedValues();

    Ui::WeightWindow *ui;
    const std::map<size_t, std::shared_ptr<Term>>& terms;
    std::shared_ptr<Weight> currentWeight;
    std::shared_ptr<Fuzzifier> fuzzifier = std::make_shared<NumericFuzzifier>();
    bool creation;
};

#endif // WEIGHT_WINDOW_H
