#include "sensitivity_analizer.h"

#include "change_iteration/fabric.h"

#include "model/metrics/fabric.h"
#include <qdebug.h>

SensitivityAnalizer::SensitivityAnalizer(const SensitivityAnalysisParameters& parameters, const PredictionParameters& predictionParameters)
    : parameters(parameters), predictionParameters(predictionParameters) {
    predictor = std::make_shared<FinalStatePredictor>(predictionParameters);
    metricsManager = std::make_shared<MetricsManager>(MetricsFabric().create(predictionParameters.metric), predictionParameters);
    gen = std::mt19937(rd());
}

void SensitivityAnalizer::analize(CalculationFCM fcm) {
    conceptsCount = parameters.changeConcepts ? fcm.concepts.size() : 0;
    weightsCount = parameters.changeWeights ? fcm.weights.size() : 0;
    if (parameters.changeConcepts) {
        analyzeConcepts(fcm);
    } else {
        std::lock_guard<std::mutex> lock(_mutex);
        allConceptsFinished = true;
    }
    if (parameters.changeWeights) {
        analyzeWeights(fcm);
    } else {
        std::lock_guard<std::mutex> lock(_mutex);
        allWeightsFinished = true;
    }
    analyzeFcm(fcm);
}

void SensitivityAnalizer::analyzeConcepts(CalculationFCM fcm) {
    const auto resultWithoutChange = predictor->predict(fcm);
    for (auto& [id, concept] : fcm.concepts) {
        double changeSum = 0.0;
        size_t predictionsCount = 0;
        std::unordered_map<double, size_t> conceptChangeCount;
        for (auto [newConcept, change] : ChangeIterationFactory<CalculationConcept>().create(concept, parameters, predictionParameters)) {
            if (
                newConcept.value < 0 || newConcept.value > 1 ||
                newConcept.triangularFuzzyValue.l < 0 || newConcept.triangularFuzzyValue.m < 0 || newConcept.triangularFuzzyValue.u < 0 ||
                newConcept.triangularFuzzyValue.l > 1 || newConcept.triangularFuzzyValue.m > 1 || newConcept.triangularFuzzyValue.u > 1
            ) {
                continue;
            }
            auto oldConcept = concept;
            concept = newConcept;
            auto predictedFcm = predictor->predict(fcm);
            ++predictionsCount;
            concept = oldConcept;
            auto sensitivity = metricsManager->calculate(resultWithoutChange, predictedFcm);
            conceptsChangeSensitivity[id][change] += sensitivity;
            ++conceptChangeCount[change];
            changeSum += sensitivity;
        }
        for (const auto& [change, _] : conceptsChangeSensitivity[id]) {
            conceptsChangeSensitivity[id][change] /= conceptChangeCount[change];
        }
        {
            std::lock_guard<std::mutex> lock(_mutex);
            conceptsSensitivity[id] = predictionsCount ? changeSum / predictionsCount : 0;
            conceptsFinished[id] = true;
            ++conceptsProcessed;
        }
    }
    {
        std::lock_guard<std::mutex> lock(_mutex);
        allConceptsFinished = true;
    }
}

void SensitivityAnalizer::analyzeWeights(CalculationFCM fcm) {
    const auto resultWithoutChange = predictor->predict(fcm);
    for (auto& [id, weight] : fcm.weights) {
        double changeSum = 0.0;
        size_t predictionsCount = 0;
        std::unordered_map<double, size_t> weightChangeCount;
        for (auto [newWeight, change] : ChangeIterationFactory<CalculationWeight>().create(weight, parameters, predictionParameters)) {
            if (
                newWeight.value < 0 || newWeight.value > 1 ||
                newWeight.triangularFuzzyValue.l < 0 || newWeight.triangularFuzzyValue.m < 0 || newWeight.triangularFuzzyValue.u < 0 ||
                newWeight.triangularFuzzyValue.l > 1 || newWeight.triangularFuzzyValue.m > 1 || newWeight.triangularFuzzyValue.u > 1
                ) {
                continue;
            }
            auto oldWeight = weight;
            weight = newWeight;
            auto predictedFcm = predictor->predict(fcm);
            ++predictionsCount;
            weight = oldWeight;
            auto sensitivity = metricsManager->calculate(resultWithoutChange, predictedFcm);
            weightsChangeSensitivity[id][change] += sensitivity;
            ++weightChangeCount[change];
            changeSum += sensitivity;
        }
        for (const auto& [change, _] : weightsChangeSensitivity[id]) {
            weightsChangeSensitivity[id][change] /= weightChangeCount[change];
        }
        {
            std::lock_guard<std::mutex> lock(_mutex);
            weightsSensitivity[id] = predictionsCount ? changeSum / predictionsCount : 0;
            weightsFinished[id] = true;
            ++weightsProcessed;
        }
    }
    {
        std::lock_guard<std::mutex> lock(_mutex);
        allWeightsFinished = true;
    }
}

void SensitivityAnalizer::analyzeFcm(const CalculationFCM& fcm) {
    double step = parameters.maxChange / parameters.steps;
    auto resultWithoutChange = predictor->predict(fcm);
    for (size_t i = 1; i <= parameters.steps; ++i) {
        double sum = 0;
        double maxChange = i * step;
        for (size_t j = 0; j < parameters.randomIterations; ++j) {
            auto changedFcm = randomizeFcm(fcm, maxChange);
            auto predictedFcm = predictor->predict(changedFcm);
            sum += metricsManager->calculate(resultWithoutChange, predictedFcm);
        }
        {
            std::lock_guard<std::mutex> lock(_mutex);
            fcmSensitivity[maxChange] = sum / parameters.randomIterations;
            ++fcmThresholdsProcessed;
        }
    }
    {
        std::lock_guard<std::mutex> lock(_mutex);
        forFcmFinished = true;
    }
}

double SensitivityAnalizer::randomize(double value, double maxChange, double min, double max) {
    std::uniform_real_distribution<double> dist(-maxChange, maxChange);
    double result = min - 1;
    while (result < min || result > max) {
        result = value + dist(gen);
    }
    return result;
}

TriangularFuzzyValue SensitivityAnalizer::randomize(TriangularFuzzyValue value, double maxChange, double min, double max) {
    std::uniform_real_distribution<double> dist(-maxChange, maxChange);
    TriangularFuzzyValue result = {min - 1, 0, 0};
    while (result.l < min || result.m < min || result.u < min || result.l > max || result.m > max || result.u > max || result.l > result.m || result.m > result.u) {
        result = value + TriangularFuzzyValue{dist(gen), dist(gen), dist(gen)};
    }
    return result;
}

CalculationFCM SensitivityAnalizer::randomizeFcm(CalculationFCM fcm, double maxChange) {
    if (parameters.changeConcepts) {
        for (auto& [id, concept] : fcm.concepts) {
            if (predictionParameters.useFuzzyValues) {
                fcm.concepts[id].triangularFuzzyValue = randomize(concept.triangularFuzzyValue, maxChange, 0, 1);
            } else {
                fcm.concepts[id].value = randomize(concept.value, maxChange, 0, 1);
            }
        }
    }
    if (parameters.changeWeights) {
        for (auto& [id, weight] : fcm.weights) {
            if (predictionParameters.useFuzzyValues) {
                fcm.weights[id].triangularFuzzyValue = randomize(weight.triangularFuzzyValue, maxChange, -1, 1);
            } else {
                fcm.weights[id].value = randomize(weight.value, maxChange, -1, 1);
            }
        }
    }
    return fcm;
}

double SensitivityAnalizer::getConceptSensitivity(size_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    return conceptsSensitivity[id];
}

std::unordered_map<double, double> SensitivityAnalizer::getConceptChangeSensitivity(size_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    return conceptsChangeSensitivity[id];
}

double SensitivityAnalizer::getWeightSensitivity(size_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    return weightsSensitivity[id];
}

std::unordered_map<double, double>SensitivityAnalizer::getWeightChangeSensitivity(size_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    return weightsChangeSensitivity[id];
}

std::unordered_map<double, double> SensitivityAnalizer::getFcmSensitivity() {
    std::lock_guard<std::mutex> lock(_mutex);
    return fcmSensitivity;
}

bool SensitivityAnalizer::getConceptFinished(size_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    return conceptsFinished[id];
}

bool SensitivityAnalizer::getWeightFinished(size_t id) {
    std::lock_guard<std::mutex> lock(_mutex);
    return weightsFinished[id];
}

bool SensitivityAnalizer::getFcmFinished() {
    std::lock_guard<std::mutex> lock(_mutex);
    return forFcmFinished;
}

bool SensitivityAnalizer::finished() {
    std::lock_guard<std::mutex> lock(_mutex);
    return allConceptsFinished && allWeightsFinished && forFcmFinished;
}

double SensitivityAnalizer::getProgress() {
    std::lock_guard<std::mutex> lock(_mutex);
    size_t totalWork =
        conceptsCount * (2 * parameters.steps + 1) +
        weightsCount  * (2 * parameters.steps  + 1) +
        parameters.steps  * parameters.randomIterations;
    size_t doneWork =
        conceptsProcessed * (2 * parameters.steps + 1) +
        weightsProcessed  * (2 * parameters.steps  + 1) +
        fcmThresholdsProcessed  * parameters.randomIterations;
    return totalWork ? static_cast<double>(doneWork) / totalWork : 1.0;
}
