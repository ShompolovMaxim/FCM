#pragma once

#include <map>
#include <QString>

struct FactorMetrics {
    QString conceptName;
    double outDegree = 0;
    double inDegree = 0;
    double centrality = 0;
    double influence = 0;
};

struct StaticAnalysisResult {
    double density = 0;
    double complexity = 0;
    double hierarchyIndex = 0;

    std::map<size_t, FactorMetrics> factors;
};
