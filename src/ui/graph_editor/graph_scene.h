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

    void setConceptColor(size_t id, QColor color, bool highlight);

    void blockConceptCreationColorEdit(bool flag);

public slots:
    void switchMode();
    void conceptCreated(std::shared_ptr<Concept> concept);
    void weightCreated(std::shared_ptr<Weight> weight);
    void conceptUpdated(std::shared_ptr<Concept> concept);
    void weightUpdated(std::shared_ptr<Weight> weight);
    void conceptDeleted(size_t id);
    void weightDeleted(size_t id);


signals:
    void modeChanged(EditMode newMode);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;

private:
    EditMode mode = EditMode::Create;
    int counter = 0;
    int edgesCounter = 0;
    NodeItem* firstNode = nullptr;
    std::shared_ptr<FCM> fcm;
    std::shared_ptr<Fuzzifier> fuzzifier;
    std::shared_ptr<CreationPresenter> presenter;
    std::map<size_t, NodeItem*> nodes;
    std::map<size_t, EdgeItem*> edges;
    bool conceptCreationColorEditBlocked = false;
};
