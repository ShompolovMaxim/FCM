#pragma once
#include <QtWidgets>

class ColorValueAdapter {
public:
    virtual QColor getColor(double value, double min = -1, double max = 1, bool conceptColor = true, bool fuzzyValues = false) = 0;
    virtual ~ColorValueAdapter() = default;
};
