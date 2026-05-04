#include <gtest/gtest.h>

#include "model/entities/fcm.h"

#include <memory>

namespace {

std::shared_ptr<Term> makeTerm(const QUuid& id, double value, const QString& name = "term") {
    auto term = std::make_shared<Term>();
    term->id = id;
    term->name = name;
    term->description = "description";
    term->value = value;
    term->fuzzyValue = {value - 1.0, value, value + 1.0};
    term->color = QColor(Qt::red);
    term->type = ElementType::Node;
    return term;
}

std::shared_ptr<Concept> makeConcept(const QUuid& id, const std::shared_ptr<Term>& term) {
    auto concept = std::make_shared<Concept>();
    concept->id = id;
    concept->name = "concept";
    concept->description = "description";
    concept->term = term;
    concept->pos = QPointF(1.0, 2.0);
    concept->startStep = 3;
    return concept;
}

std::shared_ptr<Weight> makeWeight(
    const QUuid& id,
    const std::shared_ptr<Term>& term,
    const QUuid& fromConceptId,
    const QUuid& toConceptId
) {
    auto weight = std::make_shared<Weight>();
    weight->id = id;
    weight->name = "weight";
    weight->description = "description";
    weight->term = term;
    weight->fromConceptId = fromConceptId;
    weight->toConceptId = toConceptId;
    return weight;
}

Experiment makeExperiment(
    const std::shared_ptr<Term>& term,
    const std::shared_ptr<Concept>& concept,
    const std::shared_ptr<Weight>& weight,
    const QDateTime& timestamp = QDateTime(QDate(2026, 1, 1), QTime(12, 0))
) {
    Experiment experiment;
    experiment.terms.emplace(term->id, term);
    experiment.concepts.emplace(concept->id, concept);
    experiment.weights.emplace(weight->id, weight);
    experiment.predictionParameters.algorithm = "standard";
    experiment.predictionParameters.fixedSteps = 5;
    experiment.timestamp = timestamp;
    experiment.dbId = 42;
    return experiment;
}

FCM makeModel() {
    const auto termId = QUuid::createUuid();
    const auto conceptId = QUuid::createUuid();
    const auto weightId = QUuid::createUuid();

    auto term = makeTerm(termId, 2.0);
    auto concept = makeConcept(conceptId, term);
    auto weight = makeWeight(weightId, term, conceptId, conceptId);

    FCM fcm;
    fcm.name = "model";
    fcm.description = "description";
    fcm.terms.emplace(termId, term);
    fcm.concepts.emplace(conceptId, concept);
    fcm.weights.emplace(weightId, weight);
    fcm.predictionParameters.algorithm = "standard";
    fcm.predictionParameters.fixedSteps = 5;
    fcm.experiments.push_back(makeExperiment(term, concept, weight));
    fcm.autosaveOn = true;
    fcm.dbId = 7;
    fcm.deletedTermsIds.append(1);
    fcm.deletedConceptsIds.append(2);
    fcm.deletedWeightsIds.append(3);
    fcm.deletedExperimentsIds.append(4);
    return fcm;
}

}

TEST(FcmEqualityTest, IgnoresServiceFieldsAndPointerIdentity) {
    FCM lhs = makeModel();
    FCM rhs = lhs;

    rhs.autosaveOn = false;
    rhs.dbId = 999;
    rhs.deletedTermsIds.clear();
    rhs.deletedConceptsIds.clear();
    rhs.deletedWeightsIds.clear();
    rhs.deletedExperimentsIds.clear();

    auto rhsTerm = std::make_shared<Term>(*rhs.terms.begin()->second);
    auto rhsConcept = std::make_shared<Concept>(*rhs.concepts.begin()->second);
    auto rhsWeight = std::make_shared<Weight>(*rhs.weights.begin()->second);
    rhsConcept->term = rhsTerm;
    rhsWeight->term = rhsTerm;

    rhs.terms.begin()->second = rhsTerm;
    rhs.concepts.begin()->second = rhsConcept;
    rhs.weights.begin()->second = rhsWeight;
    rhs.experiments.front() = makeExperiment(rhsTerm, rhsConcept, rhsWeight);
    rhs.experiments.front().dbId = 123;

    EXPECT_EQ(lhs, rhs);
    EXPECT_FALSE(lhs != rhs);
}

TEST(FcmEqualityTest, DetectsDifferenceInComparedFields) {
    FCM lhs = makeModel();
    FCM rhs = lhs;

    rhs.predictionParameters.fixedSteps = 10;
    EXPECT_NE(lhs, rhs);

    rhs = lhs;
    rhs.name = "other";
    EXPECT_NE(lhs, rhs);

    rhs = lhs;
    rhs.description = "other";
    EXPECT_NE(lhs, rhs);

    rhs = lhs;
    rhs.experiments.front().timestamp = rhs.experiments.front().timestamp.addDays(1);
    EXPECT_NE(lhs, rhs);

    rhs = lhs;
    auto changedTerm = std::make_shared<Term>(*rhs.terms.begin()->second);
    changedTerm->value = 999.0;
    rhs.terms.begin()->second = changedTerm;
    EXPECT_NE(lhs, rhs);
}
