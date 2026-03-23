#pragma once
#include "model/entities/calculation_fcm.h"
#include "presenter/prediction_parameters.h"

#include <mutex>
#include <atomic>

class Predictor
{
public:
    Predictor(PredictionParameters predictionParameters, const CalculationFCM& fcm);

    void perform();

    CalculationFCM getFCM(size_t step);

    double getCount();

    std::vector<double> getConceptHistoryValues(size_t conceptId, size_t step);
    std::vector<double> getWeightHistoryValues(size_t weightId, size_t step);
private:
    PredictionParameters _predictionParameters;
    std::vector<CalculationFCM> _fcms;
    std::mutex _mutex;
    std::atomic_size_t _count = 0;
};

