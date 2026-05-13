#include <gtest/gtest.h>

#include "repository/json_repository.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QTemporaryDir>

namespace {

QString testDataPath(const QString& fileName) {
    const QString baseDir = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath();
    return baseDir + "/data/" + fileName;
}

QByteArray readFile(const QString& path) {
    QFile file(path);
    EXPECT_TRUE(file.open(QIODevice::ReadOnly)) << path.toStdString();
    return file.readAll();
}

QJsonDocument readJsonDocument(const QString& path) {
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(readFile(path), &error);
    EXPECT_EQ(error.error, QJsonParseError::NoError) << error.errorString().toStdString();
    return document;
}

FCM makeModelForExport() {
    auto mainTerm = std::make_shared<Term>();
    mainTerm->id = QUuid("{11111111-1111-1111-1111-111111111111}");
    mainTerm->name = "High demand";
    mainTerm->description = "Main node term";
    mainTerm->value = 1.5;
    mainTerm->fuzzyValue = {0.5, 1.5, 2.5};
    mainTerm->color = QColor(12, 34, 56, 78);
    mainTerm->type = ElementType::Node;

    auto edgeTerm = std::make_shared<Term>();
    edgeTerm->id = QUuid("{22222222-2222-2222-2222-222222222222}");
    edgeTerm->name = "Strong influence";
    edgeTerm->description = "Edge term";
    edgeTerm->value = -0.75;
    edgeTerm->fuzzyValue = {-1.0, -0.75, -0.25};
    edgeTerm->color = QColor(90, 80, 70, 255);
    edgeTerm->type = ElementType::Edge;

    auto sourceConcept = std::make_shared<Concept>();
    sourceConcept->id = QUuid("{33333333-3333-3333-3333-333333333333}");
    sourceConcept->name = "Demand";
    sourceConcept->description = "Demand concept";
    sourceConcept->term = mainTerm;
    sourceConcept->startStep = 2;
    sourceConcept->pos = QPointF(10.5, -4.25);
    sourceConcept->nameLocation = ConceptNameLocation::BottomLeft;

    auto targetConcept = std::make_shared<Concept>();
    targetConcept->id = QUuid("{44444444-4444-4444-4444-444444444444}");
    targetConcept->name = "Supply";
    targetConcept->description = "Supply concept";
    targetConcept->term = nullptr;
    targetConcept->startStep = 0;
    targetConcept->pos = QPointF(-3.0, 8.0);
    targetConcept->nameLocation = ConceptNameLocation::UpRight;

    auto weight = std::make_shared<Weight>();
    weight->id = QUuid("{55555555-5555-5555-5555-555555555555}");
    weight->name = "Demand to Supply";
    weight->description = "Connection";
    weight->term = edgeTerm;
    weight->fromConceptId = sourceConcept->id;
    weight->toConceptId = targetConcept->id;

    auto experimentTerm = std::make_shared<Term>();
    experimentTerm->id = QUuid("{66666666-6666-6666-6666-666666666666}");
    experimentTerm->name = "Experiment term";
    experimentTerm->description = "Experiment node";
    experimentTerm->value = 0.25;
    experimentTerm->fuzzyValue = {0.1, 0.25, 0.5};
    experimentTerm->color = QColor(1, 2, 3, 4);
    experimentTerm->type = ElementType::Node;

    auto experimentConcept = std::make_shared<Concept>();
    experimentConcept->id = QUuid("{77777777-7777-7777-7777-777777777777}");
    experimentConcept->name = "Experiment concept";
    experimentConcept->description = "Experiment concept description";
    experimentConcept->term = experimentTerm;
    experimentConcept->startStep = 5;
    experimentConcept->pos = QPointF(1.25, 2.5);
    experimentConcept->nameLocation = ConceptNameLocation::CenterRight;

    auto experimentWeight = std::make_shared<Weight>();
    experimentWeight->id = QUuid("{88888888-8888-8888-8888-888888888888}");
    experimentWeight->name = "Experiment weight";
    experimentWeight->description = "Experiment connection";
    experimentWeight->term = experimentTerm;
    experimentWeight->fromConceptId = experimentConcept->id;
    experimentWeight->toConceptId = experimentConcept->id;

    Experiment experiment;
    experiment.predictionParameters.algorithm = "weights";
    experiment.predictionParameters.useFuzzyValues = true;
    experiment.predictionParameters.activationFunction = "sigmoid";
    experiment.predictionParameters.metric = "MAE";
    experiment.predictionParameters.predictToStatic = true;
    experiment.predictionParameters.threshold = 0.2;
    experiment.predictionParameters.stepsLessThreshold = 7;
    experiment.predictionParameters.fixedSteps = 11;
    experiment.predictionParameters.fuzzinessDegree = 1.8;
    experiment.timestamp = QDateTime(QDate(2026, 3, 14), QTime(15, 9, 26));
    experiment.terms.emplace(experimentTerm->id, experimentTerm);
    experiment.concepts.emplace(experimentConcept->id, experimentConcept);
    experiment.weights.emplace(experimentWeight->id, experimentWeight);

    FCM fcm;
    fcm.name = "Energy balance";
    fcm.description = "Model for json repository tests";
    fcm.predictionParameters.algorithm = "standard";
    fcm.predictionParameters.useFuzzyValues = false;
    fcm.predictionParameters.activationFunction = "tanh";
    fcm.predictionParameters.metric = "MSE";
    fcm.predictionParameters.predictToStatic = true;
    fcm.predictionParameters.threshold = 0.05;
    fcm.predictionParameters.stepsLessThreshold = 3;
    fcm.predictionParameters.fixedSteps = 9;
    fcm.predictionParameters.fuzzinessDegree = 1.4;
    fcm.terms.emplace(mainTerm->id, mainTerm);
    fcm.terms.emplace(edgeTerm->id, edgeTerm);
    fcm.concepts.emplace(sourceConcept->id, sourceConcept);
    fcm.concepts.emplace(targetConcept->id, targetConcept);
    fcm.weights.emplace(weight->id, weight);
    fcm.experiments.push_back(experiment);
    return fcm;
}

}

TEST(JsonRepositoryTest, ExportToJsonWritesExpectedDocument) {
    const FCM fcm = makeModelForExport();
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString outputPath = tempDir.path() + "/model.json";
    ASSERT_TRUE(JsonRepository::exportToJson(fcm, outputPath));

    const auto actualDocument = readJsonDocument(outputPath);
    const auto expectedDocument = readJsonDocument(testDataPath("full_model.json"));

    EXPECT_EQ(actualDocument, expectedDocument);
}

TEST(JsonRepositoryTest, ImportFromJsonReadsCompleteModel) {
    const auto imported = JsonRepository::importFromJson(testDataPath("full_model.json"));
    ASSERT_TRUE(imported.has_value());

    const FCM& fcm = imported.value();
    EXPECT_EQ(fcm.name, "Energy balance");
    EXPECT_EQ(fcm.description, "Model for json repository tests");
    EXPECT_EQ(fcm.predictionParameters.algorithm, "standard");
    EXPECT_FALSE(fcm.predictionParameters.useFuzzyValues);
    EXPECT_EQ(fcm.predictionParameters.activationFunction, "tanh");
    EXPECT_EQ(fcm.predictionParameters.metric, "MSE");
    EXPECT_TRUE(fcm.predictionParameters.predictToStatic);
    EXPECT_DOUBLE_EQ(fcm.predictionParameters.threshold, 0.05);
    EXPECT_EQ(fcm.predictionParameters.stepsLessThreshold, 3);
    EXPECT_EQ(fcm.predictionParameters.fixedSteps, 9);
    EXPECT_DOUBLE_EQ(fcm.predictionParameters.fuzzinessDegree, 1.4);

    ASSERT_EQ(fcm.terms.size(), 2U);
    ASSERT_EQ(fcm.concepts.size(), 2U);
    ASSERT_EQ(fcm.weights.size(), 1U);
    ASSERT_EQ(fcm.experiments.size(), 1U);

    const auto mainTerm = fcm.terms.at(QUuid("{11111111-1111-1111-1111-111111111111}"));
    ASSERT_NE(mainTerm, nullptr);
    EXPECT_EQ(mainTerm->name, "High demand");
    EXPECT_EQ(mainTerm->description, "Main node term");
    EXPECT_DOUBLE_EQ(mainTerm->value, 1.5);
    EXPECT_DOUBLE_EQ(mainTerm->fuzzyValue.l, 0.5);
    EXPECT_DOUBLE_EQ(mainTerm->fuzzyValue.m, 1.5);
    EXPECT_DOUBLE_EQ(mainTerm->fuzzyValue.u, 2.5);
    EXPECT_EQ(mainTerm->color, QColor(12, 34, 56, 78));
    EXPECT_EQ(mainTerm->type, ElementType::Node);
    EXPECT_EQ(mainTerm->dbId, -1);

    const auto sourceConcept = fcm.concepts.at(QUuid("{33333333-3333-3333-3333-333333333333}"));
    ASSERT_NE(sourceConcept, nullptr);
    EXPECT_EQ(sourceConcept->term, mainTerm);
    EXPECT_EQ(sourceConcept->startStep, 2U);
    EXPECT_EQ(sourceConcept->pos, QPointF(10.5, -4.25));
    EXPECT_EQ(sourceConcept->nameLocation, ConceptNameLocation::BottomLeft);
    EXPECT_EQ(sourceConcept->dbId, -1);

    const auto targetConcept = fcm.concepts.at(QUuid("{44444444-4444-4444-4444-444444444444}"));
    ASSERT_NE(targetConcept, nullptr);
    EXPECT_EQ(targetConcept->term, nullptr);
    EXPECT_EQ(targetConcept->nameLocation, ConceptNameLocation::UpRight);

    const auto weight = fcm.weights.at(QUuid("{55555555-5555-5555-5555-555555555555}"));
    ASSERT_NE(weight, nullptr);
    EXPECT_EQ(weight->term, fcm.terms.at(QUuid("{22222222-2222-2222-2222-222222222222}")));
    EXPECT_EQ(weight->fromConceptId, sourceConcept->id);
    EXPECT_EQ(weight->toConceptId, targetConcept->id);
    EXPECT_EQ(weight->dbId, -1);

    const Experiment& experiment = fcm.experiments.front();
    EXPECT_EQ(experiment.predictionParameters.algorithm, "weights");
    EXPECT_TRUE(experiment.predictionParameters.useFuzzyValues);
    EXPECT_EQ(experiment.predictionParameters.activationFunction, "sigmoid");
    EXPECT_EQ(experiment.predictionParameters.metric, "MAE");
    EXPECT_TRUE(experiment.predictionParameters.predictToStatic);
    EXPECT_DOUBLE_EQ(experiment.predictionParameters.threshold, 0.2);
    EXPECT_EQ(experiment.predictionParameters.stepsLessThreshold, 7);
    EXPECT_EQ(experiment.predictionParameters.fixedSteps, 11);
    EXPECT_DOUBLE_EQ(experiment.predictionParameters.fuzzinessDegree, 1.8);
    EXPECT_EQ(experiment.timestamp, QDateTime(QDate(2026, 3, 14), QTime(15, 9, 26)));
    EXPECT_EQ(experiment.dbId, -1);
    ASSERT_EQ(experiment.terms.size(), 1U);
    ASSERT_EQ(experiment.concepts.size(), 1U);
    ASSERT_EQ(experiment.weights.size(), 1U);
}

TEST(JsonRepositoryTest, ImportFromJsonAppliesDefaultsForLegacyJson) {
    const auto imported = JsonRepository::importFromJson(testDataPath("legacy_model.json"));
    ASSERT_TRUE(imported.has_value());

    const FCM& fcm = imported.value();
    EXPECT_DOUBLE_EQ(fcm.predictionParameters.fuzzinessDegree, 1.0);

    const auto legacyTerm = fcm.terms.at(QUuid("{aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa}"));
    ASSERT_NE(legacyTerm, nullptr);
    EXPECT_EQ(legacyTerm->color.alpha(), 255);

    const auto legacyConcept = fcm.concepts.at(QUuid("{bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb}"));
    ASSERT_NE(legacyConcept, nullptr);
    EXPECT_EQ(legacyConcept->nameLocation, ConceptNameLocation::Up);

    ASSERT_EQ(fcm.experiments.size(), 1U);
    const Experiment& experiment = fcm.experiments.front();
    EXPECT_DOUBLE_EQ(experiment.predictionParameters.fuzzinessDegree, 1.0);
    EXPECT_TRUE(experiment.terms.empty());
    EXPECT_TRUE(experiment.concepts.empty());
    EXPECT_TRUE(experiment.weights.empty());
}

TEST(JsonRepositoryTest, ImportFromJsonIgnoresMissingLinkedTerms) {
    const auto imported = JsonRepository::importFromJson(testDataPath("missing_term_links.json"));
    ASSERT_TRUE(imported.has_value());

    const FCM& fcm = imported.value();
    const auto concept = fcm.concepts.at(QUuid("{cccccccc-cccc-cccc-cccc-cccccccccccc}"));
    const auto weight = fcm.weights.at(QUuid("{dddddddd-dddd-dddd-dddd-dddddddddddd}"));

    ASSERT_NE(concept, nullptr);
    ASSERT_NE(weight, nullptr);
    EXPECT_EQ(concept->term, nullptr);
    EXPECT_EQ(weight->term, nullptr);
}

TEST(JsonRepositoryTest, ImportFromJsonReturnsNulloptForIncorrectJson) {
    const auto imported = JsonRepository::importFromJson(testDataPath("incorrect.json"));
    EXPECT_FALSE(imported.has_value());
}
