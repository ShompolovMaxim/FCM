#include "models_repository.h"

#include <algorithm>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>

ModelsRepository::ModelsRepository(QSqlDatabase database) : db(database) {}

std::optional<size_t> ModelsRepository::createConcept(const Concept& concept, const size_t experimentId) {
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO concepts (name, description, experiment_id, value, first_step, x_pos, y_pos) "
        "VALUES (:name, :description, :experiment_id, :value, :first_step, :x_pos, :y_pos)"
        );
    query.bindValue(":name", concept.name);
    query.bindValue(":description", concept.description);
    query.bindValue(":experiment_id", experimentId);
    query.bindValue(":value", concept.value);
    query.bindValue(":first_step", concept.startStep);
    query.bindValue(":x_pos", concept.pos.x());
    query.bindValue(":y_pos", concept.pos.y());

    if (!query.exec()) {
        db.rollback();
        return {};
    }

    db.commit();
    return query.lastInsertId().toUInt();
}

std::optional<size_t> ModelsRepository::createWeight(const Weight& weight, const size_t experimentId, const size_t fromConceptId, const size_t toConceptId) {
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO weights (name, description, experiment_id, value, concept_from_id, concept_to_id) "
        "VALUES (:name, :description, :experiment_id, :value, :concept_from_id, :concept_to_id)"
        );
    query.bindValue(":name", weight.name);
    query.bindValue(":description", weight.description);
    query.bindValue(":experiment_id", experimentId);
    query.bindValue(":value", weight.value);
    query.bindValue(":concept_from_id", fromConceptId);
    query.bindValue(":concept_to_id", toConceptId);

    if (!query.exec()) {
        db.rollback();
        return {};
    }

    db.commit();
    return query.lastInsertId().toUInt();
}

std::optional<size_t> ModelsRepository::createTerm(const Term& term, const size_t experimentId) {
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO terms (name, description, experiment_id, numeric_value, tr_value_l, tr_value_m, tr_value_h, color_r, color_g, color_b) "
        "VALUES (:name, :description, :experiment_id, :numeric_value, :tr_value_l, :tr_value_m, :tr_value_h, :color_r, :color_g, :color_b)"
        );
    query.bindValue(":name", term.name);
    query.bindValue(":description", term.description);
    query.bindValue(":experiment_id", experimentId);
    query.bindValue(":numeric_value", term.value);
    query.bindValue(":tr_value_l", term.fuzzyValueL);
    query.bindValue(":tr_value_m", term.fuzzyValueM);
    query.bindValue(":tr_value_h", term.fuzzyValueU);
    query.bindValue(":color_r", term.color.red());
    query.bindValue(":color_g", term.color.green());
    query.bindValue(":color_b", term.color.blue());

    if (!query.exec()) {
        db.rollback();
        return {};
    }

    db.commit();
    return query.lastInsertId().toUInt();
}

std::optional<size_t> ModelsRepository::createExperiment(const Experiment& experiment, const size_t modelId) {
    db.transaction();

    QSqlQuery queryExperiment(db);
    queryExperiment.prepare(
        "INSERT INTO experiments (model_id, timestamp, algorithm, activation, predict_to_static, metric, threshold, steps_less_threshold, fixed_steps) "
        "VALUES (:model_id, :timestamp, :algorithm, :activation, :predict_to_static, :metric, :threshold, :steps_less_threshold, :fixed_steps)"
        );
    queryExperiment.bindValue(":model_id", modelId);
    queryExperiment.bindValue(":timestamp", experiment.timestamp);
    queryExperiment.bindValue(":algorithm", experiment.predictionParameters.algorithm);
    queryExperiment.bindValue(":activation", experiment.predictionParameters.activationFunction);
    queryExperiment.bindValue(":predict_to_static", experiment.predictionParameters.predictToStatic);
    queryExperiment.bindValue(":metric", experiment.predictionParameters.metric);
    queryExperiment.bindValue(":threshold", experiment.predictionParameters.threshold);
    queryExperiment.bindValue(":steps_less_threshold", experiment.predictionParameters.stepsLessThreshold);
    queryExperiment.bindValue(":fixed_steps", experiment.predictionParameters.fixedSteps);

    if (!queryExperiment.exec()) {
        db.rollback();
        return {};
    }
    auto experimentId = queryExperiment.lastInsertId().toUInt();

    for (const auto& [_, term] : experiment.terms) {
        createTerm(term, experimentId);
    }

    std::map<size_t, size_t> conceptsDBIds;
    for (const auto& [_, concept] : experiment.concepts) {
        conceptsDBIds[concept.id] = *createConcept(concept, experimentId);
    }

    for (const auto& [_, weight] : experiment.weights) {
        createWeight(weight, experimentId, conceptsDBIds[weight.fromConceptId], conceptsDBIds[weight.toConceptId]);
    }

    db.commit();
    return experimentId;
}

std::optional<size_t> ModelsRepository::createModel(const FCM& fcm) {
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO models (name, description) "
        "VALUES (:name, :description)"
        );
    query.bindValue(":name", fcm.name);
    query.bindValue(":description", fcm.description);

    if (!query.exec()) {
        db.rollback();
        return {};
    }
    auto modelId =  query.lastInsertId().toUInt();

    for (const auto& experiment : fcm.experiments) {
        createExperiment(experiment, modelId);
    }
    const auto currentExperiment = Experiment{
        fcm.terms,
        fcm.concepts,
        fcm.weights,
        fcm.predictionParameters,
        QDateTime::currentDateTime()
    };
    createExperiment(currentExperiment, modelId);

    db.commit();
    return modelId;
}

std::optional<std::vector<Concept>> ModelsRepository::getExperimentConcepts(const size_t experimentId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, description, value, first_step, x_pos, y_pos "
                  "FROM concepts "
                  "WHERE experiment_id = :experiment_id"
                  );
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) {
        return {};
    }

    std::vector<Concept> result;

    while (query.next()) {
        auto concept = Concept{
            query.value("id").toUInt(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("value").toDouble(),
            QPointF(query.value("x_pos").toDouble(), query.value("y_pos").toDouble()),
            query.value("first_step").toUInt()
        };
        result.push_back(concept);
    }

    return result;
}

std::optional<std::vector<Weight>> ModelsRepository::getExperimentWeights(const size_t experimentId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, description, value, concept_from_id, concept_to_id "
                  "FROM weights "
                  "WHERE experiment_id = :experiment_id"
                  );
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) {
        return {};
    }

    std::vector<Weight> result;

    while (query.next()) {
        auto weight = Weight{
            query.value("id").toUInt(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("value").toDouble(),
            query.value("concept_from_id").toUInt(),
            query.value("concept_to_id").toUInt(),

        };
        result.push_back(weight);
    }

    return result;
}

std::optional<std::vector<Term>> ModelsRepository::getExperimentTerms(const size_t experimentId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, description, numeric_value, tr_value_l, tr_value_m, tr_value_h, color_r, color_g, color_b "
                  "FROM terms "
                  "WHERE experiment_id = :experiment_id"
                  );
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) {
        return {};
    }

    std::vector<Term> result;

    while (query.next()) {
        auto term =  Term{
            query.value("id").toUInt(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("numeric_value").toDouble(),
            query.value("tr_value_l").toDouble(),
            query.value("tr_value_m").toDouble(),
            query.value("tr_value_h").toDouble(),
            QColor(query.value("color_r").toInt(), query.value("color_g").toInt(), query.value("color_b").toInt())
        };
        result.push_back(term);
    }

    return result;
}

std::optional<std::vector<Experiment>> ModelsRepository::getExperiments(const size_t modelId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, timestamp, algorithm, activation, predict_to_static, metric, threshold, steps_less_threshold, fixed_steps "
                  "FROM experiments "
                  "WHERE model_id = :model_id"
                  );
    query.bindValue(":model_id", modelId);
    if (!query.exec()) {
        return {};
    }

    std::vector<Experiment> result;

    while (query.next()) {
        Experiment experiment;
        experiment.timestamp = query.value("timestamp").toDateTime();
        experiment.predictionParameters = PredictionParameters{
            query.value("algorithm").toString(),
            query.value("activation").toString(),
            query.value("metric").toString(),
            query.value("predict_to_static").toBool(),
            query.value("threshold").toDouble(),
            query.value("steps_less_threshold").toInt(),
            query.value("fixed_steps").toInt(),
        };

        auto terms = getExperimentTerms(query.value("id").toUInt());
        for (auto& term : *terms) {
            experiment.terms[term.id] = term;
        }

        auto concepts = getExperimentConcepts(query.value("id").toUInt());
        for (auto& concept : *concepts) {
            experiment.concepts[concept.id] = concept;
        }

        auto weights = getExperimentWeights(query.value("id").toUInt());
        for (auto& weight : *weights) {
            experiment.weights[weight.id] = weight;
        }

        result.push_back(experiment);
    }

    return result;
}

QList<QString> ModelsRepository::getModelsNames() {
    QStringList modelNames;

    QSqlQuery query(db);
    if (!query.exec("SELECT name FROM models")) {
        return modelNames;
    }

    while (query.next()) {
        QString name = query.value(0).toString();
        modelNames.append(name);
    }

    return modelNames;
}

std::optional<FCM> ModelsRepository::getModel(const QString modelName) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, description "
                  "FROM models "
                  "WHERE name = :name"
                  );
    query.bindValue(":name", modelName);
    if (!query.exec()) {
        return {};
    }

    if (!query.next()) {
        return {};
    }

    FCM fcm;
    fcm.name = modelName;
    fcm.description = query.value("description").toString();
    auto experiments = getExperiments(query.value("id").toUInt());
    std::sort(experiments->begin(), experiments->end(), [](const Experiment& a, const Experiment& b) {
        return a.timestamp < b.timestamp;
    });
    auto currentExperiment = (*experiments)[experiments->size() - 1];
    experiments->pop_back();
    fcm.terms = currentExperiment.terms;
    fcm.concepts = currentExperiment.concepts;
    fcm.weights = currentExperiment.weights;
    fcm.predictionParameters = currentExperiment.predictionParameters;
    fcm.experiments = *experiments;
    return fcm;
}

