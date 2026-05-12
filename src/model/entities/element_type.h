#pragma once

#include <QString>

enum class ElementType {
    Edge,
    Node
};

inline QString elementTypeToString(ElementType type) {
    switch(type) {
        case ElementType::Edge: return "Edge";
        default: return "Node";
    }
}

inline ElementType elementTypeFromString(const QString& str) {
    if (str == "Edge") {
        return ElementType::Edge;
    }
    return ElementType::Node;
}
