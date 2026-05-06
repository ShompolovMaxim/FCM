#pragma once

#include <QString>

enum class ConceptNameLocation {
    Up,
    UpLeft,
    UpRight,
    Center,
    CenterLeft,
    CenterRight,
    Bottom,
    BottomLeft,
    BottomRight
};

QString conceptNameLocationToString(ConceptNameLocation location);
ConceptNameLocation conceptNameLocationFromString(const QString& value);
