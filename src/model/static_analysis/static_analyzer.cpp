#include "static_analyzer.h"

StaticAnalyzer::StaticAnalyzer(std::shared_ptr<FCM> fcm) : fcm(fcm) {}

void StaticAnalyzer::init() {
    if (!fcm) return;

    N = fcm->concepts.size();
    C = fcm->weights.size();

    for (const auto& [idConcept, concept] : fcm->concepts) {
        od[idConcept] = 0.0;
        id[idConcept] = 0.0;
        inCount[idConcept] = 0;
        outCount[idConcept] = 0;
    }

    for (const auto& [idWeight, w] : fcm->weights) {
        double val = w->term ? w->term->value : 0;

        oldWeights[idWeight] = {w->fromConceptId, w->toConceptId, val};

        od[w->fromConceptId] += val;
        id[w->toConceptId] += val;

        outCount[w->fromConceptId]++;
        inCount[w->toConceptId]++;
    }

    for (const auto& [idConcept, _] : fcm->concepts) {
        if (inCount[idConcept] > 0) R++;
        if (outCount[idConcept] > 0) T++;
    }

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
}

const StaticAnalysisResult& StaticAnalyzer::getResult() const {
    return result;
}

void StaticAnalyzer::onConceptCreated(std::shared_ptr<Concept> c) {
    size_t idc = c->id;

    N++;

    od[idc] = 0;
    id[idc] = 0;
    inCount[idc] = 0;
    outCount[idc] = 0;

    updateDensity();
    updateHierarchy();
    updateFactors();
}

void StaticAnalyzer::onConceptDeleted(size_t idc) {
    if (inCount[idc] > 0) R--;
    if (outCount[idc] > 0) T--;

    od.erase(idc);
    id.erase(idc);
    inCount.erase(idc);
    outCount.erase(idc);

    N--;

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
}

void StaticAnalyzer::onWeightCreated(std::shared_ptr<Weight> w) {
    double val = w->term ? w->term->value : 0;

    oldWeights[w->id] = {w->fromConceptId, w->toConceptId, val};

    C++;

    if (outCount[w->fromConceptId] == 0) T++;
    outCount[w->fromConceptId]++;

    if (inCount[w->toConceptId] == 0) R++;
    inCount[w->toConceptId]++;

    od[w->fromConceptId] += val;
    id[w->toConceptId] += val;

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
    updateInfluence();
}

void StaticAnalyzer::onWeightUpdated(std::shared_ptr<Weight> w) {
    double old = oldWeights[w->id].value;
    double now = w->term ? w->term->value : 0;

    double diff = now - old;

    oldWeights[w->id].value = now;

    od[w->fromConceptId] += diff;
    id[w->toConceptId] += diff;

    updateHierarchy();
    updateFactors();
    updateInfluence();
}

void StaticAnalyzer::onWeightDeleted(size_t idw) {
    auto w = oldWeights[idw];

    C--;

    outCount[w.from]--;
    if (outCount[w.from] == 0) T--;

    inCount[w.to]--;
    if (inCount[w.to] == 0) R--;

    od[w.from] -= w.value;
    id[w.to] -= w.value;

    oldWeights.erase(idw);

    updateDensity();
    updateComplexity();
    updateHierarchy();
    updateFactors();
    updateInfluence();
}

void StaticAnalyzer::updateInfluence(size_t conceptId, size_t steps, bool influenceFrom) {
    influenceConceptId = conceptId;
    influenceSteps = steps;
    this->influenceFrom = influenceFrom;
    updateInfluence();
}

void StaticAnalyzer::updateInfluence() {
    if (influenceFrom) {
        updateInfluenceFrom(influenceConceptId, influenceSteps);
    } else {
        updateInfluenceTo(influenceConceptId, influenceSteps);
    }
}

void StaticAnalyzer::updateDensity() {
    if (N <= 2) {
        result.density = 0;
        return;
    }
    result.density = static_cast<double>(C) / (N * (N - 1));
}

void StaticAnalyzer::updateComplexity() {
    result.complexity = (T != 0) ? static_cast<double>(R) / T : 0;
}

void StaticAnalyzer::updateHierarchy() {
    if (N <= 2) {
        result.hierarchyIndex = 0;
        return;
    }

    double avg = 0;
    for (auto& [idc, _] : od) {
        avg += od[idc];
    }
    avg /= N;

    double sum = 0;
    for (auto& [idc, _] : od) {
        sum += pow(od[idc] - avg, 2);
    }

    result.hierarchyIndex = (12.0 / ((N - 1) * N * (N + 1))) * sum;
}

void StaticAnalyzer::updateInfluenceTo(size_t conceptId, size_t steps) {
    if (!fcm->concepts.count(conceptId)) return;

    std::map<size_t, std::map<size_t, double>> W;
    for (const auto& [idWeight, w] : fcm->weights) {
        if (w->term) {
            W[w->fromConceptId][w->toConceptId] = w->term->value;
        }
    }

    std::map<size_t, double> influence;
    for (const auto& [id, _] : fcm->concepts) {
        influence[id] = W[id][conceptId];
    }

    for (size_t step = 0; step + 1 < steps; ++step) {
        std::map<size_t, double> newInfluence = influence;
        for (const auto& [idC, _] : fcm->concepts) {
            double sum = 0.0;
            for (const auto& [idTo, _] : fcm->concepts) {
                if (W[idC].count(idTo)) {
                    sum += W[idC][idTo] * influence[idTo];
                }
            }
            newInfluence[idC] = sum + influence[idC];
        }
        influence = newInfluence;
    }

    for (auto& [idc, m] : result.factors) {
        if (influence.count(idc)) {
            m.influence = influence[idc];
        }
    }
}

void StaticAnalyzer::updateInfluenceFrom(size_t conceptId, size_t steps) {
    if (!fcm->concepts.count(conceptId)) return;

    std::map<size_t, std::map<size_t, double>> W;
    for (const auto& [idWeight, w] : fcm->weights) {
        if (w->term) {
            W[w->toConceptId][w->fromConceptId] = w->term->value;
        }
    }

    std::map<size_t, double> influence;
    for (const auto& [id, _] : fcm->concepts) {
        influence[id] = W[id][conceptId];
    }

    for (size_t step = 0; step + 1 < steps; ++step) {
        std::map<size_t, double> newInfluence = influence;
        for (const auto& [idC, _] : fcm->concepts) {
            double sum = 0.0;
            for (const auto& [idFrom, _] : fcm->concepts) {
                if (W[idC].count(idFrom)) {
                    sum += W[idC][idFrom] * influence[idFrom];
                }
            }
            newInfluence[idC] = sum  + influence[idC];
        }
        influence = newInfluence;
    }

    for (auto& [idc, m] : result.factors) {
        if (influence.count(idc)) {
            m.influence = influence[idc];
        }
    }
}

void StaticAnalyzer::updateFactors() {
    result.factors.clear();

    for (const auto& [idc, concept] : fcm->concepts) {
        FactorMetrics m;
        m.conceptName = concept->name;
        m.outDegree = od[idc];
        m.inDegree = id[idc];
        m.centrality = m.outDegree + m.inDegree;

        result.factors[idc] = m;
    }
}
