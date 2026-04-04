#pragma once

#include <QtWidgets>

#include "edge_item.h"
#include "edit_mode.h"
#include "node_item.h"

#include "model/entities/fcm.h"
#include "model/fuzzy_logic/fuzzifier.h"

#include "presenter/creation_presenter.h"

#include <memory>

class GraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GraphScene(std::shared_ptr<FCM> fcm, std::shared_ptr<CreationPresenter> presenter);

    void setMode(EditMode m) { mode = m; }

    void setFCM(std::shared_ptr<FCM> newFCM) { fcm = newFCM; }

    std::shared_ptr<FCM> getFCM() { return fcm; }

    GraphScene* copy() const;

    void setConceptColor(QUuid id, QColor color, bool highlight);
    void setWeightColor(QUuid id, QColor color);

    void blockConceptCreationColorEdit(bool flag);

public slots:
    void switchMode();
    void conceptCreated(std::shared_ptr<Concept> concept);
    void weightCreated(std::shared_ptr<Weight> weight);
    void conceptUpdated(std::shared_ptr<Concept> concept);
    void weightUpdated(std::shared_ptr<Weight> weight);
    void conceptDeleted(QUuid id);
    void weightDeleted(QUuid id);

signals:
    void modeChanged(EditMode newMode);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;

private:
    EditMode mode = EditMode::Create;
    NodeItem* firstNode = nullptr;
    std::shared_ptr<FCM> fcm;
    std::shared_ptr<Fuzzifier> fuzzifier;
    std::shared_ptr<CreationPresenter> presenter;
    std::map<QUuid, NodeItem*> nodes;
    std::map<QUuid, EdgeItem*> edges;
    bool conceptCreationColorEditBlocked = false;
};
