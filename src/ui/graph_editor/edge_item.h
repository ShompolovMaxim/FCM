#pragma once

#include <memory>
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

class NodeItem;

class EdgeItem : public QGraphicsPathItem
{
public:
    EdgeItem(NodeItem* s, NodeItem* d, size_t id);

    void updatePosition();

    void setValue(double v);

    size_t getId() const { return id; }

    double getValue() const { return value; }

    NodeItem* src;
    NodeItem* dst;

protected:
    QPainterPath shape() const override;

private:
    double value = 0.0;
    size_t id;
    std::unique_ptr<IColorValueAdapter> colorValueAdapter;
    QGraphicsPolygonItem* arrowItem = nullptr;
};
