#include "concept_name_location.h"

QString conceptNameLocationToString(ConceptNameLocation location) {
    switch (location) {
    case ConceptNameLocation::Up:
        return "up";
    case ConceptNameLocation::UpLeft:
        return "up left";
    case ConceptNameLocation::UpRight:
        return "up right";
    case ConceptNameLocation::Center:
        return "center";
    case ConceptNameLocation::CenterLeft:
        return "center left";
    case ConceptNameLocation::CenterRight:
        return "center right";
    case ConceptNameLocation::Bottom:
        return "bottom";
    case ConceptNameLocation::BottomLeft:
        return "bottom left";
    case ConceptNameLocation::BottomRight:
        return "bottom right";
    }

    return "up";
}

ConceptNameLocation conceptNameLocationFromString(const QString& value) {
    if (value == "up left") {
        return ConceptNameLocation::UpLeft;
    }
    if (value == "up right") {
        return ConceptNameLocation::UpRight;
    }
    if (value == "center") {
        return ConceptNameLocation::Center;
    }
    if (value == "center left") {
        return ConceptNameLocation::CenterLeft;
    }
    if (value == "center right") {
        return ConceptNameLocation::CenterRight;
    }
    if (value == "bottom") {
        return ConceptNameLocation::Bottom;
    }
    if (value == "bottom left") {
        return ConceptNameLocation::BottomLeft;
    }
    if (value == "bottom right") {
        return ConceptNameLocation::BottomRight;
    }

    return ConceptNameLocation::Up;
}
