#include "graph_view.h"

#include <QSettings>

GraphView::GraphView(QWidget* parent) : QGraphicsView(parent) {
    const QSettings settings("app.ini", QSettings::IniFormat);

    setRenderHint(QPainter::Antialiasing);
    setDragMode(RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setViewportUpdateMode(FullViewportUpdate);
    setMouseTracking(true);
    panSensitivity = settings.value("graph/panSensitivity", 2).toDouble();
    zoomFactor = settings.value("graph/zoomFactor", 1.15).toDouble();
}

void GraphView::resetScale() {
    double currentScale = transform().m11();
    scale(1.0 / currentScale, 1.0 / currentScale);
    scaleChanged(1.0);
}

void GraphView::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0) {
        scale(zoomFactor, zoomFactor);
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);
    }
    scaleChanged(transform().m11());
}

void GraphView::mousePressEvent(QMouseEvent* event)
{
    if ((event->button() == Qt::MiddleButton ||
         (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) &&
        hasNodes())
    {
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
    if (!lastPanPoint.isNull()) {
        QPointF delta = lastPanPoint - event->pos();
        lastPanPoint = event->pos();
        panAccumulator += QPointF(delta) * panSensitivity;

        int dx = std::round(panAccumulator.x());
        int dy = std::round(panAccumulator.y());

        if (dx != 0) {
            horizontalScrollBar()->setValue(
                horizontalScrollBar()->value() + dx);
            panAccumulator.rx() -= dx;
        }

        if (dy != 0) {
            verticalScrollBar()->setValue(
                verticalScrollBar()->value() + dy);
            panAccumulator.ry() -= dy;
        }
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void GraphView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!lastPanPoint.isNull() &&
        (event->button() == Qt::MiddleButton ||
         (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier))) {
        lastPanPoint = QPoint();
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}
