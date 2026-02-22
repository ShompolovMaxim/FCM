#include "edge_item.h"
#include "node_item.h"

#include "color_value_adapter/color_value_adapter.h"

NodeItem::NodeItem(const QString& name,  size_t id)
    : nodeName(name), id(id), colorValueAdapter(std::make_unique<ColorValueAdapter>())
{
    setRect(-25, -25, 50, 50);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setValue(0.0);
}

void NodeItem::setValue(double v)
{
    value = v;
    setBrush(colorValueAdapter->getColor(value, 0, 1));
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& val)
{
    if (change == ItemPositionHasChanged) {
        for (EdgeItem* e : edges)
            e->updatePosition();
    }
    return QGraphicsItem::itemChange(change, val);
}

void NodeItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w)
{
    QGraphicsEllipseItem::paint(p, o, w);
    p->setPen(Qt::black);
    p->drawText(rect(), Qt::AlignCenter, nodeName);
}
