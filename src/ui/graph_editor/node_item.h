#pragma once

#include <memory>
#include <QtWidgets>

#include "color_value_adapter/IColorValueAdapter.h"

#include "model/entities/concept.h"

#include "ui/concept_window/concept_window.h"

class EdgeItem;

class NodeItem : public QGraphicsEllipseItem
{
public:
    NodeItem(const QString& name, size_t id, std::shared_ptr<Concept> concept);

    void addEdge(EdgeItem* e) { edges.append(e); }
    QList<EdgeItem*> getEdges() const { return edges; }

    void setValue(double v);

    QString getName() const { return nodeName; }

    double getValue() const { return value; }

    size_t getId() const { return id; }

    std::shared_ptr<Concept> getConcept() { return concept; };
    void setConcept(std::shared_ptr<Concept> newConcept);

    void setPredictedValues(std::vector<double> values);
    std::vector<double> getPredictedValues() { return predictedValues; };

    ConceptWindow* getWindow() { return window; };
    void setWindow(ConceptWindow* newWindow) { window = newWindow; };

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& val) override;

    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w) override;

private:
    QString nodeName;
    std::shared_ptr<Concept> concept;
    QList<EdgeItem*> edges;
    double value = 0.0;
    size_t id;
    std::unique_ptr<IColorValueAdapter> colorValueAdapter;
    std::vector<double> predictedValues;
    QPointer<ConceptWindow> window = nullptr;
};
