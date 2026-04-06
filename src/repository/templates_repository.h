#pragma once

#include "model/entities/templates/template.h"

#include <map>
#include <optional>
#include <vector>

#include <QList>
#include <QSqlDatabase>

class TemplatesRepository {
public:
    explicit TemplatesRepository(QSqlDatabase database);

    bool transaction();
    bool commit();
    bool rollback();

    QList<QString> getTemplatesNames();

    std::optional<Template> getTemplate(const QString &templateName);
    std::optional<int> createTemplate(Template &templateModel);
    bool deleteTemplate(const QString &templateName);

private:
    std::optional<int> getTemplateId(const QString &templateName);
    std::optional<std::vector<std::shared_ptr<TemplateTerm>>> getTemplateTerms(int templateId);
    std::optional<std::vector<std::shared_ptr<TemplateConcept>>> getTemplateConcepts(
        int templateId,
        std::map<int, std::shared_ptr<TemplateConcept>> &conceptsByDbId
    );
    std::optional<std::vector<std::shared_ptr<TemplateWeight>>> getTemplateWeights(
        int templateId,
        const std::map<int, std::shared_ptr<TemplateConcept>> &conceptsByDbId
    );

    std::optional<int> createTemplateTerm(TemplateTerm &term, int templateId);
    std::optional<int> createTemplateConcept(TemplateConcept &concept, int templateId);
    std::optional<int> createTemplateWeight(TemplateWeight &weight, int templateId, int fromConceptId, int toConceptId);

    bool deleteTemplateTerms(int templateId);
    bool deleteTemplateConcepts(int templateId);
    bool deleteTemplateWeights(int templateId);

    QSqlDatabase db;
};
