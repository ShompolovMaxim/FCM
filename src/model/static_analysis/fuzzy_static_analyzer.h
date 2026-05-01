#pragma once

#include <map>
#include <memory>
#include "model/entities/fcm.h"
#include "result.h"

class FuzzyStaticAnalyzer {
public:
    explicit FuzzyStaticAnalyzer(std::shared_ptr<FCM> fcm);

    void init();

    void onConceptCreated(std::shared_ptr<Concept> concept);
    void onConceptDeleted(QUuid id);

    void onWeightCreated(std::shared_ptr<Weight> weight);
    void onWeightUpdated(std::shared_ptr<Weight> weight);
    void onWeightDeleted(QUuid id);

    const FuzzyStaticAnalysisResult& getResult() const;
    StaticAnalysisResult getNumericResult() const;

    void updateInfluence(QUuid conceptId, size_t steps, bool influenceFrom);

private:
    void updateDensity();
    void updateComplexity();
    void updateHierarchy();
    void updateFactors();

    void updateInfluence();
    void updateInfluenceTo(QUuid conceptId, size_t steps);
    void updateInfluenceFrom(QUuid conceptId, size_t steps);

private:
    std::shared_ptr<FCM> fcm;

    FuzzyStaticAnalysisResult result;

    std::map<QUuid, TriangularFuzzyValue> od;
    std::map<QUuid, TriangularFuzzyValue> id;

    std::map<QUuid, int> inCount;
    std::map<QUuid, int> outCount;

    struct WeightInfo {
        QUuid from;
        QUuid to;
        TriangularFuzzyValue value;
    };

    std::map<QUuid, WeightInfo> oldWeights;

    size_t N = 0;
    size_t C = 0;

    size_t R = 0;
    size_t T = 0;

    QUuid influenceConceptId;
    size_t influenceSteps = 1;
    bool influenceFrom = false;
};
