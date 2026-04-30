#include "linear_approximation_adapter.h"
#include "color_value_adapter.h"

LinearApproximationColorValueAdapter::LinearApproximationColorValueAdapter(std::map<QUuid, std::shared_ptr<Term>> terms) : terms(terms) {}

QColor LinearApproximationColorValueAdapter::getColor(double value, double min, double max, bool conceptColor, bool fuzzyValues) {
    std::vector<double> values;
    std::map<double, QColor> valuesColors;
    for (const auto& [_, term] : terms) {
        if (conceptColor ^ (term->type == ElementType::Edge)) {
            double value = fuzzyValues ? term->fuzzyValue.defuzzify() : term->value;
            if (std::find(values.begin(), values.end(), value) != values.end()) {
                continue;
            }
            values.push_back(value);
            valuesColors[value] = term->color;
        }
    }
    if (values.empty()) {
        return ColorValueAdapter().getColor(value, min, max);
    }
    std::sort(values.begin(), values.end());
    size_t i = 0;
    while (i < values.size() && value > values[i]) {
        ++i;
    }
    if (i == values.size()) {
        return valuesColors[values.back()];
    }
    if (i == 0) {
        return valuesColors[values[0]];
    }
    auto c0 = valuesColors[values[i - 1]];
    auto c1 = valuesColors[values[i]];
    double t = (value - values[i - 1]) / (values[i] - values[i - 1]);
    t = std::clamp(t, 0.0, 1.0);
    int r = c0.red()   + t * (c1.red()   - c0.red());
    int g = c0.green() + t * (c1.green() - c0.green());
    int b = c0.blue()  + t * (c1.blue()  - c0.blue());
    int a = c0.alpha() + t * (c1.alpha() - c0.alpha());
    return QColor(r, g, b, a);
}
