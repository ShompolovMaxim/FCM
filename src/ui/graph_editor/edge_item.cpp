#include "edge_item.h"
#include "node_item.h"

#include "model/color_value_adapter/default_adapter.h"

EdgeItem::EdgeItem(NodeItem* s, NodeItem* d, QUuid id)
    : src(s), dst(d), colorValueAdapter(std::make_unique<DefaultColorValueAdapter>()), id(id) {
    setZValue(-1);
    setValue(nullptr);

    arrowItem = new QGraphicsPolygonItem(this);
    arrowItem->setPen(Qt::NoPen);
    arrowItem->setBrush(QColor(0, 0, 0));
    arrowItem->setZValue(1);
}

void EdgeItem::setValue(std::shared_ptr<Term> newTerm) {
    term = newTerm;

    auto c = QColor(0, 0, 0);
    if (term) {
        c = term->color;
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

EdgeItem::Geometry EdgeItem::buildGeometry(const QPointF& sourcePos, const QPointF& targetPos, qreal sourceInset, qreal targetInset) {
    Geometry geometry;

    QLineF line(sourcePos, targetPos);
    if (line.length() < 40) {
        return geometry;
    }

    geometry.visible = true;

    QPointF dir = (line.p2() - line.p1()) / line.length();
    QPointF start = sourcePos + dir * sourceInset;
    QPointF end = targetPos - dir * targetInset;

    QPointF normal(-dir.y(), dir.x());
    constexpr double curvature = 80.0;
    QPointF mid = (start + end) / 2 + normal * curvature;

    geometry.path.moveTo(start);
    geometry.path.quadTo(mid, end);

    QPointF tangentVec = end - mid;
    QPolygonF arrow{{0, 0}, {-15, 8}, {-15, -8}};

    QTransform transform;
    transform.translate(end.x(), end.y());
    transform.rotate(-QLineF(QPointF(0, 0), tangentVec).angle());
    geometry.arrow = transform.map(arrow);

    return geometry;
}

void EdgeItem::updatePosition() {
    const auto geometry = buildGeometry(src->scenePos(), dst->scenePos(), 25, 25);
    if (!geometry.visible) {
        setPath(QPainterPath());
        if (arrowItem) {
            arrowItem->setVisible(false);
        }
        return;
    }

    if (arrowItem) {
        arrowItem->setVisible(true);
    }
    setPath(geometry.path);
    arrowItem->setPolygon(geometry.arrow);
}

QPainterPath EdgeItem::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(12);

    return stroker.createStroke(path());
}
