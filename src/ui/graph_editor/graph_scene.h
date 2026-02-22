#pragma once

#include <QtWidgets>

#include "edge_item.h"
#include "edit_mode.h"
#include "node_item.h"

#include "model/entities/fcm.h"
#include "model/fuzzy_logic/fuzzifier.h"

#include <memory>

class GraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GraphScene(std::shared_ptr<FCM> fcm);

    void setMode(EditMode m) { mode = m; }

    GraphScene* copy() const;

public slots:
    void switchMode();

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
};
