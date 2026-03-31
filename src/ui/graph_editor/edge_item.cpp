#include "edge_item.h"
#include "node_item.h"

#include "color_value_adapter/color_value_adapter.h"

EdgeItem::EdgeItem(NodeItem* s, NodeItem* d, size_t id)
    : src(s), dst(d), colorValueAdapter(std::make_unique<ColorValueAdapter>()), id(id) {
    setZValue(-1);
    setValue(nullptr);

    arrowItem = new QGraphicsPolygonItem(this);
    arrowItem->setPen(Qt::NoPen);
    arrowItem->setBrush(QColor(0, 0, 0));
    arrowItem->setZValue(1);
}

void EdgeItem::setValue(double value) {
    QColor c = colorValueAdapter->getColor(value, -1, 1);

    QPen pen(c, 2);
    pen.setCosmetic(true);
    setPen(pen);
    setBrush(Qt::NoBrush);

    if (arrowItem) {
        arrowItem->setBrush(c);
    }
}

void EdgeItem::setValue(std::shared_ptr<Term> newTerm) {
    term = newTerm;

    auto c = QColor(0, 0, 0);
    if (term) {
        c = colorValueAdapter->getColor(term->value, -1, 1);
    }

    QPen pen(c, 2);
    pen.setCosmetic(true);
    setPen(pen);
    setBrush(Qt::NoBrush);

    if (arrowItem) {
        arrowItem->setBrush(c);
    }
}

void EdgeItem::setColor(QColor color) {
    QPen pen(color, 2);
    pen.setCosmetic(true);
    setPen(pen);
    setBrush(Qt::NoBrush);

    if (arrowItem) {
        arrowItem->setBrush(color);
    }
}

void EdgeItem::updatePosition() {
    QLineF line(src->scenePos(), dst->scenePos());
    if (line.length() < 40) {
        if (arrowItem) arrowItem->setVisible(false);
        return;
    }

    if (arrowItem) arrowItem->setVisible(true);

    QPointF dir = (line.p2() - line.p1()) / line.length();
    QPointF start = src->scenePos() + dir * 25;
    QPointF end   = dst->scenePos() - dir * 25;

    QPointF normal(-dir.y(), dir.x());
    double curvature = 80.0;
    QPointF mid = (start + end) / 2 + normal * curvature;

    QPainterPath path;
    path.moveTo(start);
    path.quadTo(mid, end);
    setPath(path);

    QPointF tangentVec = end - mid;
    QPolygonF arrow{{0,0},{-15,8},{-15,-8}};

    QTransform t;
    t.translate(end.x(), end.y());
    t.rotate(-QLineF(QPointF(0,0), tangentVec).angle());
    arrowItem->setPolygon(t.map(arrow));
}

QPainterPath EdgeItem::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(12);

    return stroker.createStroke(path());
}
