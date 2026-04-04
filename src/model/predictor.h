#pragma once
#include "model/entities/calculation_fcm.h"
#include "presenter/prediction_parameters.h"

#include <atomic>
#include <mutex>
#include <variant>

class Predictor {
public:
    Predictor(PredictionParameters predictionParameters, const CalculationFCM& fcm);

    void perform();

    CalculationFCM getFCM(size_t step);

    size_t getCount();
    bool getFinished();

    std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> getConceptHistoryValues(QUuid conceptId, size_t step);
    std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> getWeightHistoryValues(QUuid weightId, size_t step);
private:
    PredictionParameters _predictionParameters;
    std::vector<CalculationFCM> _fcms;
    std::mutex _mutex;
    std::atomic_size_t _count = 0;
    std::atomic_bool finished = false;
};
