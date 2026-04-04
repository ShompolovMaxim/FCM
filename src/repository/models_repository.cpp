#include "models_repository.h"

#include <algorithm>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ModelsRepository::ModelsRepository(QSqlDatabase database) : db(database) {}

std::optional<int> ModelsRepository::createConcept(const Concept& conceptConst, const int experimentId, const std::optional<int>& dbTermId) {
    auto& concept = const_cast<Concept&>(conceptConst);

    db.transaction();
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO concepts (uuid, name, description, experiment_id, term_id, first_step, x_pos, y_pos) "
        "VALUES (:uuid, :name, :description, :experiment_id, :term_id, :first_step, :x_pos, :y_pos)"
        );
    query.bindValue(":uuid", concept.id);
    query.bindValue(":name", concept.name);
    query.bindValue(":description", concept.description);
    query.bindValue(":experiment_id", experimentId);
    if (dbTermId.has_value()) query.bindValue(":term_id", dbTermId.value()); else query.bindValue(":term_id", QVariant());
    query.bindValue(":first_step", concept.startStep);
    query.bindValue(":x_pos", concept.pos.x());
    query.bindValue(":y_pos", concept.pos.y());

    if (!query.exec()) {
        qDebug() << "SQL Error (createConcept):" << query.lastError().text() << "Query:" << query.lastQuery();
        db.rollback();
        return {};
    }

    concept.dbId = query.lastInsertId().toInt();
    db.commit();
    return concept.dbId;
}

std::optional<int> ModelsRepository::createWeight(const Weight& weightConst, const int experimentId, const int fromConceptId, const int toConceptId, const std::map<QUuid, int>& termsDBIds) {
    auto& weight = const_cast<Weight&>(weightConst);

    db.transaction();
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO weights (uuid, name, description, experiment_id, term_id, concept_from_id, concept_to_id) "
        "VALUES (:uuid, :name, :description, :experiment_id, :term_id, :concept_from_id, :concept_to_id)"
        );

    query.bindValue(":uuid", weight.id);
    query.bindValue(":name", weight.name);
    query.bindValue(":description", weight.description);
    query.bindValue(":experiment_id", experimentId);
    if (weight.term) query.bindValue(":term_id", termsDBIds.at(weight.term->id)); else query.bindValue(":term_id", QVariant());
    query.bindValue(":concept_from_id", fromConceptId);
    query.bindValue(":concept_to_id", toConceptId);

    if (!query.exec()) {
        qDebug() << "SQL Error (createWeight):" << query.lastError().text() << "Query:" << query.lastQuery();
        db.rollback();
        return {};
    }

    weight.dbId = query.lastInsertId().toInt();
    db.commit();
    return weight.dbId;
}

std::optional<int> ModelsRepository::createTerm(const Term& termConst, const int experimentId) {
    auto& term = const_cast<Term&>(termConst);

    db.transaction();
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO terms (uuid, name, description, experiment_id, numeric_value, tr_value_l, tr_value_m, tr_value_h, color_r, color_g, color_b, type) "
        "VALUES (:uuid, :name, :description, :experiment_id, :numeric_value, :tr_value_l, :tr_value_m, :tr_value_h, :color_r, :color_g, :color_b, :type)"
        );

    query.bindValue(":uuid", term.id);
    query.bindValue(":name", term.name);
    query.bindValue(":description", term.description);
    query.bindValue(":experiment_id", experimentId);
    query.bindValue(":numeric_value", term.value);
    query.bindValue(":tr_value_l", term.fuzzyValue.l);
    query.bindValue(":tr_value_m", term.fuzzyValue.m);
    query.bindValue(":tr_value_h", term.fuzzyValue.u);
    query.bindValue(":color_r", term.color.red());
    query.bindValue(":color_g", term.color.green());
    query.bindValue(":color_b", term.color.blue());
    query.bindValue(":type", elementTypeToString(term.type));

    if (!query.exec()) {
        qDebug() << "SQL Error (createTerm):" << query.lastError().text() << "Query:" << query.lastQuery();
        db.rollback();
        return {};
    }

    term.dbId = query.lastInsertId().toInt();
    db.commit();
    return term.dbId;
}

std::optional<int> ModelsRepository::createExperiment(const Experiment& experimentConst, const int modelId) {
    auto& experiment = const_cast<Experiment&>(experimentConst);

    db.transaction();
    QSqlQuery queryExperiment(db);
    queryExperiment.prepare(
        "INSERT INTO experiments (model_id, timestamp, algorithm, use_fuzzy_values, activation, predict_to_static, metric, threshold, steps_less_threshold, fixed_steps) "
        "VALUES (:model_id, :timestamp, :algorithm, :use_fuzzy_values, :activation, :predict_to_static, :metric, :threshold, :steps_less_threshold, :fixed_steps)"
        );

    queryExperiment.bindValue(":model_id", modelId);
    queryExperiment.bindValue(":timestamp", experiment.timestamp);
    queryExperiment.bindValue(":algorithm", experiment.predictionParameters.algorithm);
    queryExperiment.bindValue(":use_fuzzy_values", experiment.predictionParameters.useFuzzyValues);
    queryExperiment.bindValue(":activation", experiment.predictionParameters.activationFunction);
    queryExperiment.bindValue(":predict_to_static", experiment.predictionParameters.predictToStatic);
    queryExperiment.bindValue(":metric", experiment.predictionParameters.metric);
    queryExperiment.bindValue(":threshold", experiment.predictionParameters.threshold);
    queryExperiment.bindValue(":steps_less_threshold", experiment.predictionParameters.stepsLessThreshold);
    queryExperiment.bindValue(":fixed_steps", experiment.predictionParameters.fixedSteps);

    if (!queryExperiment.exec()) {
        qDebug() << "SQL Error (createExperiment):" << queryExperiment.lastError().text() << "Query:" << queryExperiment.lastQuery();
        db.rollback();
        return {};
    }

    experiment.dbId = queryExperiment.lastInsertId().toInt();

    std::map<QUuid, int> termsDBIds;
    for (auto& [_, termPtr] : experiment.terms) {
        auto termDbId = createTerm(*termPtr, experiment.dbId);
        if (!termDbId) { db.rollback(); return {}; }
        termsDBIds[termPtr->id] = *termDbId;
    }

    std::map<QUuid, int> conceptsDBIds;
    for (auto& [_, conceptPtr] : experiment.concepts) {
        std::optional<int> dbTermId;
        if (conceptPtr->term) dbTermId = termsDBIds.at(conceptPtr->term->id);

        auto conceptDbId = createConcept(*conceptPtr, experiment.dbId, dbTermId);
        if (!conceptDbId) { db.rollback(); return {}; }
        conceptsDBIds[conceptPtr->id] = *conceptDbId;
    }

    for (auto& [_, weightPtr] : experiment.weights) {
        if (!createWeight(*weightPtr, experiment.dbId, conceptsDBIds.at(weightPtr->fromConceptId), conceptsDBIds.at(weightPtr->toConceptId), termsDBIds)) {
            db.rollback();
            return {};
        }
    }

    db.commit();
    return experiment.dbId;
}

std::optional<int> ModelsRepository::createModel(const FCM& fcmConst) {
    auto& fcm = const_cast<FCM&>(fcmConst);

    db.transaction();
    QSqlQuery query(db);
    query.prepare("INSERT INTO models (name, description) VALUES (:name, :description)");
    query.bindValue(":name", fcm.name);
    query.bindValue(":description", fcm.description);

    if (!query.exec()) { qDebug() << "SQL Error (createModel):" << query.lastError().text() << "Query:" << query.lastQuery(); db.rollback(); return {}; }

    fcm.dbId = query.lastInsertId().toInt();

    for (auto& experiment : fcm.experiments) {
        if (!createExperiment(experiment, fcm.dbId)) { db.rollback(); return {}; }
    }

    if (!fcm.terms.empty() || !fcm.concepts.empty() || !fcm.weights.empty()) {
        Experiment current;
        current.terms = fcm.terms;
        current.concepts = fcm.concepts;
        current.weights = fcm.weights;
        current.predictionParameters = fcm.predictionParameters;
        current.timestamp = QDateTime::currentDateTime();
        if (!createExperiment(current, fcm.dbId)) { db.rollback(); return {}; }
    }

    db.commit();
    return fcm.dbId;
}

std::optional<std::vector<Concept>> ModelsRepository::getExperimentConcepts(const int experimentId, const std::map<QUuid, std::shared_ptr<Term>>& terms) {
    QSqlQuery query(db);
    query.prepare("SELECT id, uuid, name, description, term_id, first_step, x_pos, y_pos FROM concepts WHERE experiment_id = :experiment_id");
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) { qDebug() << "SQL Error (getExperimentConcepts):" << query.lastError().text() << "Query:" << query.lastQuery(); return {}; }

    std::vector<Concept> result;
    while (query.next()) {
        std::shared_ptr<Term> termPtr = nullptr;
        auto termIdValue = query.value("term_id");
        if (!termIdValue.isNull()) {
            int dbTermId = termIdValue.toInt();
            for (const auto& [_, t] : terms) { if (t->dbId == dbTermId) { termPtr = t; } }
        }
        Concept concept{query.value("uuid").toUuid(), query.value("name").toString(), query.value("description").toString(), termPtr, QPointF(query.value("x_pos").toDouble(), query.value("y_pos").toDouble()), query.value("first_step").toUInt()};
        concept.dbId = query.value("id").toInt();
        result.emplace_back(concept);
    }
    return result;
}

std::optional<std::vector<Weight>> ModelsRepository::getExperimentWeights(const int experimentId, const std::map<QUuid, std::shared_ptr<Term>>& terms, const std::map<int, std::shared_ptr<Concept>>& conceptsByDbId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, uuid, name, description, term_id, concept_from_id, concept_to_id FROM weights WHERE experiment_id = :experiment_id");
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) { qDebug() << "SQL Error (getExperimentWeights):" << query.lastError().text() << "Query:" << query.lastQuery(); return {}; }

    std::vector<Weight> result;
    while (query.next()) {
        std::shared_ptr<Term> termPtr = nullptr;
        auto termIdValue = query.value("term_id");
        if (!termIdValue.isNull()) {
            int dbTermId = termIdValue.toInt();
            for (const auto& [_, t] : terms) { if (t->dbId == dbTermId) { termPtr = t; break; } }
        }

        int dbConceptFromId = query.value("concept_from_id").toInt();
        int dbConceptToId = query.value("concept_to_id").toInt();
        QUuid conceptFromUuid, conceptToUuid;
        auto itFrom = conceptsByDbId.find(dbConceptFromId);
        if (itFrom != conceptsByDbId.end()) conceptFromUuid = itFrom->second->id;
        auto itTo = conceptsByDbId.find(dbConceptToId);
        if (itTo != conceptsByDbId.end()) conceptToUuid = itTo->second->id;

        Weight weight{query.value("uuid").toUuid(), query.value("name").toString(), query.value("description").toString(), termPtr, conceptFromUuid, conceptToUuid};
        weight.dbId = query.value("id").toInt();
        result.emplace_back(weight);
    }
    return result;
}

std::optional<std::map<QUuid, std::shared_ptr<Term>>> ModelsRepository::getExperimentTerms(const int experimentId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, uuid, name, description, numeric_value, tr_value_l, tr_value_m, tr_value_h, color_r, color_g, color_b, type FROM terms WHERE experiment_id = :experiment_id");
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) { qDebug() << "SQL Error (getExperimentTerms):" << query.lastError().text() << "Query:" << query.lastQuery(); return {}; }

    std::map<QUuid, std::shared_ptr<Term>> result;
    while (query.next()) {
        auto term = std::make_shared<Term>(Term{query.value("uuid").toUuid(), query.value("name").toString(), query.value("description").toString(), query.value("numeric_value").toDouble(), query.value("tr_value_l").toDouble(), query.value("tr_value_m").toDouble(), query.value("tr_value_h").toDouble(), QColor(query.value("color_r").toInt(), query.value("color_g").toInt(), query.value("color_b").toInt()), elementTypeFromString(query.value("type").toString())});
        term->dbId = query.value("id").toInt();
        result[term->id] = term;
    }
    return result;
}

std::optional<std::vector<Experiment>> ModelsRepository::getExperiments(const int modelId) {
    QSqlQuery query(db);
    query.prepare("SELECT id, timestamp, algorithm, activation, predict_to_static, metric, threshold, steps_less_threshold, fixed_steps, use_fuzzy_values FROM experiments WHERE model_id = :model_id");
    query.bindValue(":model_id", modelId);
    if (!query.exec()) { qDebug() << "SQL Error (getExperiments):" << query.lastError().text() << "Query:" << query.lastQuery(); return {}; }

    std::vector<Experiment> result;
    while (query.next()) {
        Experiment experiment;
        experiment.dbId = query.value("id").toInt();
        experiment.timestamp = query.value("timestamp").toDateTime();
        experiment.predictionParameters = { query.value("algorithm").toString(), query.value("use_fuzzy_values").toBool(), query.value("activation").toString(), query.value("metric").toString(), query.value("predict_to_static").toBool(), query.value("threshold").toDouble(), query.value("steps_less_threshold").toInt(), query.value("fixed_steps").toInt() };

        auto termsOpt = getExperimentTerms(experiment.dbId);
        if (!termsOpt) return {};
        experiment.terms = *termsOpt;

        auto conceptsOpt = getExperimentConcepts(experiment.dbId, experiment.terms);
        if (!conceptsOpt) return {};
        std::map<int, std::shared_ptr<Concept>> conceptsByDbId;
        for (auto& c : *conceptsOpt) {
            auto ptr = std::make_shared<Concept>(c);
            conceptsByDbId[ptr->dbId] = ptr;
            experiment.concepts[ptr->id] = ptr;
        }

        auto weightsOpt = getExperimentWeights(experiment.dbId, experiment.terms, conceptsByDbId);
        if (!weightsOpt) return {};
        for (auto& w : *weightsOpt) experiment.weights[w.id] = std::make_shared<Weight>(w);

        result.push_back(experiment);
    }
    return result;
}

QList<QString> ModelsRepository::getModelsNames() {
    QStringList modelNames;
    QSqlQuery query(db);
    if (!query.exec("SELECT name FROM models")) { qDebug() << "SQL Error (getModelsNames):" << query.lastError().text() << "Query:" << query.lastQuery(); return modelNames; }
    while (query.next()) modelNames.append(query.value(0).toString());
    return modelNames;
}

std::optional<FCM> ModelsRepository::getModel(const QString modelName) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, description FROM models WHERE name = :name");
    query.bindValue(":name", modelName);
    if (!query.exec() || !query.next()) { qDebug() << "SQL Error (getModel):" << query.lastError().text() << "Query:" << query.lastQuery(); return {}; }

    FCM fcm;
    fcm.dbId = query.value("id").toInt();
    fcm.name = modelName;
    fcm.description = query.value("description").toString();

    auto experimentsOpt = getExperiments(fcm.dbId);
    if (!experimentsOpt) return {};
    auto experiments = *experimentsOpt;
    std::sort(experiments.begin(), experiments.end(), [](const Experiment& a, const Experiment& b) { return a.timestamp < b.timestamp; });

    auto current = experiments.back();
    experiments.pop_back();
    fcm.terms = current.terms;
    fcm.concepts = current.concepts;
    fcm.weights = current.weights;
    fcm.predictionParameters = current.predictionParameters;
    fcm.experiments = experiments;

    return fcm;
}
