#include <gtest/gtest.h>

#include "model/entities/helpers/fcm_copy.h"
#include "model/prediction/prediction_parameters.h"

namespace {

}

TEST(FcmCopyTest, NullInputReturnsNullptr) {
    EXPECT_EQ(cloneFCMForRuntime(nullptr), nullptr);
}

TEST(FcmCopyTest, CreatesDeepCopy) {
    const QUuid nodeTermId("{11111111-1111-1111-1111-111111111111}");
    const QUuid edgeTermId("{22222222-2222-2222-2222-222222222222}");
    const QUuid conceptId("{33333333-3333-3333-3333-333333333333}");
    const QUuid otherConceptId("{44444444-4444-4444-4444-444444444444}");
    const QUuid weightId("{55555555-5555-5555-5555-555555555555}");

    auto nodeTerm = std::make_shared<Term>();
    nodeTerm->id = nodeTermId;
    nodeTerm->name = "Node term";
    nodeTerm->description = "Node term description";
    nodeTerm->value = 0.2;
    nodeTerm->fuzzyValue = {0.1, 0.2, 0.3};
    nodeTerm->type = ElementType::Node;
    nodeTerm->color = QColor(10, 20, 30, 255);

    auto edgeTerm = std::make_shared<Term>();
    edgeTerm->id = edgeTermId;
    edgeTerm->name = "Edge term";
    edgeTerm->description = "Edge term description";
    edgeTerm->value = 0.8;
    edgeTerm->fuzzyValue = {0.7, 0.8, 0.9};
    edgeTerm->type = ElementType::Edge;
    edgeTerm->color = QColor(10, 20, 30, 255);

    auto concept = std::make_shared<Concept>();
    concept->id = conceptId;
    concept->name = "Source";
    concept->description = "Source description";
    concept->term = nodeTerm;
    concept->startStep = 2;
    concept->pos = QPointF(1.0, 2.0);
    concept->nameLocation = ConceptNameLocation::BottomRight;

    auto otherConcept = std::make_shared<Concept>();
    otherConcept->id = otherConceptId;
    otherConcept->name = "Target";
    otherConcept->description = "Target description";
    otherConcept->term = nullptr;
    otherConcept->startStep = 0;
    otherConcept->pos = QPointF(-1.0, 5.0);
    otherConcept->nameLocation = ConceptNameLocation::Center;

    auto weight = std::make_shared<Weight>();
    weight->id = weightId;
    weight->name = "Connection";
    weight->description = "Connection description";
    weight->term = edgeTerm;
    weight->fromConceptId = conceptId;
    weight->toConceptId = otherConceptId;

    auto original = std::make_shared<FCM>();
    original->name = "Original";
    original->description = "Runtime copy source";
    original->predictionParameters = PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0};
    original->autosaveOn = true;
    original->dbId = 42;
    original->deletedTermsIds = {1};
    original->deletedConceptsIds = {2};
    original->deletedWeightsIds = {3};
    original->deletedExperimentsIds = {4};
    original->terms.emplace(nodeTermId, nodeTerm);
    original->terms.emplace(edgeTermId, edgeTerm);
    original->concepts.emplace(conceptId, concept);
    original->concepts.emplace(otherConceptId, otherConcept);
    original->weights.emplace(weightId, weight);

    const auto clone = cloneFCMForRuntime(original);

    ASSERT_NE(clone, nullptr);
    EXPECT_NE(clone.get(), original.get());
    EXPECT_EQ(clone->name, original->name);
    EXPECT_EQ(clone->description, original->description);
    EXPECT_EQ(clone->predictionParameters, original->predictionParameters);
    EXPECT_EQ(clone->autosaveOn, original->autosaveOn);
    EXPECT_EQ(clone->dbId, original->dbId);
    EXPECT_EQ(clone->deletedTermsIds, original->deletedTermsIds);
    EXPECT_TRUE(clone->experiments.empty());

    const auto originalConcept = original->concepts.begin()->second;
    const auto clonedConcept = clone->concepts.begin()->second;
    ASSERT_NE(clonedConcept, nullptr);
    EXPECT_NE(clonedConcept.get(), originalConcept.get());
    ASSERT_NE(clonedConcept->term, nullptr);
    EXPECT_NE(clonedConcept->term.get(), originalConcept->term.get());
    EXPECT_EQ(clonedConcept->term, clone->terms.at(originalConcept->term->id));

    const auto originalWeight = original->weights.begin()->second;
    const auto clonedWeight = clone->weights.begin()->second;
    ASSERT_NE(clonedWeight, nullptr);
    ASSERT_NE(clonedWeight->term, nullptr);
    EXPECT_NE(clonedWeight->term.get(), originalWeight->term.get());
    EXPECT_EQ(clonedWeight->term, clone->terms.at(originalWeight->term->id));

    originalConcept->term->value = 0.99;
    originalWeight->term->name = "Changed";
    EXPECT_DOUBLE_EQ(clonedConcept->term->value, 0.2);
    EXPECT_EQ(clonedWeight->term->name, "Edge term");
}
