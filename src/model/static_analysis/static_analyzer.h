#pragma once

#include <map>
#include <memory>
#include "model/entities/fcm.h"
#include "result.h"

class StaticAnalyzer {
public:
    explicit StaticAnalyzer(std::shared_ptr<FCM> fcm);

    void init();

    void onConceptCreated(std::shared_ptr<Concept> concept);
    void onConceptDeleted(size_t id);

    void onWeightCreated(std::shared_ptr<Weight> weight);
    void onWeightUpdated(std::shared_ptr<Weight> weight);
    void onWeightDeleted(size_t id);

    const StaticAnalysisResult& getResult() const;

    void updateInfluence(size_t conceptId, size_t steps, bool influenceFrom);

private:
    void updateDensity();
    void updateComplexity();
    void updateHierarchy();
    void updateFactors();
    void updateInfluence();
    void updateInfluenceTo(size_t conceptId, size_t steps);
    void updateInfluenceFrom(size_t conceptId, size_t steps);

private:
    std::shared_ptr<FCM> fcm;

    StaticAnalysisResult result;

    std::map<size_t, double> od;
    std::map<size_t, double> id;

    std::map<size_t, int> inCount;
    std::map<size_t, int> outCount;

    struct WeightInfo {
        size_t from;
        size_t to;
        double value;
    };

    std::map<size_t, WeightInfo> oldWeights;

    size_t N = 0;
    size_t C = 0;

    size_t R = 0;
    size_t T = 0;

    size_t influenceConceptId;
    size_t influenceSteps;
    bool influenceFrom;
};
