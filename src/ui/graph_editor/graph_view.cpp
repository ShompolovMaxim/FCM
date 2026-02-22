#include "graph_scene.h"
#include "graph_view.h"

GraphView::GraphView(QWidget* parent) : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setViewportUpdateMode(FullViewportUpdate);
    panSensitivity = 2;
}

void GraphView::resetScale() {
    double currentScale = transform().m11();
    scale(1.0 / currentScale, 1.0 / currentScale);
    scaleChanged(1.0);
}

void GraphView::wheelEvent(QWheelEvent* event)
{
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
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
        QPointF delta = mapToScene(lastPanPoint) - mapToScene(event->pos());
        delta *= panSensitivity;
        lastPanPoint = event->pos();
        //translate(delta.x(), delta.y());
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
