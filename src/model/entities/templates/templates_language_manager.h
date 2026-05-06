#pragma once

#include "template_type.h"

#include <QList>
#include <QPair>
#include <QSettings>
#include <QStringList>

class TemplatesLanguageManager {
public:
    static TemplateType currentDefaultTemplateType(const QSettings &settings);
    static QStringList filterTemplateNamesForCurrentLanguage(
        const QList<QPair<QString, TemplateType>> &templatesNamesWithTypes,
        const QSettings &settings
    );
    static QStringList extractTemplateNames(
        const QList<QPair<QString, TemplateType>> &templatesNamesWithTypes
    );
};
