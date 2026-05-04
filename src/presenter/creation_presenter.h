#ifndef CREATION_PRESENTER_H
#define CREATION_PRESENTER_H

#include "element_window_mode.h"
#include "scene_presenter.h"

#include "model/entities/fcm.h"

#include "ui/concept_window/concept_window.h"
#include "ui/weight_window/weight_window.h"

#include <QObject>
#include <QWidget>

class CreationPresenter : public ScenePresenter {
    Q_OBJECT
public:
    CreationPresenter(std::shared_ptr<FCM> fcm, QWidget* elementWindowParent, QObject* parent = nullptr);

    void createConcept(const QPointF pos) override;
    void createWeight(QUuid nodeId) override;
    void createWeight(QUuid fromNodeId, QUuid toNodeId) override;
    void updateConcept(QUuid id, ElementWindowMode mode) override;
    void updateWeight(QUuid id, ElementWindowMode mode) override;

    void setConceptPredictedValues(QUuid id);
    void setWeightPredictedValues(QUuid id);

    void updateTerm(QUuid id);
    void deleteTerm(QUuid id);

    void retranslateElementsWindows();

    void emitAutosave() override;

    void closeWindows();

private slots:
    void deleteConcept(QUuid id);
    void deleteWeight(QUuid id);

private:
    void updateTermsLists();

    std::shared_ptr<FCM> fcm;
    std::map<QUuid, ConceptWindow*> conceptWindows;
    std::map<QUuid, WeightWindow*> weightWindows;
    std::optional<QUuid> firstNodeId;
    QWidget* elementWindowParent;
};

#endif // CREATION_PRESENTER_H
