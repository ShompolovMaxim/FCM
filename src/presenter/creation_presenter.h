#ifndef CREATION_PRESENTER_H
#define CREATION_PRESENTER_H

#include "model/entities/fcm.h"

#include "ui/concept_window/concept_window.h"
#include "ui/weight_window/weight_window.h"

#include <QObject>

class CreationPresenter : public QObject {
    Q_OBJECT
public:
    CreationPresenter(std::shared_ptr<FCM> fcm, QObject* parent = nullptr);

    void createConcept(const QPointF pos);
    void createWeight(size_t nodeId);
    void createWeight(size_t fromNodeId, size_t toNodeId);
    void updateConcept(size_t id);
    void updateWeight(size_t id);

    void setConceptPredictedValues(size_t id);
    void setWeightPredictedValues(size_t id);

private slots:
    void deleteConcept(size_t id);
    void deleteWeight(size_t id);

signals:
    void conceptCreated(std::shared_ptr<Concept> concept);
    void weightCreated(std::shared_ptr<Weight> weight);
    void conceptUpdated(std::shared_ptr<Concept> concept);
    void weightUpdated(std::shared_ptr<Weight> weight);
    void conceptDeleted(size_t id);
    void weightDeleted(size_t id);

private:
    std::shared_ptr<FCM> fcm;
    std::map<size_t, ConceptWindow*> conceptWindows;
    std::map<size_t, WeightWindow*> weightWindows;
    std::optional<size_t> firstNodeId;
};

#endif // CREATION_PRESENTER_H
