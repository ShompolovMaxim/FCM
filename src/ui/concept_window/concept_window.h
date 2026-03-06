#ifndef CONCEPT_WINDOW_H
#define CONCEPT_WINDOW_H

#include "model/entities/term.h"
#include "model/entities/concept.h"
#include "model/fuzzy_logic/numeric_fuzzifier.h"

#include <QDialog>

namespace Ui {
class ConceptWindow;
}

class ConceptWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConceptWindow(const std::map<size_t, Term>& terms, Concept currentConcept, std::vector<double> predictedValues, QWidget *parent = nullptr);
    ~ConceptWindow();

    void setPredictedValues(std::vector<double> predictedValues);

signals:
    void applied(const Concept& value);

private slots:
    void onApplyClicked();
    void onOkClicked();

private:
    Ui::ConceptWindow *ui;
    const std::map<size_t, Term>& terms;
    Concept currentConcept;
    std::shared_ptr<Fuzzifier> fuzzifier = std::make_shared<NumericFuzzifier>();

    void updateCurrentConcept();
};

#endif // CONCEPT_WINDOW_H
