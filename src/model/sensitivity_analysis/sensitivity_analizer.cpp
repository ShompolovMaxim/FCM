#include "sensitivity_analizer.h"

#include "model/metrics/fabric.h"

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
    double step = parameters.maxChange * 2 / parameters.steps;
    const auto resultWithoutChange = predictor->predict(fcm);
    for (auto& [id, concept] : fcm.concepts) {
        double changeSum = 0.0;
        size_t predictionsCount = 0;
        int steps = parameters.steps;
        for (int i = -steps; i <= steps; ++i) {
            double newConceptValue = concept.value + step * i;
            if (newConceptValue < 0 || newConceptValue > 1) {
                continue;
            }
            double oldConceptValue = concept.value;
            concept.value = newConceptValue;
            auto predictedFcm = predictor->predict(fcm);
            ++predictionsCount;
            concept.value = oldConceptValue;
            auto sensitivity = metricsManager->calculate(resultWithoutChange, predictedFcm);
            conceptsChangeSensitivity[id][step * i] = sensitivity;
            changeSum += sensitivity;
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
    double step = parameters.maxChange * 2 / parameters.steps;
    const auto resultWithoutChange = predictor->predict(fcm);
    for (auto& [id, weight] : fcm.weights) {
        double changeSum = 0.0;
        size_t predictionsCount = 0;
        int steps = parameters.steps;
        for (int i = -steps; i <= steps; ++i) {
            double newWeightValue = weight.value + step * i;
            if (newWeightValue < -1 || newWeightValue > 1) {
                continue;
            }
            double oldWeightValue = weight.value;
            weight.value = newWeightValue;
            auto predictedFcm = predictor->predict(fcm);
            ++predictionsCount;
            weight.value = oldWeightValue;
            auto sensitivity = metricsManager->calculate(resultWithoutChange, predictedFcm);
            weightsChangeSensitivity[id][step * i] = sensitivity;
            changeSum += sensitivity;
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

CalculationFCM SensitivityAnalizer::randomizeFcm(CalculationFCM fcm, double maxChange) {
    if (parameters.changeConcepts) {
        for (auto& [id, concept] : fcm.concepts) {
            fcm.concepts[id].value = randomize(concept.value, maxChange, 0, 1);
        }
    }
    if (parameters.changeWeights) {
        for (auto& [id, weight] : fcm.weights) {
            fcm.weights[id].value = randomize(weight.value, maxChange, -1, 1);
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
