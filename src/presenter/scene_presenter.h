#pragma once

#include "element_window_mode.h"
#include "model/entities/concept.h"
#include "model/entities/weight.h"

#include <QObject>
#include <optional>
#include <QPointF>

class ScenePresenter : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~ScenePresenter() = default;

    virtual void createConcept(const QPointF pos) = 0;
    virtual void createWeight(QUuid nodeId) = 0;
    virtual void createWeight(QUuid fromNodeId, QUuid toNodeId) = 0;
    virtual void updateConceptPosition(QUuid id, const QPointF& pos) = 0;
    virtual void updateConcept(QUuid id, ElementWindowMode mode) = 0;
    virtual void updateWeight(QUuid id, ElementWindowMode mode) = 0;
    virtual void emitAutosave() = 0;
    virtual bool hasPendingWeightStart() const { return false; }
    virtual std::optional<QUuid> pendingWeightStartId() const { return std::nullopt; }
    virtual void cancelPendingWeightCreation() {}

signals:
    void conceptCreated(std::shared_ptr<Concept> concept);
    void weightCreated(std::shared_ptr<Weight> weight);
    void conceptUpdated(std::shared_ptr<Concept> concept);
    void weightUpdated(std::shared_ptr<Weight> weight);
    void conceptDeleted(QUuid id);
    void weightDeleted(QUuid id);
    void autosave();
};
