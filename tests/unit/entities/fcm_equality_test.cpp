#include <gtest/gtest.h>

#include "model/entities/fcm.h"

#include <memory>

namespace {

FCM makeModel() {
    const auto termId = QUuid::createUuid();
    const auto conceptId = QUuid::createUuid();
    const auto weightId = QUuid::createUuid();

    auto term = std::make_shared<Term>();
    term->id = termId;
    term->name = "term";
    term->description = "description";
    term->value = 2.0;
    term->fuzzyValue = {1.0, 2.0, 3.0};
    term->color = QColor(Qt::red);
    term->type = ElementType::Node;

    auto concept = std::make_shared<Concept>();
    concept->id = conceptId;
    concept->name = "concept";
    concept->description = "description";
    concept->term = term;
    concept->pos = QPointF(1.0, 2.0);
    concept->startStep = 3;

    auto weight = std::make_shared<Weight>();
    weight->id = weightId;
    weight->name = "weight";
    weight->description = "description";
    weight->term = term;
    weight->fromConceptId = conceptId;
    weight->toConceptId = conceptId;

    FCM fcm;
    fcm.name = "model";
    fcm.description = "description";
    fcm.terms.emplace(termId, term);
    fcm.concepts.emplace(conceptId, concept);
    fcm.weights.emplace(weightId, weight);
    fcm.predictionParameters.algorithm = "standard";
    fcm.predictionParameters.fixedSteps = 5;
    Experiment experiment;
    experiment.terms.emplace(termId, term);
    experiment.concepts.emplace(conceptId, concept);
    experiment.weights.emplace(weightId, weight);
    experiment.predictionParameters.algorithm = "standard";
    experiment.predictionParameters.fixedSteps = 5;
    experiment.timestamp = QDateTime(QDate(2026, 1, 1), QTime(12, 0));
    experiment.dbId = 42;
    fcm.experiments.push_back(experiment);
    fcm.autosaveOn = true;
    fcm.dbId = 7;
    fcm.deletedTermsIds.append(1);
    fcm.deletedConceptsIds.append(2);
    fcm.deletedWeightsIds.append(3);
    fcm.deletedExperimentsIds.append(4);
    return fcm;
}

}

TEST(FcmEqualityTest, IgnoresRuntimeOnlyFields) {
    FCM lhs = makeModel();
    FCM rhs = lhs;

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
    Experiment experiment;
    experiment.terms.emplace(rhsTerm->id, rhsTerm);
    experiment.concepts.emplace(rhsConcept->id, rhsConcept);
    experiment.weights.emplace(rhsWeight->id, rhsWeight);
    experiment.predictionParameters.algorithm = "standard";
    experiment.predictionParameters.fixedSteps = 5;
    experiment.timestamp = QDateTime(QDate(2026, 1, 1), QTime(12, 0));
    experiment.dbId = 42;
    rhs.experiments.front() = experiment;
    rhs.experiments.front().dbId = 123;

    EXPECT_EQ(lhs, rhs);
    EXPECT_FALSE(lhs != rhs);
}

TEST(FcmEqualityTest, DetectsFieldChanges) {
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
    rhs.autosaveOn = false;
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
