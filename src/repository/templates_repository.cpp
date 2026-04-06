#include "templates_repository.h"

#include "model/element_type.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

TemplatesRepository::TemplatesRepository(QSqlDatabase database) : db(database) {}

bool TemplatesRepository::transaction() {
    return db.transaction();
}

bool TemplatesRepository::commit() {
    return db.commit();
}

bool TemplatesRepository::rollback() {
    return db.rollback();
}

QList<QString> TemplatesRepository::getTemplatesNames() {
    QStringList templateNames;
    QSqlQuery query(db);
    if (!query.exec("SELECT name FROM templates")) return templateNames;
    while (query.next()) templateNames.append(query.value(0).toString());
    return templateNames;
}

std::optional<int> TemplatesRepository::getTemplateId(const QString &templateName) {
    QSqlQuery query(db);
    query.prepare("SELECT id FROM templates WHERE name=:name");
    query.bindValue(":name", templateName);
    if (!query.exec() || !query.next()) return {};
    return query.value("id").toInt();
}

std::optional<Template> TemplatesRepository::getTemplate(const QString &templateName) {
    QSqlQuery query(db);
    query.prepare("SELECT name,description FROM templates WHERE name=:name");
    query.bindValue(":name", templateName);
    if (!query.exec() || !query.next()) return {};

    Template templateModel;
    templateModel.name = query.value("name").toString();
    templateModel.description = query.value("description").toString();

    auto templateIdOpt = getTemplateId(templateName);
    if (!templateIdOpt) return {};
    int templateId = *templateIdOpt;

    auto termsOpt = getTemplateTerms(templateId);
    if (!termsOpt) return {};
    templateModel.terms = *termsOpt;

    std::map<int, std::shared_ptr<TemplateConcept>> conceptsByDbId;
    auto conceptsOpt = getTemplateConcepts(templateId, conceptsByDbId);
    if (!conceptsOpt) return {};
    templateModel.concepts = *conceptsOpt;

    auto weightsOpt = getTemplateWeights(templateId, conceptsByDbId);
    if (!weightsOpt) return {};
    templateModel.weights = *weightsOpt;

    return templateModel;
}

std::optional<int> TemplatesRepository::createTemplate(Template &templateModel) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO templates (name,description) VALUES (:name,:description)");
    query.bindValue(":name", templateModel.name);
    query.bindValue(":description", templateModel.description);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }

    int templateId = query.lastInsertId().toInt();

    for (const auto &term : templateModel.terms) {
        if (!createTemplateTerm(*term, templateId)) return {};
    }

    std::map<const TemplateConcept *, int> conceptsIds;
    for (const auto &concept : templateModel.concepts) {
        auto idOpt = createTemplateConcept(*concept, templateId);
        if (!idOpt) return {};
        conceptsIds[concept.get()] = *idOpt;
    }

    for (const auto &weight : templateModel.weights) {
        if (!weight->fromConcept || !weight->toConcept) return {};

        auto fromIt = conceptsIds.find(weight->fromConcept.get());
        auto toIt = conceptsIds.find(weight->toConcept.get());
        if (fromIt == conceptsIds.end() || toIt == conceptsIds.end()) return {};

        if (!createTemplateWeight(*weight, templateId, fromIt->second, toIt->second)) return {};
    }

    return templateId;
}

bool TemplatesRepository::deleteTemplate(const QString &templateName) {
    auto templateIdOpt = getTemplateId(templateName);
    if (!templateIdOpt) return false;
    int templateId = *templateIdOpt;

    if (!deleteTemplateWeights(templateId)) return false;
    if (!deleteTemplateConcepts(templateId)) return false;
    if (!deleteTemplateTerms(templateId)) return false;

    QSqlQuery query(db);
    query.prepare("DELETE FROM templates WHERE id=:id");
    query.bindValue(":id", templateId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    return true;
}

std::optional<std::vector<std::shared_ptr<TemplateTerm>>> TemplatesRepository::getTemplateTerms(int templateId) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT "
        "id,name,description,numeric_value,tr_value_l,tr_value_m,tr_value_h,"
        "color_r,color_g,color_b,type "
        "FROM templates_terms WHERE template_id=:template_id"
    );
    query.bindValue(":template_id", templateId);
    if (!query.exec()) return {};

    std::vector<std::shared_ptr<TemplateTerm>> result;
    while (query.next()) {
        auto term = std::make_shared<TemplateTerm>();
        term->name = query.value("name").toString();
        term->description = query.value("description").toString();
        term->value = query.value("numeric_value").toDouble();
        term->fuzzyValue.l = query.value("tr_value_l").toDouble();
        term->fuzzyValue.m = query.value("tr_value_m").toDouble();
        term->fuzzyValue.u = query.value("tr_value_h").toDouble();
        term->color = QColor(
            query.value("color_r").toInt(),
            query.value("color_g").toInt(),
            query.value("color_b").toInt()
        );
        term->type = elementTypeFromString(query.value("type").toString());
        result.push_back(term);
    }

    return result;
}

std::optional<std::vector<std::shared_ptr<TemplateConcept>>> TemplatesRepository::getTemplateConcepts(
    int templateId,
    std::map<int, std::shared_ptr<TemplateConcept>> &conceptsByDbId
) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id,name,description,first_step,x_pos,y_pos "
        "FROM templates_concepts WHERE template_id=:template_id"
    );
    query.bindValue(":template_id", templateId);
    if (!query.exec()) return {};

    std::vector<std::shared_ptr<TemplateConcept>> result;
    while (query.next()) {
        auto concept = std::make_shared<TemplateConcept>();
        int conceptDbId = query.value("id").toInt();
        concept->name = query.value("name").toString();
        concept->description = query.value("description").toString();
        concept->startStep = static_cast<size_t>(query.value("first_step").toUInt());
        concept->pos = QPointF(query.value("x_pos").toDouble(), query.value("y_pos").toDouble());
        conceptsByDbId[conceptDbId] = concept;
        result.push_back(concept);
    }

    return result;
}

std::optional<std::vector<std::shared_ptr<TemplateWeight>>> TemplatesRepository::getTemplateWeights(
    int templateId,
    const std::map<int, std::shared_ptr<TemplateConcept>> &conceptsByDbId
) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id,name,description,concept_from_id,concept_to_id "
        "FROM templates_weights WHERE template_id=:template_id"
    );
    query.bindValue(":template_id", templateId);
    if (!query.exec()) return {};

    std::vector<std::shared_ptr<TemplateWeight>> result;
    while (query.next()) {
        auto fromIt = conceptsByDbId.find(query.value("concept_from_id").toInt());
        auto toIt = conceptsByDbId.find(query.value("concept_to_id").toInt());
        if (fromIt == conceptsByDbId.end() || toIt == conceptsByDbId.end()) return {};

        auto weight = std::make_shared<TemplateWeight>();
        weight->name = query.value("name").toString();
        weight->description = query.value("description").toString();
        weight->fromConcept = fromIt->second;
        weight->toConcept = toIt->second;
        result.push_back(weight);
    }

    return result;
}

std::optional<int> TemplatesRepository::createTemplateTerm(TemplateTerm &term, int templateId) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO templates_terms "
        "(template_id,name,description,numeric_value,tr_value_l,tr_value_m,tr_value_h,"
        "color_r,color_g,color_b,type) "
        "VALUES (:template_id,:name,:description,:numeric_value,:tr_value_l,:tr_value_m,"
        ":tr_value_h,:color_r,:color_g,:color_b,:type)"
    );
    query.bindValue(":template_id", templateId);
    query.bindValue(":name", term.name);
    query.bindValue(":description", term.description);
    query.bindValue(":numeric_value", term.value);
    query.bindValue(":tr_value_l", term.fuzzyValue.l);
    query.bindValue(":tr_value_m", term.fuzzyValue.m);
    query.bindValue(":tr_value_h", term.fuzzyValue.u);
    query.bindValue(":color_r", term.color.red());
    query.bindValue(":color_g", term.color.green());
    query.bindValue(":color_b", term.color.blue());
    query.bindValue(":type", elementTypeToString(term.type));
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }

    return query.lastInsertId().toInt();
}

std::optional<int> TemplatesRepository::createTemplateConcept(TemplateConcept &concept, int templateId) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO templates_concepts "
        "(template_id,name,description,first_step,x_pos,y_pos) "
        "VALUES (:template_id,:name,:description,:first_step,:x_pos,:y_pos)"
    );
    query.bindValue(":template_id", templateId);
    query.bindValue(":name", concept.name);
    query.bindValue(":description", concept.description);
    query.bindValue(":first_step", static_cast<qulonglong>(concept.startStep));
    query.bindValue(":x_pos", concept.pos.x());
    query.bindValue(":y_pos", concept.pos.y());
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }

    return query.lastInsertId().toInt();
}

std::optional<int> TemplatesRepository::createTemplateWeight(TemplateWeight &weight, int templateId, int fromConceptId, int toConceptId) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO templates_weights "
        "(template_id,concept_from_id,concept_to_id,name,description) "
        "VALUES (:template_id,:concept_from_id,:concept_to_id,:name,:description)"
    );
    query.bindValue(":template_id", templateId);
    query.bindValue(":concept_from_id", fromConceptId);
    query.bindValue(":concept_to_id", toConceptId);
    query.bindValue(":name", weight.name);
    query.bindValue(":description", weight.description);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }

    return query.lastInsertId().toInt();
}

bool TemplatesRepository::deleteTemplateTerms(int templateId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM templates_terms WHERE template_id=:template_id");
    query.bindValue(":template_id", templateId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    return true;
}

bool TemplatesRepository::deleteTemplateConcepts(int templateId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM templates_concepts WHERE template_id=:template_id");
    query.bindValue(":template_id", templateId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    return true;
}

bool TemplatesRepository::deleteTemplateWeights(int templateId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM templates_weights WHERE template_id=:template_id");
    query.bindValue(":template_id", templateId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    return true;
}
