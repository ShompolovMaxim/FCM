#include "templates_language_manager.h"

#include <QSettings>

TemplateType TemplatesLanguageManager::currentDefaultTemplateType() {
    const QSettings settings("app.ini", QSettings::IniFormat);
    return settings.value("language", "").toString().isEmpty()
        ? TemplateType::DefaultEnglish
        : TemplateType::DefaultRussian;
}

QStringList TemplatesLanguageManager::filterTemplateNamesForCurrentLanguage(
    const QList<QPair<QString, TemplateType>> &templatesNamesWithTypes
) {
    const TemplateType currentType = currentDefaultTemplateType();
    QStringList templateNames;
    templateNames.reserve(templatesNamesWithTypes.size());

    for (const auto &[templateName, templateType] : templatesNamesWithTypes) {
        if (templateType == TemplateType::User || templateType == currentType) {
            templateNames.append(templateName);
        }
    }

    return templateNames;
}

QStringList TemplatesLanguageManager::extractTemplateNames(
    const QList<QPair<QString, TemplateType>> &templatesNamesWithTypes
) {
    QStringList templateNames;
    templateNames.reserve(templatesNamesWithTypes.size());

    for (const auto &[templateName, _] : templatesNamesWithTypes) {
        templateNames.append(templateName);
    }

    return templateNames;
}
