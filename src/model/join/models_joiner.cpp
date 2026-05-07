#include "models_joiner.h"

#include "common/crash_log.h"

ModelsJoiner::ModelsJoiner() {}

std::shared_ptr<FCM> ModelsJoiner::join(
    const std::shared_ptr<FCM> baseFCM,
    const std::vector<std::shared_ptr<FCM>>& fcms,
    const JoinMode& joinMode,
    const QString& resultName
) {
    if (!baseFCM) {
        Logger::warn("Join base model missing");
        return std::make_shared<FCM>();
    }
    if (!fuzzifier) {
        Logger::warn("Join fuzzifier missing");
        return std::make_shared<FCM>();
    }

    auto result = std::make_shared<FCM>();
    result->name = resultName;
    result->description = baseFCM->description;
    for (const auto& [id, term] : baseFCM->terms) {
        result->terms[id] = std::make_shared<Term>(*term);
    }

    std::map<QString, double> conceptsNumericSum;
    std::map<std::pair<QString, QString>, double> weightsNumericSum;
    std::map<QString, TriangularFuzzyValue> conceptsFuzzySum;
    std::map<std::pair<QString, QString>, TriangularFuzzyValue> weightsFuzzySum;
    std::map<QString, size_t> conceptsCount;
    std::map<std::pair<QString, QString>, size_t> weightsCount;
    std::map<QString, QString> conceptsDescription;
    std::map<std::pair<QString, QString>, QString> weightsDescription;
    std::map<QString, size_t> conceptsStartStepSum;
    std::map<QString, QPointF> conceptsPos;
    std::map<QString, ConceptNameLocation> conceptsNameLocation;
    std::map<std::pair<QString, QString>, QString> weightsName;
    for (const auto& fcm : fcms) {
        for (const auto& [_, concept] : fcm->concepts) {
            conceptsCount[concept->name];
            if (conceptsCount[concept->name] == 0) {
                conceptsDescription[concept->name] = concept->description;
                conceptsPos[concept->name] = concept->pos;
                conceptsNameLocation[concept->name] = concept->nameLocation;
            }
            if (concept->term) {
                ++conceptsCount[concept->name];
                conceptsNumericSum[concept->name] += concept->term->value;
                conceptsFuzzySum[concept->name] += concept->term->fuzzyValue;
            }
            conceptsStartStepSum[concept->name] += concept->startStep;
        }
        for (const auto& [_, weight] : fcm->weights) {
            const auto fromIt = fcm->concepts.find(weight->fromConceptId);
            const auto toIt = fcm->concepts.find(weight->toConceptId);
            if (fromIt == fcm->concepts.end() || toIt == fcm->concepts.end()) {
                Logger::warn("Join concept missing");
                continue;
            }
            auto fromName = fromIt->second->name;
            auto toName = toIt->second->name;
            weightsCount[{fromName, toName}];
            if (weightsCount[{fromName, toName}] == 0) {
                weightsDescription[{fromName, toName}] = weight->description;
                weightsName[{fromName, toName}] = weight->name;
            }
            if (weight->term) {
                ++weightsCount[{fromName, toName}];
                weightsNumericSum[{fromName, toName}] += weight->term->value;
                weightsFuzzySum[{fromName, toName}] += weight->term->fuzzyValue;
            }
        }
    }

    std::map<QUuid, std::shared_ptr<Term>> conceptsTerms;
    std::map<QUuid, std::shared_ptr<Term>> weightsTerms;
    for (auto [id, term] : result->terms) {
        if (term->type == ElementType::Node) {
            conceptsTerms[id] = term;
        } else {
            weightsTerms[id] = term;
        }
    }

    std::map<QString, QUuid> conceptsIds;
    for (const auto& [name, count] : conceptsCount) {
        auto id = QUuid::createUuid();
        conceptsIds[name] = id;
        std::shared_ptr<Term> term = {};
        if (count) {
            double newNumeric = conceptsNumericSum[name] / count;
            TriangularFuzzyValue newFuzzy = {
                conceptsFuzzySum[name].l / count,
                conceptsFuzzySum[name].m / count,
                conceptsFuzzySum[name].u / count
            };
            if (joinMode == JoinMode::Numeric) {
                term = fuzzifier->fuzzify(conceptsTerms, newNumeric);
            }
            if (joinMode == JoinMode::Fuzzy) {
                term = fuzzifier->fuzzify(conceptsTerms, newFuzzy);
            }
            if (joinMode == JoinMode::Gibrid) {
                term = fuzzifier->fuzzify(conceptsTerms, newNumeric, newFuzzy);
            }
        }
        result->concepts[id] = std::make_shared<Concept>(Concept{
            id,
            name,
            conceptsDescription[name],
            term,
            conceptsPos[name],
            count ? conceptsStartStepSum[name] / count : 0,
            conceptsNameLocation[name]
        });
    }

    for (const auto& [concepts, count] : weightsCount) {
        auto id = QUuid::createUuid();
        std::shared_ptr<Term> term = {};
        if (count) {
            double newNumeric = weightsNumericSum[concepts] / count;
            TriangularFuzzyValue newFuzzy = {
                weightsFuzzySum[concepts].l / count,
                weightsFuzzySum[concepts].m / count,
                weightsFuzzySum[concepts].u / count
            };
            if (joinMode == JoinMode::Numeric) {
                term = fuzzifier->fuzzify(weightsTerms, newNumeric);
            }
            if (joinMode == JoinMode::Fuzzy) {
                term = fuzzifier->fuzzify(weightsTerms, newFuzzy);
            }
            if (joinMode == JoinMode::Gibrid) {
                term = fuzzifier->fuzzify(weightsTerms, newNumeric, newFuzzy);
            }
        }
        result->weights[id] = std::make_shared<Weight>(Weight{
            id,
            weightsName[concepts],
            weightsDescription[concepts],
            term,
            conceptsIds[concepts.first],
            conceptsIds[concepts.second],
        });
    }

    return result;
}

