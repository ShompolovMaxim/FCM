#pragma once
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

class EdgeItem;

class NodeItem : public QGraphicsEllipseItem
{
public:
    NodeItem(const QString& name, size_t id);

    void addEdge(EdgeItem* e) { edges.append(e); }
    QList<EdgeItem*> getEdges() const { return edges; }

    void setValue(double v);

    QString getName() const { return nodeName; }

    double getValue() const { return value; }

    size_t getId() const { return id; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& val) override;

    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;

private:
    QString nodeName;
    QList<EdgeItem*> edges;
    double value = 0.0;
    size_t id;
    std::unique_ptr<IColorValueAdapter> colorValueAdapter;
};
