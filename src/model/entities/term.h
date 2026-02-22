#pragma once
#include <QString>
struct Term {
    size_t id;
    QString name;
    double value;
    double fuzzyValueL;
    double fuzzyValueM;
    double fuzzyValueU;
    QString description;
};
