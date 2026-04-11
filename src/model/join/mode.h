#pragma once

#include <QString>

enum class JoinMode {
    Numeric,
    Fuzzy,
    Gibrid
};

inline QString joinModeToString(JoinMode mode) {
    if (mode == JoinMode::Numeric) {
        return "numeric";
    } else if (mode == JoinMode::Fuzzy) {
        return "fuzzy";
    } else {
        return "gibrid";
    }
}

inline JoinMode joinModeFromString(const QString& str) {
    if (str == "Numeric") {
        return JoinMode::Numeric;
    } else if (str == "Fuzzy") {
        return JoinMode::Fuzzy;
    } else if (str == "Gibrid") {
        return JoinMode::Gibrid;
    }

    return JoinMode::Numeric;
}
