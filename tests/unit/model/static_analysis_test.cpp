#include <gtest/gtest.h>

#include "model/static_analysis/fuzzy_static_analyzer.h"
#include "model/static_analysis/static_analyzer.h"

namespace {

std::shared_ptr<FCM> makeAnalysisModel(bool fuzzy) {
    const QUuid aId("{11111111-1111-1111-1111-111111111111}");
    const QUuid bId("{22222222-2222-2222-2222-222222222222}");
    const QUuid cId("{33333333-3333-3333-3333-333333333333}");
    const QUuid abId("{44444444-4444-4444-4444-444444444444}");
    const QUuid bcId("{55555555-5555-5555-5555-555555555555}");

    auto conceptA = std::make_shared<Concept>();
    conceptA->id = aId;
    conceptA->name = "A";
    conceptA->description = "A description";
    conceptA->term = nullptr;
    conceptA->startStep = 0;
    conceptA->pos = QPointF();
    conceptA->nameLocation = ConceptNameLocation::Up;

    auto conceptB = std::make_shared<Concept>();
    conceptB->id = bId;
    conceptB->name = "B";
    conceptB->description = "B description";
    conceptB->term = nullptr;
    conceptB->startStep = 0;
    conceptB->pos = QPointF();
    conceptB->nameLocation = ConceptNameLocation::Up;

    auto conceptC = std::make_shared<Concept>();
    conceptC->id = cId;
    conceptC->name = "C";
    conceptC->description = "C description";
    conceptC->term = nullptr;
    conceptC->startStep = 0;
    conceptC->pos = QPointF();
    conceptC->nameLocation = ConceptNameLocation::Up;

    auto termAB = std::make_shared<Term>();
    termAB->id = QUuid::createUuid();
    termAB->name = "AB";
    termAB->description = "AB description";
    termAB->value = 0.5;
    termAB->fuzzyValue = {0.5, 0.5, 0.5};
    termAB->type = ElementType::Edge;
    termAB->color = QColor(10, 20, 30, 255);

    auto termBC = std::make_shared<Term>();
    termBC->id = QUuid::createUuid();
    termBC->name = "BC";
    termBC->description = "BC description";
    termBC->value = -0.25;
    termBC->fuzzyValue = {-0.25, -0.25, -0.25};
    termBC->type = ElementType::Edge;
    termBC->color = QColor(10, 20, 30, 255);

    auto weightAB = std::make_shared<Weight>();
    weightAB->id = abId;
    weightAB->name = "A->B";
    weightAB->description = "A->B description";
    weightAB->term = termAB;
    weightAB->fromConceptId = aId;
    weightAB->toConceptId = bId;

    auto weightBC = std::make_shared<Weight>();
    weightBC->id = bcId;
    weightBC->name = "B->C";
    weightBC->description = "B->C description";
    weightBC->term = termBC;
    weightBC->fromConceptId = bId;
    weightBC->toConceptId = cId;

    if (fuzzy) {
        weightAB->term->value = 0.0;
        weightBC->term->value = 0.0;
    }

    auto fcm = std::make_shared<FCM>();
    fcm->concepts.emplace(aId, conceptA);
    fcm->concepts.emplace(bId, conceptB);
    fcm->concepts.emplace(cId, conceptC);
    fcm->weights.emplace(abId, weightAB);
    fcm->weights.emplace(bcId, weightBC);
    return fcm;
}

}

TEST(StaticAnalyzerTest, CalculatesMetricsForSimpleGraph) {
    const auto fcm = makeAnalysisModel(false);
    StaticAnalyzer analyzer(fcm);

    analyzer.init();
    analyzer.updateInfluence(QUuid("{33333333-3333-3333-3333-333333333333}"), 1, false);

    const auto& result = analyzer.getResult();
    EXPECT_NEAR(result.density, 2.0 / 6.0, 1e-9);
    EXPECT_DOUBLE_EQ(result.complexity, 1.0);
    EXPECT_NEAR(result.hierarchyIndex, 0.1458333333, 1e-9);
    ASSERT_EQ(result.factors.size(), 3U);
    EXPECT_DOUBLE_EQ(result.factors.at(QUuid("{11111111-1111-1111-1111-111111111111}")).outDegree, 0.5);
    EXPECT_DOUBLE_EQ(result.factors.at(QUuid("{22222222-2222-2222-2222-222222222222}")).inDegree, 0.5);
    EXPECT_DOUBLE_EQ(result.factors.at(QUuid("{22222222-2222-2222-2222-222222222222}")).outDegree, -0.25);
    EXPECT_DOUBLE_EQ(result.factors.at(QUuid("{22222222-2222-2222-2222-222222222222}")).influence, -0.25);
}

TEST(StaticAnalyzerTest, UpdatesFactorMetricsWhenWeightChanges) {
    const auto fcm = makeAnalysisModel(false);
    StaticAnalyzer analyzer(fcm);
    analyzer.init();

    const auto weight = fcm->weights.at(QUuid("{44444444-4444-4444-4444-444444444444}"));
    weight->term->value = 0.75;
    analyzer.onWeightUpdated(weight);

    const auto& result = analyzer.getResult();
    EXPECT_DOUBLE_EQ(result.factors.at(QUuid("{11111111-1111-1111-1111-111111111111}")).outDegree, 0.75);
    EXPECT_DOUBLE_EQ(result.factors.at(QUuid("{22222222-2222-2222-2222-222222222222}")).inDegree, 0.75);
}

TEST(FuzzyStaticAnalyzerTest, NumericProjectionMatchesEquivalentCrispWeights) {
    const auto fcm = makeAnalysisModel(true);
    FuzzyStaticAnalyzer analyzer(fcm);

    analyzer.init();
    analyzer.updateInfluence(QUuid("{33333333-3333-3333-3333-333333333333}"), 1, false);

    const auto numeric = analyzer.getNumericResult();
    EXPECT_NEAR(numeric.density, 2.0 / 6.0, 1e-9);
    EXPECT_DOUBLE_EQ(numeric.complexity, 1.0);
    EXPECT_NEAR(numeric.hierarchyIndex, 0.1458333333, 1e-9);
    EXPECT_DOUBLE_EQ(numeric.factors.at(QUuid("{22222222-2222-2222-2222-222222222222}")).influence, -0.25);
}
