#pragma once

#include <memory>
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

#include "model/entities/term.h"

class NodeItem;

class EdgeItem : public QGraphicsPathItem
{
public:
    struct Geometry {
        QPainterPath path;
        QPolygonF arrow;
        bool visible = false;
    };

    EdgeItem(NodeItem* s, NodeItem* d, QUuid id);

    static Geometry buildGeometry(const QPointF& sourcePos, const QPointF& targetPos, qreal sourceInset, qreal targetInset);

    void updatePosition();

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
