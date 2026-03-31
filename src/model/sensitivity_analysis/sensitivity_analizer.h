#pragma once

#include "parameters.h"

#include "model/entities/calculation_fcm.h"
#include "model/final_state_predictor.h"
#include "model/metrics_manager.h"

#include "presenter/prediction_parameters.h"

#include <cstdint>
#include <mutex>
#include <random>

class SensitivityAnalizer {
public:
    SensitivityAnalizer(const SensitivityAnalysisParameters&  parameters, const PredictionParameters& predictionParameters);

    void analize(CalculationFCM fcm);

    double getConceptSensitivity(size_t id);
    double getWeightSensitivity(size_t id);
    std::unordered_map<double, double> getConceptChangeSensitivity(size_t id);
    std::unordered_map<double, double> getWeightChangeSensitivity(size_t id);
    std::unordered_map<double, double> getFcmSensitivity();

    bool getConceptFinished(size_t id);
    bool getWeightFinished(size_t id);
    bool getFcmFinished();
    bool finished();
    double getProgress();

private:
    void analyzeConcepts(CalculationFCM fcm);
    void analyzeWeights(CalculationFCM fcm);
    void analyzeFcm(const CalculationFCM& fcm);

    double randomize(double value, double maxChange, double min, double max);
    CalculationFCM randomizeFcm(CalculationFCM value, double maxChange);

    const PredictionParameters predictionParameters;
    const SensitivityAnalysisParameters  parameters;

    std::unordered_map<size_t, double> conceptsSensitivity;
    std::unordered_map<size_t, double> weightsSensitivity;
    std::unordered_map<size_t, std::unordered_map<double, double>> conceptsChangeSensitivity;
    std::unordered_map<size_t, std::unordered_map<double, double>> weightsChangeSensitivity;
    std::unordered_map<double, double> fcmSensitivity;

    std::unordered_map<size_t, bool> conceptsFinished;
    std::unordered_map<size_t, bool> weightsFinished;
    bool allConceptsFinished = false;
    bool allWeightsFinished = false;
    bool forFcmFinished = false;

    std::shared_ptr<FinalStatePredictor> predictor;
    std::shared_ptr<MetricsManager> metricsManager;

    std::mutex _mutex;

    std::random_device rd;
    std::mt19937 gen;

    size_t conceptsProcessed = 0;
    size_t weightsProcessed = 0;
    size_t fcmThresholdsProcessed = 0;
    size_t conceptsCount;
    size_t weightsCount;

};
