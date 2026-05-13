#pragma once

#include "template_type.h"

#include <QList>
#include <QPair>
#include <QStringList>

class TemplatesLanguageManager {
public:
    static TemplateType currentDefaultTemplateType();
    static QStringList filterTemplateNamesForCurrentLanguage(
        const QList<QPair<QString, TemplateType>> &templatesNamesWithTypes
    );
    static QStringList extractTemplateNames(
        const QList<QPair<QString, TemplateType>> &templatesNamesWithTypes
    );
};
