#include "color_value_adapter.h"

ColorValueAdapter::ColorValueAdapter(/*const std::shared_ptr<Term>& terms*/) /*: IColorValueAdapter(), terms(terms)*/ {}

QColor ColorValueAdapter::getColor(double value, double min, double max) {
    value = std::clamp(value, min, max);
    double t = (value - min) / (max - min);

    int r = int(255 * (1.0 - t));
    int g = int(255 * t);
    int b = 0;

    return QColor(r, g, b);
}
