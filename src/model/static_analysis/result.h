#pragma once

#include <map>
#include <QString>
#include <QUuid>
#include "model/entities/triangular_fuzzy_value.h"

template<typename T>
struct FactorMetricsT {
    QString conceptName;

    T inDegree{};
    T outDegree{};
    T centrality{};
    T influence{};
};

template<typename T>
struct StaticAnalysisResultT {
    double density{};
    double complexity{};
    T hierarchyIndex{};

    std::map<QUuid, FactorMetricsT<T>> factors;
};

using FactorMetrics = FactorMetricsT<double>;
using StaticAnalysisResult = StaticAnalysisResultT<double>;

using FuzzyFactorMetrics = FactorMetricsT<TriangularFuzzyValue>;
using FuzzyStaticAnalysisResult = StaticAnalysisResultT<TriangularFuzzyValue>;
