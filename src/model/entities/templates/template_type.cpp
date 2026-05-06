#include "template_type.h"

QString templateTypeToString(TemplateType type) {
    switch (type) {
    case TemplateType::User:
        return "User";
    case TemplateType::DefaultEnglish:
        return "DefaultEnglish";
    case TemplateType::DefaultRussian:
        return "DefaultRussian";
    }

    return "User";
}

TemplateType templateTypeFromString(const QString &value) {
    if (value == "DefaultEnglish") {
        return TemplateType::DefaultEnglish;
    }
    if (value == "DefaultRussian") {
        return TemplateType::DefaultRussian;
    }
    return TemplateType::User;
}
