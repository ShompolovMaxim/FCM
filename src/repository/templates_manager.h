#pragma once

#include "model/entities/fcm.h"
#include "templates_repository.h"

#include <map>
#include <optional>

#include <QList>

class TemplatesManager {
public:
    explicit TemplatesManager(TemplatesRepository repository);

    bool createTemplate(const FCM &fcm);
    std::optional<FCM> getFCM(const QString &templateName);
    QList<QString> getTemplatesNames();
    bool deleteTemplate(const QString &templateName);

private:
    Template toTemplate(const FCM &fcm);
    FCM toFCM(const Template &templateModel);

    TemplatesRepository repo;
};
