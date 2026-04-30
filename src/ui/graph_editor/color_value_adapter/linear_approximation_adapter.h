#pragma once

#include "IColorValueAdapter.h"

#include "model/entities/term.h"

#include <map>

class LinearApproximationColorValueAdapter : public IColorValueAdapter {
public:
    LinearApproximationColorValueAdapter(std::map<QUuid, std::shared_ptr<Term>> terms);

    QColor getColor(double value, double min = -1, double max = 1, bool conceptColor = true, bool fuzzyValues = false);

private:
    std::map<QUuid, std::shared_ptr<Term>> terms;
};
