#include "fcm_copy.h"

namespace {
std::map<QUuid, std::shared_ptr<Term>> cloneTerms(const std::map<QUuid, std::shared_ptr<Term>>& terms) {
    std::map<QUuid, std::shared_ptr<Term>> clonedTerms;
    for (const auto& [id, term] : terms) {
        clonedTerms[id] = std::make_shared<Term>(*term);
    }
    return clonedTerms;
}

std::map<QUuid, std::shared_ptr<Concept>> cloneConcepts(
    const std::map<QUuid, std::shared_ptr<Concept>>& concepts,
    const std::map<QUuid, std::shared_ptr<Term>>& clonedTerms
) {
    std::map<QUuid, std::shared_ptr<Concept>> clonedConcepts;
    for (const auto& [id, concept] : concepts) {
        auto clonedConcept = std::make_shared<Concept>(*concept);
        if (concept->term) {
            auto termIt = clonedTerms.find(concept->term->id);
            clonedConcept->term = termIt != clonedTerms.end() ? termIt->second : nullptr;
        } else {
            clonedConcept->term = nullptr;
        }
        clonedConcepts[id] = clonedConcept;
    }
    return clonedConcepts;
}

std::map<QUuid, std::shared_ptr<Weight>> cloneWeights(
    const std::map<QUuid, std::shared_ptr<Weight>>& weights,
    const std::map<QUuid, std::shared_ptr<Term>>& clonedTerms
) {
    std::map<QUuid, std::shared_ptr<Weight>> clonedWeights;
    for (const auto& [id, weight] : weights) {
        auto clonedWeight = std::make_shared<Weight>(*weight);
        if (weight->term) {
            auto termIt = clonedTerms.find(weight->term->id);
            clonedWeight->term = termIt != clonedTerms.end() ? termIt->second : nullptr;
        } else {
            clonedWeight->term = nullptr;
        }
        clonedWeights[id] = clonedWeight;
    }
    return clonedWeights;
}
}

std::shared_ptr<FCM> cloneFCMForRuntime(const std::shared_ptr<FCM>& fcm) {
    if (!fcm) {
        return {};
    }

    auto clonedFcm = std::make_shared<FCM>();
    clonedFcm->name = fcm->name;
    clonedFcm->description = fcm->description;
    clonedFcm->predictionParameters = fcm->predictionParameters;
    clonedFcm->autosaveOn = fcm->autosaveOn;
    clonedFcm->dbId = fcm->dbId;
    clonedFcm->deletedTermsIds = fcm->deletedTermsIds;
    clonedFcm->deletedConceptsIds = fcm->deletedConceptsIds;
    clonedFcm->deletedWeightsIds = fcm->deletedWeightsIds;
    clonedFcm->deletedExperimentsIds = fcm->deletedExperimentsIds;

    clonedFcm->terms = cloneTerms(fcm->terms);
    clonedFcm->concepts = cloneConcepts(fcm->concepts, clonedFcm->terms);
    clonedFcm->weights = cloneWeights(fcm->weights, clonedFcm->terms);

    return clonedFcm;
}
