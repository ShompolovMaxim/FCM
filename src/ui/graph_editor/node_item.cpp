#include "edge_item.h"
#include "node_item.h"

#include "color_value_adapter/color_value_adapter.h"

NodeItem::NodeItem(const QString& name,  size_t id, std::shared_ptr<Concept> concept)
    : nodeName(concept->name), id(concept->id), concept(concept), colorValueAdapter(std::make_unique<ColorValueAdapter>())
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

void NodeItem::setConcept(std::shared_ptr<Concept> newConcept) {
    setValue(newConcept->value);
    concept = newConcept;

    update();
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& val)
{
    if (change == ItemPositionHasChanged) {
        concept->pos = scenePos();
        for (EdgeItem* e : edges) {
            e->updatePosition();
        }
    }
    return QGraphicsItem::itemChange(change, val);
}

void NodeItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w)
{
    QGraphicsEllipseItem::paint(p, o, w);
    p->setPen(Qt::black);
    p->drawText(rect(), Qt::AlignCenter, concept->name);
}

void NodeItem::setPredictedValues(std::vector<double> values) {
    predictedValues = values;
    if (!window) {
        return;
    }
    window->setPredictedValues(values);
}
