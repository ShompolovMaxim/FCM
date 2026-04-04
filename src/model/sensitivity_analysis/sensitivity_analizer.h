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

    double getConceptSensitivity(QUuid id);
    double getWeightSensitivity(QUuid id);
    std::map<double, double> getConceptChangeSensitivity(QUuid id);
    std::map<double, double> getWeightChangeSensitivity(QUuid id);
    std::map<double, double> getFcmSensitivity();

    bool getConceptFinished(QUuid id);
    bool getWeightFinished(QUuid id);
    bool getFcmFinished();
    bool finished();
    double getProgress();

private:
    void analyzeConcepts(CalculationFCM fcm);
    void analyzeWeights(CalculationFCM fcm);
    void analyzeFcm(const CalculationFCM& fcm);

    double randomize(double value, double maxChange, double min, double max);
    TriangularFuzzyValue randomize(TriangularFuzzyValue value, double maxChange, double min, double max);
    CalculationFCM randomizeFcm(CalculationFCM value, double maxChange);

    const PredictionParameters predictionParameters;
    const SensitivityAnalysisParameters  parameters;

    std::map<QUuid, double> conceptsSensitivity;
    std::map<QUuid, double> weightsSensitivity;
    std::map<QUuid, std::map<double, double>> conceptsChangeSensitivity;
    std::map<QUuid, std::map<double, double>> weightsChangeSensitivity;
    std::map<double, double> fcmSensitivity;

    std::map<QUuid, bool> conceptsFinished;
    std::map<QUuid, bool> weightsFinished;
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
