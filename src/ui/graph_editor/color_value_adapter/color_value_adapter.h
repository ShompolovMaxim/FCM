#pragma once
#include "IColorValueAdapter.h"

class ColorValueAdapter : public IColorValueAdapter {
public:
    QColor getColor(double value, double min = -1, double max = 1) override;
};
