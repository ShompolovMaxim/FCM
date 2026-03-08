#pragma once

#include "model/entities/fcm.h"

#include <vector>
#include <map>

struct FactorMetrics
{
    QString conceptName;

    double outDegree;
    double inDegree;
    double centrality;
};

struct StaticAnalysisResult
{
    double density;
    double complexity;
    double hierarchyIndex;

    std::vector<FactorMetrics> factors;
};

class StaticAnalyzer
{
public:
    static StaticAnalysisResult analyze(const FCM& fcm);
};
