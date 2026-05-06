#include "edge_item.h"
#include "node_item.h"

#include "color_value_adapter/color_value_adapter.h"

NodeItem::NodeItem(std::shared_ptr<Concept> concept)
    : nodeName(concept->name), id(concept->id), concept(concept), colorValueAdapter(std::make_unique<ColorValueAdapter>()), QObject() {
    setRect(-25, -25, 50, 50);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);

    label = new QGraphicsSimpleTextItem(nodeName, this);
    updateLabelPosition();

    setValue(nullptr);
}

void NodeItem::setName(QString name) {
    nodeName = name;
    concept->name = name;

    if (label) {
        label->setText(name);
        updateLabelPosition();
    }

    update();
}

void NodeItem::setNameLocation(ConceptNameLocation location) {
    concept->nameLocation = location;
    updateLabelPosition();
    update();
}

void NodeItem::setValue(std::shared_ptr<Term> newTerm) {
    term = newTerm;
    if (term) {
        setBrush(term->color);
    } else {
        setBrush(QColor(255, 255, 255));
    }
}

void NodeItem::setColor(QColor color) {
    setBrush(color);
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& val) {
    if (change == ItemPositionHasChanged) {
        concept->pos = scenePos();
        emit positionChanged(concept->id);

        for (EdgeItem* e : edges) {
            e->updatePosition();
        }

        updateLabelPosition();
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

void NodeItem::updateLabelPosition() {
    if (!label) {
        return;
    }

    const QRectF ellipseRect = rect();
    const QRectF labelRect = label->boundingRect();
    const qreal xPadding = 3.0;
    const qreal yPadding = 3.0;

    qreal x = ellipseRect.center().x() - labelRect.width() / 2;
    qreal y = ellipseRect.top() - labelRect.height() - yPadding;

    switch (concept->nameLocation) {
    case ConceptNameLocation::Up:
        break;
    case ConceptNameLocation::UpLeft:
        x = ellipseRect.left() - labelRect.width() - xPadding * 0.7;
        break;
    case ConceptNameLocation::UpRight:
        x = ellipseRect.right() + xPadding * 0.7;
        break;
    case ConceptNameLocation::Center:
        y = ellipseRect.center().y() - labelRect.height() / 2;
        break;
    case ConceptNameLocation::CenterLeft:
        x = ellipseRect.left() - labelRect.width() - xPadding;
        y = ellipseRect.center().y() - labelRect.height() / 2;
        break;
    case ConceptNameLocation::CenterRight:
        x = ellipseRect.right() + xPadding;
        y = ellipseRect.center().y() - labelRect.height() / 2;
        break;
    case ConceptNameLocation::Bottom:
        y = ellipseRect.bottom() + yPadding;
        break;
    case ConceptNameLocation::BottomLeft:
        x = ellipseRect.left() - labelRect.width() - xPadding * 0.7;
        y = ellipseRect.bottom() + yPadding;
        break;
    case ConceptNameLocation::BottomRight:
        x = ellipseRect.right() + xPadding * 0.7;
        y = ellipseRect.bottom() + yPadding * 0.7;
        break;
    }

    label->setPos(x, y);
}
