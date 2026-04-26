#include "edge_item.h"
#include "node_item.h"
#include "graph_scene.h"

#include "color_value_adapter/color_value_adapter.h"

NodeItem::NodeItem(std::shared_ptr<Concept> concept)
    : nodeName(concept->name), id(concept->id), concept(concept), colorValueAdapter(std::make_unique<ColorValueAdapter>()), QObject() {
    setRect(-25, -25, 50, 50);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);

    label = new QGraphicsSimpleTextItem(nodeName, this);
    //label->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    QRectF r = rect();
    QRectF br = label->boundingRect();
    label->setPos(
        r.center().x() - br.width() / 2,
        r.top() - br.height() - 5
        );

    setValue(nullptr);
}

void NodeItem::setName(QString name) {
    nodeName = name;
    concept->name = name;

    if (label) {
        label->setText(name);

        QRectF r = rect();
        QRectF br = label->boundingRect();
        label->setPos(r.center().x() - br.width() / 2, r.top() - br.height() - 5);
    }

    update();
}

void NodeItem::setValue(std::shared_ptr<Term> newTerm) {
    term = newTerm;
    if (term) {
        setBrush(colorValueAdapter->getColor(term->value, 0, 1));
    } else {
        setBrush(QColor(255, 255, 255));
    }
}

void NodeItem::setValue(double value) {
    setBrush(colorValueAdapter->getColor(value, 0, 1));
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& val) {
    if (change == ItemPositionHasChanged) {
        concept->pos = scenePos();
        emit positionChanged();

        for (EdgeItem* e : edges) {
            e->updatePosition();
        }

        if (label) {
            QRectF r = rect();
            QRectF br = label->boundingRect();
            label->setPos(r.center().x() - br.width() / 2, r.top() - br.height() - 5);
        }
    }
    return QGraphicsItem::itemChange(change, val);
}

void NodeItem::paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) {
    QGraphicsEllipseItem::paint(p, o, w);
}

void NodeItem::highlight(bool flag) {
    QPen pen(flag ? Qt::blue : Qt::black);
    setPen(pen);
    update();
}
