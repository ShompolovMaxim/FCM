#include "models_repository.h"

#include <algorithm>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>

std::map<size_t, Term> toValueTerms(
    const std::map<size_t, std::shared_ptr<Term>>& termsPtr
    ) {
    std::map<size_t, Term> result;

    for (const auto& [id, termPtr] : termsPtr) {
        result[id] = *termPtr;
    }

    return result;
}

ModelsRepository::ModelsRepository(QSqlDatabase database) : db(database) {}

std::optional<size_t> ModelsRepository::createConcept(const Concept& concept, const size_t experimentId, const std::optional<size_t>& dbTermId) {
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO concepts (name, description, experiment_id, term_id, first_step, x_pos, y_pos) "
        "VALUES (:name, :description, :experiment_id, :term_id, :first_step, :x_pos, :y_pos)"
        );
    query.bindValue(":name", concept.name);
    query.bindValue(":description", concept.description);
    query.bindValue(":experiment_id", experimentId);

    if (dbTermId.has_value()) {
        query.bindValue(":term_id", dbTermId.value());
    } else {
        query.bindValue(":term_id", QVariant()); // NULL
    }

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

std::optional<size_t> ModelsRepository::createWeight(const Weight& weight, const size_t experimentId, const size_t fromConceptId, const size_t toConceptId, const std::map<size_t, size_t>& termsDBIds) {
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO weights (name, description, experiment_id, term_id, concept_from_id, concept_to_id) "
        "VALUES (:name, :description, :experiment_id, :term_id, :concept_from_id, :concept_to_id)"
        );

    query.bindValue(":name", weight.name);
    query.bindValue(":description", weight.description);
    query.bindValue(":experiment_id", experimentId);

    if (weight.term) {
        query.bindValue(":term_id", termsDBIds.at(weight.term->id));
    } else {
        query.bindValue(":term_id", QVariant());
    }

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

    std::map<size_t, size_t> termsDBIds;
    for (const auto& [_, term] : experiment.terms) {
        auto termDbId = createTerm(term, experimentId);
        if (!termDbId) {
            db.rollback();
            return {};
        }
        termsDBIds[term.id] = *termDbId;
    }

    std::map<size_t, size_t> conceptsDBIds;
    for (const auto& [_, concept] : experiment.concepts) {

        std::optional<size_t> dbTermId;

        if (concept.term) {
            dbTermId = termsDBIds.at(concept.term->id);
        }

        auto conceptDbId = createConcept(concept, experimentId, dbTermId);
        if (!conceptDbId) {
            db.rollback();
            return {};
        }

        conceptsDBIds[concept.id] = *conceptDbId;
    }

    for (const auto& [_, weight] : experiment.weights) {
        if (!createWeight(
                weight,
                experimentId,
                conceptsDBIds.at(weight.fromConceptId),
                conceptsDBIds.at(weight.toConceptId),
                termsDBIds
                )) {
            db.rollback();
            return {};
        }
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
    std::map<size_t, Weight> valueWeights;
    for (const auto& [id, wPtr] : fcm.weights) {
        valueWeights[id] = *wPtr;
    }

    auto currentExperiment = Experiment{
        toValueTerms(fcm.terms),
        {},
        valueWeights,
        fcm.predictionParameters,
        QDateTime::currentDateTime()
    };
    for (const auto& [id, concept] : fcm.concepts) {
        currentExperiment.concepts[id] = *concept;
    }
    createExperiment(currentExperiment, modelId);

    db.commit();
    return modelId;
}

std::optional<std::vector<Concept>> ModelsRepository::getExperimentConcepts(const size_t experimentId, const std::map<size_t, Term>& terms) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id, name, description, term_id, first_step, x_pos, y_pos "
        "FROM concepts WHERE experiment_id = :experiment_id"
        );

    query.bindValue(":experiment_id", experimentId);

    if (!query.exec()) {
        return {};
    }

    std::vector<Concept> result;

    std::map<size_t, std::shared_ptr<Term>> termPool;

    while (query.next()) {

        std::shared_ptr<Term> termPtr = nullptr;

        auto termIdValue = query.value("term_id");
        if (!termIdValue.isNull()) {
            size_t dbTermId = termIdValue.toUInt();

            if (!termPool.count(dbTermId)) {
                termPool[dbTermId] = std::make_shared<Term>(terms.at(dbTermId));
            }

            termPtr = termPool[dbTermId];
        }

        result.emplace_back(Concept{
            query.value("id").toUInt(),
            query.value("name").toString(),
            query.value("description").toString(),
            termPtr,
            QPointF(
                query.value("x_pos").toDouble(),
                query.value("y_pos").toDouble()
                ),
            query.value("first_step").toUInt()
        });
    }

    return result;
}

std::optional<std::vector<Weight>> ModelsRepository::getExperimentWeights(const size_t experimentId, const std::map<size_t, Term>& terms) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id, name, description, term_id, concept_from_id, concept_to_id "
        "FROM weights "
        "WHERE experiment_id = :experiment_id"
        );
    query.bindValue(":experiment_id", experimentId);

    if (!query.exec()) {
        return {};
    }

    std::vector<Weight> result;

    std::map<size_t, std::shared_ptr<Term>> termPool;

    while (query.next()) {

        std::shared_ptr<Term> termPtr = nullptr;

        auto termIdValue = query.value("term_id");
        if (!termIdValue.isNull()) {
            size_t dbTermId = termIdValue.toUInt();

            if (!termPool.count(dbTermId)) {
                termPool[dbTermId] = std::make_shared<Term>(terms.at(dbTermId));
            }

            termPtr = termPool[dbTermId];
        }

        result.emplace_back(Weight{
            query.value("id").toUInt(),
            query.value("name").toString(),
            query.value("description").toString(),
            termPtr,
            query.value("concept_from_id").toUInt(),
            query.value("concept_to_id").toUInt()
        });
    }

    return result;
}

std::optional<std::map<size_t, std::shared_ptr<Term>>> ModelsRepository::getExperimentTerms(const size_t experimentId) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id, name, description, numeric_value, tr_value_l, tr_value_m, tr_value_h, color_r, color_g, color_b "
        "FROM terms "
        "WHERE experiment_id = :experiment_id"
        );

    query.bindValue(":experiment_id", experimentId);

    if (!query.exec()) {
        return {};
    }

    std::map<size_t, std::shared_ptr<Term>> result;

    while (query.next()) {
        auto term = std::make_shared<Term>(Term{
            query.value("id").toUInt(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("numeric_value").toDouble(),
            query.value("tr_value_l").toDouble(),
            query.value("tr_value_m").toDouble(),
            query.value("tr_value_h").toDouble(),
            QColor(query.value("color_r").toInt(), query.value("color_g").toInt(), query.value("color_b").toInt())
        });

        result[term->id] = term;
    }

    return result;
}

std::optional<std::vector<Experiment>> ModelsRepository::getExperiments(const size_t modelId) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id, timestamp, algorithm, activation, predict_to_static, metric, threshold, steps_less_threshold, fixed_steps "
        "FROM experiments WHERE model_id = :model_id"
        );

    query.bindValue(":model_id", modelId);

    if (!query.exec()) {
        return {};
    }

    std::vector<Experiment> result;

    while (query.next()) {
        auto experimentId = query.value("id").toUInt();

        Experiment experiment;
        experiment.timestamp = query.value("timestamp").toDateTime();
        experiment.predictionParameters = {
            query.value("algorithm").toString(),
            query.value("activation").toString(),
            query.value("metric").toString(),
            query.value("predict_to_static").toBool(),
            query.value("threshold").toDouble(),
            query.value("steps_less_threshold").toInt(),
            query.value("fixed_steps").toInt(),
        };

        auto terms = getExperimentTerms(experimentId);
        if (!terms) return {};
        experiment.terms = toValueTerms(*terms);

        auto concepts = getExperimentConcepts(experimentId, experiment.terms);
        if (!concepts) return {};
        for (auto& c : *concepts) {
            experiment.concepts[c.id] = c;
        }

        auto weights = getExperimentWeights(experimentId, experiment.terms);
        if (!weights) return {};
        for (auto& w : *weights) {
            experiment.weights[w.id] = w;
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
    query.prepare(
        "SELECT id, name, description FROM models WHERE name = :name"
        );
    query.bindValue(":name", modelName);

    if (!query.exec() || !query.next()) {
        return {};
    }

    FCM fcm;
    fcm.name = modelName;
    fcm.description = query.value("description").toString();

    auto experiments = getExperiments(query.value("id").toUInt());
    if (!experiments) return {};

    std::sort(experiments->begin(), experiments->end(),
              [](const Experiment& a, const Experiment& b) {
                  return a.timestamp < b.timestamp;
              });

    auto current = experiments->back();
    experiments->pop_back();

    for (const auto& [id, term] : current.terms) {
        fcm.terms[id] = std::make_shared<Term>(term);
    }

    for (const auto& [id, concept] : current.concepts) {

        auto c = std::make_shared<Concept>(concept);

        if (c->term) {
            size_t termId = c->term->id;
            c->term = fcm.terms.at(termId);
        }

        fcm.concepts[id] = c;
    }

    for (const auto& [id, weight] : current.weights) {
        fcm.weights[id] = std::make_shared<Weight>(weight);
    }
    fcm.predictionParameters = current.predictionParameters;
    fcm.experiments = *experiments;

    return fcm;
}
