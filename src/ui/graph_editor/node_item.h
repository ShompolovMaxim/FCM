#pragma once

#include <memory>
#include <QtWidgets>

#include "model/color_value_adapter/adapter.h"
#include "model/entities/concept.h"

class EdgeItem;

class NodeItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    NodeItem(std::shared_ptr<Concept> concept);

    void addEdge(EdgeItem* e) { edges.append(e); }
    void removeEdge(EdgeItem* edge) { edges.removeOne(edge); }

    void setValue(std::shared_ptr<Term> newTerm);
    void setColor(QColor color);

    QString getName() const { return nodeName; }
    void setName(QString name);
    void setNameLocation(ConceptNameLocation location);

    QUuid getId() const { return id; }

    std::shared_ptr<Concept> getConcept() { return concept; }

    void highlight(bool flag);

signals:
    void positionChanged(QUuid id);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& val) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;

private:
    void updateLabelPosition();

    QString nodeName;
    std::shared_ptr<Concept> concept;
    QList<EdgeItem*> edges;
    std::shared_ptr<Term> term;
    QUuid id;
    std::unique_ptr<ColorValueAdapter> colorValueAdapter;
    std::vector<double> predictedValues;

    QGraphicsSimpleTextItem* label = nullptr;
};
