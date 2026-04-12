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
    if (str == "numeric") {
        return JoinMode::Numeric;
    } else if (str == "fuzzy") {
        return JoinMode::Fuzzy;
    } else if (str == "gibrid") {
        return JoinMode::Gibrid;
    }

    return JoinMode::Numeric;
}
