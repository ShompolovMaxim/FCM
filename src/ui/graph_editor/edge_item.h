#pragma once

#include <memory>
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

#include "model/entities/term.h"

class NodeItem;

class EdgeItem : public QGraphicsPathItem
{
public:
    EdgeItem(NodeItem* s, NodeItem* d, QUuid id);

    void updatePosition();

    void setValue(double v);

    void setValue(std::shared_ptr<Term> newTerm);

    void setColor(QColor color);

    QUuid getId() const { return id; }

    NodeItem* src;
    NodeItem* dst;

protected:
    QPainterPath shape() const override;

private:
    std::shared_ptr<Term> term;
    QUuid id;
    std::unique_ptr<IColorValueAdapter> colorValueAdapter;
    QGraphicsPolygonItem* arrowItem = nullptr;
};
