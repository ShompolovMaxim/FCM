#include <gtest/gtest.h>

#include "model/join/models_joiner.h"

namespace {

std::shared_ptr<FCM> makeJoinedInput(double conceptA, double conceptB, double weightValue, size_t startStep) {
    const QUuid conceptAId = QUuid::createUuid();
    const QUuid conceptBId = QUuid::createUuid();
    const QUuid weightId = QUuid::createUuid();

    auto nodeTermA = std::make_shared<Term>();
    nodeTermA->id = QUuid::createUuid();
    nodeTermA->name = "A term";
    nodeTermA->description = "A term description";
    nodeTermA->value = conceptA;
    nodeTermA->fuzzyValue = {conceptA - 0.1, conceptA, conceptA + 0.1};
    nodeTermA->type = ElementType::Node;
    nodeTermA->color = QColor(10, 20, 30, 255);

    auto nodeTermB = std::make_shared<Term>();
    nodeTermB->id = QUuid::createUuid();
    nodeTermB->name = "B term";
    nodeTermB->description = "B term description";
    nodeTermB->value = conceptB;
    nodeTermB->fuzzyValue = {conceptB - 0.1, conceptB, conceptB + 0.1};
    nodeTermB->type = ElementType::Node;
    nodeTermB->color = QColor(10, 20, 30, 255);

    auto edgeTerm = std::make_shared<Term>();
    edgeTerm->id = QUuid::createUuid();
    edgeTerm->name = "Weight term";
    edgeTerm->description = "Weight term description";
    edgeTerm->value = weightValue;
    edgeTerm->fuzzyValue = {weightValue - 0.1, weightValue, weightValue + 0.1};
    edgeTerm->type = ElementType::Edge;
    edgeTerm->color = QColor(10, 20, 30, 255);

    auto conceptAEntity = std::make_shared<Concept>();
    conceptAEntity->id = conceptAId;
    conceptAEntity->name = "A";
    conceptAEntity->description = "A description";
    conceptAEntity->term = nodeTermA;
    conceptAEntity->startStep = startStep;
    conceptAEntity->pos = QPointF(5.0, 1.0);
    conceptAEntity->nameLocation = ConceptNameLocation::CenterRight;

    auto conceptBEntity = std::make_shared<Concept>();
    conceptBEntity->id = conceptBId;
    conceptBEntity->name = "B";
    conceptBEntity->description = "B description";
    conceptBEntity->term = nodeTermB;
    conceptBEntity->startStep = startStep + 2;
    conceptBEntity->pos = QPointF(-3.0, 4.0);
    conceptBEntity->nameLocation = ConceptNameLocation::BottomLeft;

    auto weight = std::make_shared<Weight>();
    weight->id = weightId;
    weight->name = "A->B";
    weight->description = "A->B description";
    weight->term = edgeTerm;
    weight->fromConceptId = conceptAId;
    weight->toConceptId = conceptBId;

    auto fcm = std::make_shared<FCM>();
    fcm->concepts.emplace(conceptAId, conceptAEntity);
    fcm->concepts.emplace(conceptBId, conceptBEntity);
    fcm->weights.emplace(weightId, weight);
    return fcm;
}

}

TEST(ModelsJoinerTest, MissingBaseReturnsEmptyModel) {
    ModelsJoiner joiner;

    const auto result = joiner.join(nullptr, {}, JoinMode::Numeric, "Result");

    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->concepts.empty());
    EXPECT_TRUE(result->weights.empty());
}

TEST(ModelsJoinerTest, JoinsAndAveragesValues) {
    ModelsJoiner joiner;
    auto base = std::make_shared<FCM>();
    base->description = "Base description";

    const QUuid nodeLowId = QUuid::createUuid();
    const QUuid nodeMidId = QUuid::createUuid();
    const QUuid nodeHighId = QUuid::createUuid();
    const QUuid edgeLowId = QUuid::createUuid();
    const QUuid edgeMidId = QUuid::createUuid();
    const QUuid edgeHighId = QUuid::createUuid();

    auto nodeLow = std::make_shared<Term>();
    nodeLow->id = nodeLowId;
    nodeLow->name = "Node low";
    nodeLow->description = "Node low description";
    nodeLow->value = 0.0;
    nodeLow->fuzzyValue = {0.0, 0.0, 0.0};
    nodeLow->type = ElementType::Node;
    nodeLow->color = QColor(10, 20, 30, 255);
    base->terms.emplace(nodeLowId, nodeLow);

    auto nodeMid = std::make_shared<Term>();
    nodeMid->id = nodeMidId;
    nodeMid->name = "Node mid";
    nodeMid->description = "Node mid description";
    nodeMid->value = 0.5;
    nodeMid->fuzzyValue = {0.4, 0.5, 0.6};
    nodeMid->type = ElementType::Node;
    nodeMid->color = QColor(10, 20, 30, 255);
    base->terms.emplace(nodeMidId, nodeMid);

    auto nodeHigh = std::make_shared<Term>();
    nodeHigh->id = nodeHighId;
    nodeHigh->name = "Node high";
    nodeHigh->description = "Node high description";
    nodeHigh->value = 1.0;
    nodeHigh->fuzzyValue = {0.9, 1.0, 1.0};
    nodeHigh->type = ElementType::Node;
    nodeHigh->color = QColor(10, 20, 30, 255);
    base->terms.emplace(nodeHighId, nodeHigh);

    auto edgeLow = std::make_shared<Term>();
    edgeLow->id = edgeLowId;
    edgeLow->name = "Edge low";
    edgeLow->description = "Edge low description";
    edgeLow->value = -1.0;
    edgeLow->fuzzyValue = {-1.0, -1.0, -0.8};
    edgeLow->type = ElementType::Edge;
    edgeLow->color = QColor(10, 20, 30, 255);
    base->terms.emplace(edgeLowId, edgeLow);

    auto edgeMid = std::make_shared<Term>();
    edgeMid->id = edgeMidId;
    edgeMid->name = "Edge mid";
    edgeMid->description = "Edge mid description";
    edgeMid->value = 0.1;
    edgeMid->fuzzyValue = {0.0, 0.1, 0.2};
    edgeMid->type = ElementType::Edge;
    edgeMid->color = QColor(10, 20, 30, 255);
    base->terms.emplace(edgeMidId, edgeMid);

    auto edgeHigh = std::make_shared<Term>();
    edgeHigh->id = edgeHighId;
    edgeHigh->name = "Edge high";
    edgeHigh->description = "Edge high description";
    edgeHigh->value = 0.9;
    edgeHigh->fuzzyValue = {0.8, 0.9, 1.0};
    edgeHigh->type = ElementType::Edge;
    edgeHigh->color = QColor(10, 20, 30, 255);
    base->terms.emplace(edgeHighId, edgeHigh);
    const std::vector<std::shared_ptr<FCM>> models = {
        makeJoinedInput(0.1, 0.9, 0.0, 2),
        makeJoinedInput(0.8, 0.2, 0.3, 4)
    };

    const auto result = joiner.join(base, models, JoinMode::Numeric, "Joined");

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "Joined");
    EXPECT_EQ(result->description, "Base description");
    EXPECT_EQ(result->terms.size(), base->terms.size());
    ASSERT_EQ(result->concepts.size(), 2U);
    ASSERT_EQ(result->weights.size(), 1U);

    std::shared_ptr<Concept> conceptA;
    std::shared_ptr<Concept> conceptB;
    for (const auto& [_, concept] : result->concepts) {
        if (concept->name == "A") {
            conceptA = concept;
        } else if (concept->name == "B") {
            conceptB = concept;
        }
    }

    ASSERT_NE(conceptA, nullptr);
    ASSERT_NE(conceptB, nullptr);
    ASSERT_NE(conceptA->term, nullptr);
    ASSERT_NE(conceptB->term, nullptr);
    EXPECT_EQ(conceptA->term->name, "Node mid");
    EXPECT_EQ(conceptB->term->name, "Node mid");
    EXPECT_EQ(conceptA->startStep, 3U);
    EXPECT_EQ(conceptB->startStep, 5U);
    EXPECT_EQ(conceptA->pos, QPointF(5.0, 1.0));
    EXPECT_EQ(conceptB->nameLocation, ConceptNameLocation::BottomLeft);

    const auto weight = result->weights.begin()->second;
    ASSERT_NE(weight, nullptr);
    ASSERT_NE(weight->term, nullptr);
    EXPECT_EQ(weight->term->name, "Edge mid");
    EXPECT_EQ(weight->fromConceptId, conceptA->id);
    EXPECT_EQ(weight->toConceptId, conceptB->id);
}
