#pragma once

#include <memory>
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

#include "model/entities/term.h"

class NodeItem;

class EdgeItem : public QGraphicsPathItem
{
public:
    EdgeItem(NodeItem* s, NodeItem* d, size_t id);

    void updatePosition();

    void setValue(double v);

    void setValue(std::shared_ptr<Term> newTerm);

    size_t getId() const { return id; }

    //double getValue() const { return value; }

    NodeItem* src;
    NodeItem* dst;

protected:
    QPainterPath shape() const override;

private:
    std::shared_ptr<Term> term;
    size_t id;
    std::unique_ptr<IColorValueAdapter> colorValueAdapter;
    QGraphicsPolygonItem* arrowItem = nullptr;
};
