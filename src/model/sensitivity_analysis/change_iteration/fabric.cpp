#include "fabric.h"

#include "fuzzy.h"
#include "numeric.h"

#include "model/entities/calculation_concept.h"
#include "model/entities/calculation_weight.h"


template <typename T>
ChangeRange<T> ChangeIterationFactory<T>::create(const T& value, const SensitivityAnalysisParameters& parameters, const PredictionParameters& predictionParameters) {
    double step = parameters.maxChange * 2 / parameters.steps;
    if (predictionParameters.useFuzzyValues) {
        return ChangeRange<T>(
            std::make_unique<FuzzyChangeIterator<T>>(value, step, parameters.steps, -parameters.steps, -parameters.steps, -parameters.steps),
            std::make_unique<FuzzyChangeIterator<T>>(value, step, parameters.steps, -parameters.steps, -parameters.steps, parameters.steps + 1)
        );
    } else {
        return ChangeRange<T>(
            std::make_unique<NumericChangeIterator<T>>(value, step, -parameters.steps),
            std::make_unique<NumericChangeIterator<T>>(value, step, parameters.steps + 1)
        );
    }
}

template class ChangeIterationFactory<CalculationWeight>;
template class ChangeIterationFactory<CalculationConcept>;
