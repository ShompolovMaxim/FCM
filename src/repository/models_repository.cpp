#include "models_repository.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

ModelsRepository::ModelsRepository(QSqlDatabase database) : db(database) {}

bool ModelsRepository::transaction() {
    return db.transaction();
}

bool ModelsRepository::commit() {
    return db.commit();
}

bool ModelsRepository::rollback() {
    return db.rollback();
}

std::optional<int> ModelsRepository::createModel(FCM &fcm) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO models (name,description,autosave_on) VALUES (:name,:description,:autosave_on)");
    query.bindValue(":name", fcm.name);
    query.bindValue(":description", fcm.description);
    query.bindValue(":autosave_on", fcm.autosaveOn);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }
    fcm.dbId = query.lastInsertId().toInt();
    return fcm.dbId;
}

bool ModelsRepository::updateModel(const FCM &fcm) {
    QSqlQuery query(db);
    query.prepare("UPDATE models SET name=:name,description=:description,autosave_on=:autosave_on WHERE id=:id");
    query.bindValue(":name", fcm.name);
    query.bindValue(":description", fcm.description);
    query.bindValue(":autosave_on", fcm.autosaveOn);
    query.bindValue(":id", fcm.dbId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool ModelsRepository::deleteModel(int modelId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM models WHERE id=:id");
    query.bindValue(":id", modelId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

std::optional<int> ModelsRepository::createExperiment(Experiment &experiment, int modelId) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO experiments "
        "(model_id,timestamp,algorithm,use_fuzzy_values,activation,predict_to_static,"
        "metric,threshold,steps_less_threshold,fixed_steps) "
        "VALUES (:model_id,:timestamp,:algorithm,:use_fuzzy_values,:activation,"
        ":predict_to_static,:metric,:threshold,:steps_less_threshold,:fixed_steps)"
    );
    query.bindValue(":model_id", modelId);
    query.bindValue(":timestamp", experiment.timestamp);
    query.bindValue(":algorithm", experiment.predictionParameters.algorithm);
    query.bindValue(":use_fuzzy_values", experiment.predictionParameters.useFuzzyValues);
    query.bindValue(":activation", experiment.predictionParameters.activationFunction);
    query.bindValue(":predict_to_static", experiment.predictionParameters.predictToStatic);
    query.bindValue(":metric", experiment.predictionParameters.metric);
    query.bindValue(":threshold", experiment.predictionParameters.threshold);
    query.bindValue(":steps_less_threshold", experiment.predictionParameters.stepsLessThreshold);
    query.bindValue(":fixed_steps", experiment.predictionParameters.fixedSteps);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }
    experiment.dbId = query.lastInsertId().toInt();
    return experiment.dbId;
}

bool ModelsRepository::updateExperiment(const Experiment &experiment) {
    QSqlQuery query(db);
    query.prepare(
        "UPDATE experiments SET "
        "algorithm=:algorithm,use_fuzzy_values=:use_fuzzy_values,activation=:activation,"
        "predict_to_static=:predict_to_static,metric=:metric,"
        "threshold=:threshold,steps_less_threshold=:steps_less_threshold,fixed_"
        "steps=:fixed_steps,timestamp=:timestamp WHERE id=:id"
    );
    query.bindValue(":algorithm", experiment.predictionParameters.algorithm);
    query.bindValue(":use_fuzzy_values", experiment.predictionParameters.useFuzzyValues);
    query.bindValue(":activation", experiment.predictionParameters.activationFunction);
    query.bindValue(":predict_to_static", experiment.predictionParameters.predictToStatic);
    query.bindValue(":metric", experiment.predictionParameters.metric);
    query.bindValue(":threshold", experiment.predictionParameters.threshold);
    query.bindValue(":steps_less_threshold", experiment.predictionParameters.stepsLessThreshold);
    query.bindValue(":fixed_steps", experiment.predictionParameters.fixedSteps);
    query.bindValue(":timestamp", experiment.timestamp);
    query.bindValue(":id", experiment.dbId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool ModelsRepository::deleteExperiment(int experimentId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM experiments WHERE id=:id");
    query.bindValue(":id", experimentId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

std::optional<int> ModelsRepository::createTerm(Term &term, int experimentId) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO terms (uuid,name,description,experiment_id,numeric_value,tr_value_l,tr_value_m,tr_value_h,color_r,color_g,color_b,type) "
        "VALUES (:uuid,:name,:description,:experiment_id,:numeric_value,:tr_value_l,:tr_value_m,:tr_value_h,:color_r,:color_g,:color_b,:type)"
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
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }
    term.dbId = query.lastInsertId().toInt();
    return term.dbId;
}

bool ModelsRepository::updateTerm(const Term &term) {
    QSqlQuery query(db);
    query.prepare(
        "UPDATE terms SET name=:name,description=:description,numeric_value=:numeric_value,"
        "tr_value_l=:tr_value_l,tr_value_m=:tr_value_m,tr_value_h=:tr_value_h,"
        "color_r=:color_r,color_g=:color_g,color_b=:color_b,type=:type WHERE id=:id");
    query.bindValue(":name", term.name);
    query.bindValue(":description", term.description);
    query.bindValue(":numeric_value", term.value);
    query.bindValue(":tr_value_l", term.fuzzyValue.l);
    query.bindValue(":tr_value_m", term.fuzzyValue.m);
    query.bindValue(":tr_value_h", term.fuzzyValue.u);
    query.bindValue(":color_r", term.color.red());
    query.bindValue(":color_g", term.color.green());
    query.bindValue(":color_b", term.color.blue());
    query.bindValue(":type", elementTypeToString(term.type));
    query.bindValue(":id", term.dbId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool ModelsRepository::deleteTerm(int termId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM terms WHERE id=:id");
    query.bindValue(":id", termId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

std::optional<int> ModelsRepository::createConcept(Concept &concept, int experimentId, const std::optional<int> &dbTermId) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO concepts (uuid,name,description,experiment_id,term_id,first_step,x_pos,y_pos) "
        "VALUES (:uuid,:name,:description,:experiment_id,:term_id,:first_step,:x_pos,:y_pos)");
    query.bindValue(":uuid", concept.id);
    query.bindValue(":name", concept.name);
    query.bindValue(":description", concept.description);
    query.bindValue(":experiment_id", experimentId);
    if (dbTermId.has_value())
        query.bindValue(":term_id", dbTermId.value());
    else
        query.bindValue(":term_id", QVariant());
    query.bindValue(":first_step", concept.startStep);
    query.bindValue(":x_pos", concept.pos.x());
    query.bindValue(":y_pos", concept.pos.y());
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }
    concept.dbId = query.lastInsertId().toInt();
    return concept.dbId;
}

bool ModelsRepository::updateConcept(const Concept &concept) {
    QSqlQuery query(db);
    query.prepare(
        "UPDATE concepts SET "
        "name=:name,description=:description,term_id=:term_id,"
        "first_step=:first_step,x_pos=:x_pos,y_pos=:y_pos WHERE id=:id");
    query.bindValue(":name", concept.name);
    query.bindValue(":description", concept.description);
    if (concept.term)
        query.bindValue(":term_id", concept.term->dbId);
    else
        query.bindValue(":term_id", QVariant());
    query.bindValue(":first_step", concept.startStep);
    query.bindValue(":x_pos", concept.pos.x());
    query.bindValue(":y_pos", concept.pos.y());
    query.bindValue(":id", concept.dbId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool ModelsRepository::deleteConcept(int conceptId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM concepts WHERE id=:id");
    query.bindValue(":id", conceptId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

std::optional<int> ModelsRepository::createWeight(Weight &weight, int experimentId, int fromConceptId, int toConceptId, const std::map<QUuid, int> &termsDBIds) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO weights (uuid,name,description,experiment_id,term_id,concept_from_id,concept_to_id) "
        "VALUES (:uuid,:name,:description,:experiment_id,:term_id,:concept_from_id,:concept_to_id)");
    query.bindValue(":uuid", weight.id);
    query.bindValue(":name", weight.name);
    query.bindValue(":description", weight.description);
    query.bindValue(":experiment_id", experimentId);
    if (weight.term)
        query.bindValue(":term_id", termsDBIds.at(weight.term->id));
    else
        query.bindValue(":term_id", QVariant());
    query.bindValue(":concept_from_id", fromConceptId);
    query.bindValue(":concept_to_id", toConceptId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }
    weight.dbId = query.lastInsertId().toInt();
    return weight.dbId;
}

bool ModelsRepository::updateWeight(const Weight &weight) {
    QSqlQuery query(db);
    query.prepare(
        "UPDATE weights SET name=:name,description=:description,term_id=:term_id "
        "WHERE id=:id");
    query.bindValue(":name", weight.name);
    query.bindValue(":description", weight.description);
    if (weight.term)
        query.bindValue(":term_id", weight.term->dbId);
    else
        query.bindValue(":term_id", QVariant());
    query.bindValue(":id", weight.dbId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

bool ModelsRepository::deleteWeight(int weightId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM weights WHERE id=:id");
    query.bindValue(":id", weightId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

QList<QString> ModelsRepository::getModelsNames() {
    QStringList modelNames;
    QSqlQuery query(db);
    if (!query.exec("SELECT name FROM models")) return modelNames;
    while (query.next()) modelNames.append(query.value(0).toString());
    return modelNames;
}

std::optional<FCM> ModelsRepository::getModel(const QString &modelName) {
    QSqlQuery query(db);
    query.prepare("SELECT id,name,description,autosave_on FROM models WHERE name=:name");
    query.bindValue(":name", modelName);
    if (!query.exec() || !query.next()) return {};
    FCM fcm;
    fcm.dbId = query.value("id").toInt();
    fcm.name = query.value("name").toString();
    fcm.description = query.value("description").toString();
    fcm.autosaveOn = query.value("autosave_on").toBool();
    auto experimentsOpt = getExperiments(fcm.dbId);
    if (!experimentsOpt) return {};
    auto experiments = *experimentsOpt;
    if (experiments.empty()) return {};
    std::sort(experiments.begin(), experiments.end(), [](const Experiment &a, const Experiment &b) { return a.timestamp < b.timestamp; });
    auto current = experiments.back();
    experiments.pop_back();
    fcm.terms = current.terms;
    fcm.concepts = current.concepts;
    fcm.weights = current.weights;
    fcm.predictionParameters = current.predictionParameters;
    fcm.experiments = experiments;
    return fcm;
}

std::optional<std::vector<Experiment>> ModelsRepository::getExperiments(int modelId) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT "
        "id,timestamp,algorithm,activation,predict_to_static,metric,"
        "threshold,steps_less_threshold,fixed_steps,use_fuzzy_values "
        "FROM experiments WHERE model_id=:model_id");
    query.bindValue(":model_id", modelId);
    if (!query.exec()) return {};
    std::vector<Experiment> result;
    while (query.next()) {
        Experiment experiment;
        experiment.dbId = query.value("id").toInt();
        experiment.timestamp = query.value("timestamp").toDateTime();
        experiment.predictionParameters = {
            query.value("algorithm").toString(),
            query.value("use_fuzzy_values").toBool(),
            query.value("activation").toString(),
            query.value("metric").toString(),
            query.value("predict_to_static").toBool(),
            query.value("threshold").toDouble(),
            query.value("steps_less_threshold").toInt(),
            query.value("fixed_steps").toInt()
        };
        auto termsOpt = getExperimentTerms(experiment.dbId);
        if (!termsOpt) return {};
        experiment.terms = *termsOpt;
        auto conceptsOpt = getExperimentConcepts(experiment.dbId, experiment.terms);
        if (!conceptsOpt) return {};
        std::map<int, std::shared_ptr<Concept>> conceptsByDbId;
        for (auto &c : *conceptsOpt) {
            auto ptr = std::make_shared<Concept>(c);
            conceptsByDbId[ptr->dbId] = ptr;
            experiment.concepts[ptr->id] = ptr;
        }
        auto weightsOpt = getExperimentWeights(experiment.dbId, experiment.terms, conceptsByDbId);
        if (!weightsOpt) return {};
        for (auto &w : *weightsOpt) experiment.weights[w.id] = std::make_shared<Weight>(w);
        result.push_back(experiment);
    }
    return result;
}

std::optional<std::vector<std::pair<int, QDateTime>>> ModelsRepository::getExperimentsInfo(int modelId) {
    QSqlQuery query(db);
    query.prepare("SELECT id,timestamp FROM experiments WHERE model_id=:model_id");
    query.bindValue(":model_id", modelId);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return {};
    }

    std::vector<std::pair<int, QDateTime>> result;
    while (query.next())
        result.emplace_back(query.value("id").toInt(), query.value("timestamp").toDateTime());

    return result;
}

std::optional<std::map<QUuid, std::shared_ptr<Term>>> ModelsRepository::getExperimentTerms(int experimentId) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT "
        "id,uuid,name,description,numeric_value,tr_value_l,tr_value_m,"
        "tr_value_h,color_r,color_g,color_b,type FROM terms WHERE "
        "experiment_id=:experiment_id");
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) return {};
    std::map<QUuid, std::shared_ptr<Term>> result;
    while (query.next()) {
        auto term = std::make_shared<Term>(Term{
            query.value("uuid").toUuid(),
            query.value("name").toString(),
            query.value("description").toString(),
            query.value("numeric_value").toDouble(),
            query.value("tr_value_l").toDouble(),
            query.value("tr_value_m").toDouble(),
            query.value("tr_value_h").toDouble(),
            QColor(query.value("color_r").toInt(), query.value("color_g").toInt(), query.value("color_b").toInt()),
            elementTypeFromString(query.value("type").toString())
        });
        term->dbId = query.value("id").toInt();
        result[term->id] = term;
    }
    return result;
}

std::optional<std::vector<Concept>> ModelsRepository::getExperimentConcepts(int experimentId, const std::map<QUuid, std::shared_ptr<Term>> &terms) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id,uuid,name,description,term_id,first_step,x_pos,y_pos FROM "
        "concepts WHERE experiment_id=:experiment_id");
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) return {};
    std::vector<Concept> result;
    while (query.next()) {
        std::shared_ptr<Term> termPtr = nullptr;
        auto termIdValue = query.value("term_id");
        if (!termIdValue.isNull()) {
            int dbTermId = termIdValue.toInt();
            for (const auto &[_, t] : terms)
                if (t->dbId == dbTermId) termPtr = t;
        }
        Concept concept {
            query.value("uuid").toUuid(),
            query.value("name").toString(),
            query.value("description").toString(),
            termPtr,
            QPointF(query.value("x_pos").toDouble(), query.value("y_pos").toDouble()),
            query.value("first_step").toUInt()
        };
        concept.dbId = query.value("id").toInt();
        result.emplace_back(concept);
    }
    return result;
}

std::optional<std::vector<Weight>> ModelsRepository::getExperimentWeights(
    int experimentId,
    const std::map<QUuid, std::shared_ptr<Term>> &terms,
    const std::map<int, std::shared_ptr<Concept>> &conceptsByDbId
) {
    QSqlQuery query(db);
    query.prepare(
        "SELECT id,uuid,name,description,term_id,concept_from_id,concept_to_id "
        "FROM weights WHERE experiment_id=:experiment_id");
    query.bindValue(":experiment_id", experimentId);
    if (!query.exec()) return {};
    std::vector<Weight> result;
    while (query.next()) {
        std::shared_ptr<Term> termPtr = nullptr;
        auto termIdValue = query.value("term_id");
        if (!termIdValue.isNull()) {
            int dbTermId = termIdValue.toInt();
            for (const auto &[_, t] : terms)
                if (t->dbId == dbTermId) {
                    termPtr = t;
                    break;
                }
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
