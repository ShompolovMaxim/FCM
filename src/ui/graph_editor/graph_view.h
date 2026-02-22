#pragma once

#include <QtWidgets>

#include "node_item.h"

class GraphView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphView(QWidget* parent = nullptr);
public slots:
    void resetScale();

signals:
    void scaleChanged(double newScale);

protected:
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPoint lastPanPoint;
    QPointF panAccumulator;
    double panSensitivity;

    bool hasNodes() const
    {
        for (auto* item : scene()->items()) {
            if (dynamic_cast<NodeItem*>(item))
                return true;
        }
        return false;
    }
};
