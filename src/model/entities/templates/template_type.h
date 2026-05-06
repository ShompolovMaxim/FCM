#pragma once

#include <QString>

enum class TemplateType {
    User,
    DefaultEnglish,
    DefaultRussian
};

QString templateTypeToString(TemplateType type);
TemplateType templateTypeFromString(const QString &value);
