#include "static_analyzer.h"

#include <cmath>

StaticAnalysisResult StaticAnalyzer::analyze(const FCM& fcm)
{
    StaticAnalysisResult result;

    size_t N = fcm.concepts.size();
    size_t C = fcm.weights.size();

    if (N == 0) {
        return result;
    }

    result.density = static_cast<double>(C) / (N * (N - 1));

    std::map<size_t,double> od;
    std::map<size_t,double> id;

    for (const auto& [idConcept, concept] : fcm.concepts)
    {
        od[idConcept] = 0.0;
        id[idConcept] = 0.0;
    }

    for (const auto& [idWeight, weight] : fcm.weights)
    {
        od[weight.fromConceptId] += weight.value;
        id[weight.toConceptId] += weight.value;
    }

    size_t R = 0;
    size_t T = 0;

    for (const auto& [idConcept, concept] : fcm.concepts)
    {
        if (id[idConcept] != 0) {
            R++;
        }

        if (od[idConcept] != 0) {
            T++;
        }

        FactorMetrics m;

        m.conceptName = concept->name;
        m.outDegree = od[idConcept];
        m.inDegree = id[idConcept];
        m.centrality = m.outDegree + m.inDegree;

        result.factors.push_back(m);
    }

    if (T != 0) {
        result.complexity = static_cast<double>(R) / T;
    }

    double avg = 0;

    for (const auto& [idConcept, concept] : fcm.concepts) {
        avg += od[idConcept];
    }

    avg /= N;

    double sum = 0;

    for (const auto& [idConcept, concept] : fcm.concepts) {
        sum += pow(od[idConcept] - avg, 2);
    }

    result.hierarchyIndex = (12.0 / ((N - 1) * N * (N + 1))) * sum;

    return result;
}
