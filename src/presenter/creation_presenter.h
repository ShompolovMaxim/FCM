#ifndef CREATION_PRESENTER_H
#define CREATION_PRESENTER_H

#include "element_window_mode.h"

#include "model/entities/fcm.h"

#include "ui/concept_window/concept_window.h"
#include "ui/weight_window/weight_window.h"

#include <QObject>
#include <QWidget>

class CreationPresenter : public QObject {
    Q_OBJECT
public:
    CreationPresenter(std::shared_ptr<FCM> fcm, QWidget* elementWindowParent, QObject* parent = nullptr);

    void createConcept(const QPointF pos);
    void createWeight(QUuid nodeId);
    void createWeight(QUuid fromNodeId, QUuid toNodeId);
    void updateConcept(QUuid id, ElementWindowMode mode);
    void updateWeight(QUuid id, ElementWindowMode mode);

    void setConceptPredictedValues(QUuid id);
    void setWeightPredictedValues(QUuid id);

    void updateTerm(QUuid id);
    void deleteTerm(QUuid id);

    void retranslateElementsWindows();

    void emitAutosave();

    void closeWindows();

private slots:
    void deleteConcept(QUuid id);
    void deleteWeight(QUuid id);

signals:
    void conceptCreated(std::shared_ptr<Concept> concept);
    void weightCreated(std::shared_ptr<Weight> weight);
    void conceptUpdated(std::shared_ptr<Concept> concept);
    void weightUpdated(std::shared_ptr<Weight> weight);
    void conceptDeleted(QUuid id);
    void weightDeleted(QUuid id);

    void autosave();

private:
    void updateTermsLists();

    std::shared_ptr<FCM> fcm;
    std::map<QUuid, ConceptWindow*> conceptWindows;
    std::map<QUuid, WeightWindow*> weightWindows;
    std::optional<QUuid> firstNodeId;
    QWidget* elementWindowParent;
};

#endif // CREATION_PRESENTER_H
