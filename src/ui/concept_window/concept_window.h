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
    explicit ConceptWindow(const std::map<QUuid, std::shared_ptr<Term>>& terms, std::shared_ptr<Concept> currentConcept, QWidget *parent = nullptr);
    ~ConceptWindow();

    void setPredictedValues();

    void updateTermsList();

signals:
    void applied(const std::shared_ptr<Concept>& value);
    void deleted(QUuid id);

private slots:
    void onApplyClicked();
    void onCancelClicked();
    void closeEvent(QCloseEvent* event) override;
    void onOkClicked();
    void onDelete();

private:
    void updateCurrentConcept();

    void setNumericPredictedValues();
    void setFuzzyPredictedValues();
    void setSensitivity();

    bool creation;
    Ui::ConceptWindow *ui;
    const std::map<QUuid, std::shared_ptr<Term>>& terms;
    std::shared_ptr<Concept> currentConcept;
    std::shared_ptr<Fuzzifier> fuzzifier = std::make_shared<NumericFuzzifier>();
};

#endif // CONCEPT_WINDOW_H
