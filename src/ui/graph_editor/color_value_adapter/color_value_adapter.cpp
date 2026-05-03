#include "color_value_adapter.h"

ColorValueAdapter::ColorValueAdapter() {}

QColor ColorValueAdapter::getColor(double value, double min, double max, bool conceptColor, bool fuzzyValues) {
    double t = std::clamp((value - min) / (max - min), 0.0, 1.0);
    double hue = t * 120.0;
    double brightness = 0.6 + 0.4 * t;
    return QColor::fromHsvF(hue / 360.0, 1.0, brightness);
}
