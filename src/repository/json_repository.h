#pragma once

#include <QString>
#include <optional>
#include "model/entities/fcm.h"

class JsonRepository {
public:
    static bool exportToJson(const FCM& fcm, const QString& path);
    static std::optional<FCM> importFromJson(const QString& path);
};
