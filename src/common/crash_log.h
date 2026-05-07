#pragma once

#include <QDebug>

class Logger {
public:
    static void warn(const char* message) {
        qWarning().noquote() << message;
    }

    static void critical(const char* message) {
        qCritical().noquote() << message;
    }
};
