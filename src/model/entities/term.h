#pragma once

#include <QColor>
#include <QString>

struct Term {
    size_t id;
    QString name;
    QString description;
    double value = 0;
    double fuzzyValueL = 0;
    double fuzzyValueM = 0;
    double fuzzyValueU = 0;
    QColor color;
};
