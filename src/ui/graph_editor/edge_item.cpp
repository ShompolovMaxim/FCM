#include "edge_item.h"
#include "node_item.h"

#include "color_value_adapter/color_value_adapter.h"

EdgeItem::EdgeItem(NodeItem* s, NodeItem* d, size_t id)
    : src(s), dst(d), colorValueAdapter(std::make_unique<ColorValueAdapter>()), id(id)
{
    setZValue(-1);
    setValue(0.0);
    //setFlag(ItemIsSelectable);
}

void EdgeItem::setValue(double v)
{
    value = v;
    QColor c = colorValueAdapter->getColor(value, -1, 1);
    setPen(QPen(c, 2));
    setBrush(c);
}

void EdgeItem::updatePosition()
{
    QLineF c(src->scenePos(), dst->scenePos());
    if (c.length() < 40) return;

    QPointF dir = (c.p2() - c.p1()) / c.length();
    QPointF start = src->scenePos() + dir * 25;
    QPointF end   = dst->scenePos() - dir * 25;

    QPainterPath path;
    path.moveTo(start);
    path.lineTo(end);

    QPolygonF arrow{{0,0},{-15,8},{-15,-8}};
    QTransform t;
    t.translate(end.x(), end.y());
    t.rotate(-QLineF(start, end).angle());

    QPolygonF arrowT = t.map(arrow);
    path.moveTo(arrowT[0]);
    path.addPolygon(arrowT);
    path.closeSubpath();

    setPath(path);
}
