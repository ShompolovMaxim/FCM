#include "fuzzy_static_analyzer.h"

#include <cmath>
#include <algorithm>

FuzzyStaticAnalyzer::FuzzyStaticAnalyzer(std::shared_ptr<FCM> fcm)
    : fcm(fcm) {}

const FuzzyStaticAnalysisResult& FuzzyStaticAnalyzer::getResult() const {
    return result;
}

void FuzzyStaticAnalyzer::init() {
    if (!fcm) {
        return;
    }

    N = fcm->concepts.size();
    C = fcm->weights.size();

    od.clear();
    id.clear();
    inCount.clear();
    outCount.clear();
    oldWeights.clear();

    for (const auto& [idC, _] : fcm->concepts) {
        od[idC] = TriangularFuzzyValue{};
        id[idC] = TriangularFuzzyValue{};
        inCount[idC] = 0;
        outCount[idC] = 0;
    }

    for (const auto& [idW, w] : fcm->weights) {
        TriangularFuzzyValue val;

        if (w->term) {
            val = w->term->fuzzyValue;
        }

        oldWeights[idW] = {w->fromConceptId, w->toConceptId, val};

        od[w->fromConceptId] += val;
        id[w->toConceptId] += val;

        outCount[w->fromConceptId]++;
        inCount[w->toConceptId]++;
    }

    for (const auto& [idC, _] : fcm->concepts) {
        if (inCount[idC] > 0) {
            R++;
        }

        if (outCount[idC] > 0) {
            T++;
        }
    }

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
}

void FuzzyStaticAnalyzer::updateDensity() {
    if (N <= 1) {
        result.density = 0.0;
        return;
    }

    result.density = static_cast<double>(C) /
                     static_cast<double>(N * (N - 1));
}

void FuzzyStaticAnalyzer::updateComplexity() {
    if (T == 0) {
        result.complexity = 0.0;
        return;
    }

    result.complexity = static_cast<double>(R) / static_cast<double>(T);
}

void FuzzyStaticAnalyzer::updateHierarchy() {
    if (N <= 2) {
        result.hierarchyIndex = TriangularFuzzyValue{};
        return;
    }

    TriangularFuzzyValue avg;

    for (const auto& [idC, _] : od) {
        avg += od[idC];
    }

    avg /= static_cast<double>(N);

    TriangularFuzzyValue sum;

    for (const auto& [idC, _] : od) {
        TriangularFuzzyValue diff;

        diff = od[idC];
        diff -= avg;

        TriangularFuzzyValue sq;
        sq = diff;
        sq *= diff;

        sum += sq;
    }

    double coef = 12.0 / ((N - 1) * N * (N + 1));

    result.hierarchyIndex = sum;
    result.hierarchyIndex *= TriangularFuzzyValue{coef, coef, coef};
}

void FuzzyStaticAnalyzer::updateFactors() {
    result.factors.clear();

    for (const auto& [idC, concept] : fcm->concepts) {
        FuzzyFactorMetrics m;

        m.conceptName = concept->name;

        m.outDegree = od[idC];
        m.inDegree = id[idC];

        m.centrality = m.outDegree;
        m.centrality += m.inDegree;

        result.factors[idC] = m;
    }
}

void FuzzyStaticAnalyzer::onConceptCreated(std::shared_ptr<Concept> concept) {
    QUuid idc = concept->id;

    N++;

    od[idc] = TriangularFuzzyValue{};
    id[idc] = TriangularFuzzyValue{};
    inCount[idc] = 0;
    outCount[idc] = 0;

    updateDensity();
    updateHierarchy();
    updateFactors();
}

void FuzzyStaticAnalyzer::onConceptDeleted(QUuid idc) {
    od.erase(idc);
    id.erase(idc);
    inCount.erase(idc);
    outCount.erase(idc);

    N--;

    updateDensity();
    updateHierarchy();
    updateFactors();
}

void FuzzyStaticAnalyzer::onWeightCreated(std::shared_ptr<Weight> weight) {
    TriangularFuzzyValue val;

    if (weight->term) {
        val = weight->term->fuzzyValue;
    }

    oldWeights[weight->id] = {
        weight->fromConceptId,
        weight->toConceptId,
        val
    };

    C++;

    if (outCount[weight->fromConceptId] == 0) {
        T++;
    }

    if (inCount[weight->toConceptId] == 0) {
        R++;
    }

    outCount[weight->fromConceptId]++;
    inCount[weight->toConceptId]++;

    od[weight->fromConceptId] += val;
    id[weight->toConceptId] += val;

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
}

void FuzzyStaticAnalyzer::onWeightUpdated(std::shared_ptr<Weight> weight) {
    WeightInfo& old = oldWeights[weight->id];

    TriangularFuzzyValue now;

    if (weight->term) {
        now = weight->term->fuzzyValue;
    }

    TriangularFuzzyValue diff;
    diff = now;
    diff -= old.value;

    old.value = now;

    od[weight->fromConceptId] += diff;
    id[weight->toConceptId] += diff;

    updateHierarchy();
    updateFactors();
}

void FuzzyStaticAnalyzer::onWeightDeleted(QUuid idw) {
    WeightInfo w = oldWeights[idw];

    C--;

    outCount[w.from]--;
    if (outCount[w.from] == 0) {
        T--;
    }

    inCount[w.to]--;
    if (inCount[w.to] == 0) {
        R--;
    }

    od[w.from] -= w.value;
    id[w.to] -= w.value;

    oldWeights.erase(idw);

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
}

void FuzzyStaticAnalyzer::updateInfluence(QUuid conceptId, size_t steps, bool from) {
    influenceConceptId = conceptId;
    influenceSteps = steps;
    influenceFrom = from;

    updateInfluence();
}

void FuzzyStaticAnalyzer::updateInfluence() {
    if (influenceFrom) {
        updateInfluenceFrom(influenceConceptId, influenceSteps);
    } else {
        updateInfluenceTo(influenceConceptId, influenceSteps);
    }
}

void FuzzyStaticAnalyzer::updateInfluenceTo(QUuid conceptId, size_t steps) {
    if (!fcm->concepts.count(conceptId)) {
        return;
    }

    std::map<QUuid, std::map<QUuid, TriangularFuzzyValue>> W;

    for (const auto& [idW, w] : fcm->weights) {
        if (w->term) {
            W[w->fromConceptId][w->toConceptId] = w->term->fuzzyValue;
        }
    }

    std::map<QUuid, TriangularFuzzyValue> influence;

    for (const auto& [idC, _] : fcm->concepts) {
        influence[idC] = W[idC][conceptId];
    }

    for (size_t step = 0; step + 1 < steps; step++) {
        std::map<QUuid, TriangularFuzzyValue> nextInfluence = influence;

        for (const auto& [idC, _] : fcm->concepts) {

            TriangularFuzzyValue sum;

            for (const auto& [idTo, _] : fcm->concepts) {
                if (W[idC].count(idTo)) {
                    sum += W[idC][idTo] * influence[idTo];
                }
            }

            nextInfluence[idC] = sum + influence[idC];
        }

        influence = nextInfluence;
    }

    for (auto& [idC, m] : result.factors) {
        m.influence = influence[idC];
    }
}

void FuzzyStaticAnalyzer::updateInfluenceFrom(QUuid conceptId, size_t steps) {
    if (!fcm->concepts.count(conceptId)) {
        return;
    }

    std::map<QUuid, std::map<QUuid, TriangularFuzzyValue>> W;

    for (const auto& [idW, w] : fcm->weights) {
        if (w->term) {
            W[w->toConceptId][w->fromConceptId] = w->term->fuzzyValue;
        }
    }

    std::map<QUuid, TriangularFuzzyValue> influence;

    for (const auto& [idC, _] : fcm->concepts) {
        influence[idC] = W[idC][conceptId];
    }

    for (size_t step = 0; step + 1 < steps; step++) {
        std::map<QUuid, TriangularFuzzyValue> nextInfluence = influence;

        for (const auto& [idC, _] : fcm->concepts) {

            TriangularFuzzyValue sum;

            for (const auto& [idFrom, _] : fcm->concepts) {
                if (W[idC].count(idFrom)) {
                    sum += W[idC][idFrom] * influence[idFrom];
                }
            }

            nextInfluence[idC] = sum + influence[idC];
        }

        influence = nextInfluence;
    }

    for (auto& [idC, m] : result.factors) {
        m.influence = influence[idC];
    }
}

StaticAnalysisResult FuzzyStaticAnalyzer::getNumericResult() const {
    StaticAnalysisResult out;

    out.density = result.density;
    out.complexity = result.complexity;

    out.hierarchyIndex = result.hierarchyIndex.defuzzify();

    for (const auto& [idC, fuzzyFactor] : result.factors) {
        FactorMetrics m;

        m.conceptName = fuzzyFactor.conceptName;

        m.inDegree = fuzzyFactor.inDegree.defuzzify();
        m.outDegree = fuzzyFactor.outDegree.defuzzify();
        m.centrality = fuzzyFactor.centrality.defuzzify();
        m.influence = fuzzyFactor.influence.defuzzify();

        out.factors[idC] = m;
    }

    return out;
}
