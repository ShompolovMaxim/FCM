#include "templates_manager.h"

#include "common/logger.h"

#include <memory>

TemplatesManager::TemplatesManager(TemplatesRepository repository) : repo(repository) {}

bool TemplatesManager::createTemplate(const FCM &fcm) {
    if (!repo.transaction()) {
        Logger::warn("Template transaction failed");
        return false;
    }

    auto templateModel = toTemplate(fcm);
    if (!repo.createTemplate(templateModel)) {
        Logger::warn("Template create failed");
        repo.rollback();
        return false;
    }

    if (!repo.commit()) {
        Logger::warn("Template commit failed");
        repo.rollback();
        return false;
    }

    return true;
}

std::optional<FCM> TemplatesManager::getFCM(const QString &templateName) {
    auto templateOpt = repo.getTemplate(templateName);
    if (!templateOpt) {
        Logger::warn("Template load failed");
        return {};
    }

    return toFCM(*templateOpt);
}

QList<QPair<QString, TemplateType>> TemplatesManager::getTemplatesNames() {
    return repo.getTemplatesNames();
}

bool TemplatesManager::deleteTemplate(const QString &templateName) {
    if (!repo.transaction()) {
        Logger::warn("Template transaction failed");
        return false;
    }

    if (!repo.deleteTemplate(templateName)) {
        Logger::warn("Template delete failed");
        repo.rollback();
        return false;
    }

    if (!repo.commit()) {
        Logger::warn("Template commit failed");
        repo.rollback();
        return false;
    }

    return true;
}

Template TemplatesManager::toTemplate(const FCM &fcm) {
    Template templateModel;
    templateModel.name = fcm.name;
    templateModel.description = fcm.description;

    std::map<QUuid, std::shared_ptr<TemplateConcept>> conceptsById;

    for (const auto &[_, term] : fcm.terms) {
        auto templateTerm = std::make_shared<TemplateTerm>();
        templateTerm->name = term->name;
        templateTerm->description = term->description;
        templateTerm->value = term->value;
        templateTerm->fuzzyValue = term->fuzzyValue;
        templateTerm->color = term->color;
        templateTerm->type = term->type;
        templateModel.terms.push_back(templateTerm);
    }

    for (const auto &[conceptId, concept] : fcm.concepts) {
        auto templateConcept = std::make_shared<TemplateConcept>();
        templateConcept->name = concept->name;
        templateConcept->description = concept->description;
        templateConcept->pos = concept->pos;
        templateConcept->startStep = concept->startStep;
        templateConcept->nameLocation = concept->nameLocation;
        templateModel.concepts.push_back(templateConcept);
        conceptsById[conceptId] = templateConcept;
    }

    for (const auto &[_, weight] : fcm.weights) {
        auto fromIt = conceptsById.find(weight->fromConceptId);
        auto toIt = conceptsById.find(weight->toConceptId);
        if (fromIt == conceptsById.end() || toIt == conceptsById.end()) {
            continue;
        }

        auto templateWeight = std::make_shared<TemplateWeight>();
        templateWeight->name = weight->name;
        templateWeight->description = weight->description;
        templateWeight->fromConcept = fromIt->second;
        templateWeight->toConcept = toIt->second;
        templateModel.weights.push_back(templateWeight);
    }

    return templateModel;
}

FCM TemplatesManager::toFCM(const Template &templateModel) {
    FCM fcm;
    fcm.name = templateModel.name;
    fcm.description = templateModel.description;

    std::map<const TemplateConcept *, QUuid> conceptsIds;

    for (const auto &templateTerm : templateModel.terms) {
        auto term = std::make_shared<Term>();
        term->id = QUuid::createUuid();
        term->name = templateTerm->name;
        term->description = templateTerm->description;
        term->value = templateTerm->value;
        term->fuzzyValue = templateTerm->fuzzyValue;
        term->color = templateTerm->color;
        term->type = templateTerm->type;
        fcm.terms[term->id] = term;
    }

    for (const auto &templateConcept : templateModel.concepts) {
        auto concept = std::make_shared<Concept>();
        concept->id = QUuid::createUuid();
        concept->name = templateConcept->name;
        concept->description = templateConcept->description;
        concept->term = nullptr;
        concept->pos = templateConcept->pos;
        concept->startStep = templateConcept->startStep;
        concept->nameLocation = templateConcept->nameLocation;
        fcm.concepts[concept->id] = concept;
        conceptsIds[templateConcept.get()] = concept->id;
    }

    for (const auto &templateWeight : templateModel.weights) {
        auto fromIt = conceptsIds.find(templateWeight->fromConcept.get());
        auto toIt = conceptsIds.find(templateWeight->toConcept.get());
        if (fromIt == conceptsIds.end() || toIt == conceptsIds.end()) {
            continue;
        }

        auto weight = std::make_shared<Weight>();
        weight->id = QUuid::createUuid();
        weight->name = templateWeight->name;
        weight->description = templateWeight->description;
        weight->term = nullptr;
        weight->fromConceptId = fromIt->second;
        weight->toConceptId = toIt->second;
        fcm.weights[weight->id] = weight;
    }

    return fcm;
}

