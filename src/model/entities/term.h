#pragma once

#include <QColor>
#include <QString>

struct Term {
    size_t id;
    QString name;
    QString description;
    double value;
    double fuzzyValueL;
    double fuzzyValueM;
    double fuzzyValueU;
    QColor color;
};
