#pragma once

#include <memory>
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

#include "model/entities/concept.h"

class EdgeItem;

class NodeItem : public QGraphicsEllipseItem
{
public:
    NodeItem(std::shared_ptr<Concept> concept);

    void addEdge(EdgeItem* e) { edges.append(e); }
    void removeEdge(EdgeItem* edge) { edges.removeOne(edge); }

    void setValue(std::shared_ptr<Term> newTerm);
    void setValue(double value);

    QString getName() const { return nodeName; }
    void setName(QString name);

    //std::shared_ptr<Term> getValue() const { return term; }

    size_t getId() const { return id; }

    std::shared_ptr<Concept> getConcept() { return concept; };

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& val) override;

    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;

private:
    QString nodeName;
    std::shared_ptr<Concept> concept;
    QList<EdgeItem*> edges;
    std::shared_ptr<Term> term;
    size_t id;
    std::unique_ptr<IColorValueAdapter> colorValueAdapter;
    std::vector<double> predictedValues;
};
