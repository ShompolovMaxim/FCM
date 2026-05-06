#include "json_repository.h"

#include "model/entities/concept_name_location.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace {

QJsonObject serializePredictionParameters(const PredictionParameters& predictionParameters) {
    QJsonObject params;
    params["algorithm"] = predictionParameters.algorithm;
    params["use_fuzzy_values"] = predictionParameters.useFuzzyValues;
    params["activation"] = predictionParameters.activationFunction;
    params["metric"] = predictionParameters.metric;
    params["predict_to_static"] = predictionParameters.predictToStatic;
    params["threshold"] = predictionParameters.threshold;
    params["steps_less_threshold"] = predictionParameters.stepsLessThreshold;
    params["fixed_steps"] = predictionParameters.fixedSteps;
    params["fuzziness_degree"] = predictionParameters.fuzzinessDegree;
    return params;
}

PredictionParameters deserializePredictionParameters(const QJsonObject& params) {
    PredictionParameters predictionParameters;
    predictionParameters.algorithm = params["algorithm"].toString();
    predictionParameters.useFuzzyValues = params["use_fuzzy_values"].toBool();
    predictionParameters.activationFunction = params["activation"].toString();
    predictionParameters.metric = params["metric"].toString();
    predictionParameters.predictToStatic = params["predict_to_static"].toBool();
    predictionParameters.threshold = params["threshold"].toDouble();
    predictionParameters.stepsLessThreshold = params["steps_less_threshold"].toInt();
    predictionParameters.fixedSteps = params["fixed_steps"].toInt();
    predictionParameters.fuzzinessDegree = params.contains("fuzziness_degree") ? params["fuzziness_degree"].toDouble() : 1.0;
    return predictionParameters;
}

QJsonObject serializeTerm(const std::shared_ptr<Term>& term) {
    QJsonObject jsonTerm;
    jsonTerm["id"] = term->id.toString(QUuid::WithoutBraces);
    jsonTerm["name"] = term->name;
    jsonTerm["description"] = term->description;
    jsonTerm["numeric_value"] = term->value;
    jsonTerm["tr_value_l"] = term->fuzzyValue.l;
    jsonTerm["tr_value_m"] = term->fuzzyValue.m;
    jsonTerm["tr_value_h"] = term->fuzzyValue.u;
    jsonTerm["color_r"] = term->color.red();
    jsonTerm["color_g"] = term->color.green();
    jsonTerm["color_b"] = term->color.blue();
    jsonTerm["color_a"] = term->color.alpha();
    jsonTerm["type"] = elementTypeToString(term->type);
    return jsonTerm;
}

std::shared_ptr<Term> deserializeTerm(const QJsonObject& obj) {
    auto term = std::make_shared<Term>();
    term->id = QUuid(obj["id"].toString());
    term->name = obj["name"].toString();
    term->description = obj["description"].toString();
    term->value = obj["numeric_value"].toDouble();
    term->fuzzyValue.l = obj["tr_value_l"].toDouble();
    term->fuzzyValue.m = obj["tr_value_m"].toDouble();
    term->fuzzyValue.u = obj["tr_value_h"].toDouble();
    term->color = QColor(
        obj["color_r"].toInt(),
        obj["color_g"].toInt(),
        obj["color_b"].toInt(),
        obj.contains("color_a") ? obj["color_a"].toInt() : 255
    );
    term->type = elementTypeFromString(obj["type"].toString());
    term->dbId = -1;
    return term;
}

QJsonObject serializeConcept(const std::shared_ptr<Concept>& concept) {
    QJsonObject jsonConcept;
    jsonConcept["id"] = concept->id.toString(QUuid::WithoutBraces);
    jsonConcept["name"] = concept->name;
    jsonConcept["description"] = concept->description;
    if (concept->term) {
        jsonConcept["term_id"] = concept->term->id.toString(QUuid::WithoutBraces);
    }
    jsonConcept["first_step"] = static_cast<qint64>(concept->startStep);
    jsonConcept["x_pos"] = concept->pos.x();
    jsonConcept["y_pos"] = concept->pos.y();
    jsonConcept["name_location"] = conceptNameLocationToString(concept->nameLocation);
    return jsonConcept;
}

std::shared_ptr<Concept> deserializeConcept(
    const QJsonObject& obj,
    const std::map<QUuid, std::shared_ptr<Term>>& terms
) {
    auto concept = std::make_shared<Concept>();
    concept->id = QUuid(obj["id"].toString());
    concept->name = obj["name"].toString();
    concept->description = obj["description"].toString();
    if (obj.contains("term_id")) {
        concept->term = terms.at(QUuid(obj["term_id"].toString()));
    }
    concept->pos = QPointF(obj["x_pos"].toDouble(), obj["y_pos"].toDouble());
    concept->startStep = static_cast<size_t>(obj["first_step"].toInt());
    concept->nameLocation = obj.contains("name_location")
        ? conceptNameLocationFromString(obj["name_location"].toString())
        : ConceptNameLocation::Up;
    concept->dbId = -1;
    return concept;
}

QJsonObject serializeWeight(const std::shared_ptr<Weight>& weight) {
    QJsonObject jsonWeight;
    jsonWeight["id"] = weight->id.toString(QUuid::WithoutBraces);
    jsonWeight["name"] = weight->name;
    jsonWeight["description"] = weight->description;
    if (weight->term) {
        jsonWeight["term_id"] = weight->term->id.toString(QUuid::WithoutBraces);
    }
    jsonWeight["concept_from_id"] = weight->fromConceptId.toString(QUuid::WithoutBraces);
    jsonWeight["concept_to_id"] = weight->toConceptId.toString(QUuid::WithoutBraces);
    return jsonWeight;
}

std::shared_ptr<Weight> deserializeWeight(
    const QJsonObject& obj,
    const std::map<QUuid, std::shared_ptr<Term>>& terms
) {
    auto weight = std::make_shared<Weight>();
    weight->id = QUuid(obj["id"].toString());
    weight->name = obj["name"].toString();
    weight->description = obj["description"].toString();
    if (obj.contains("term_id")) {
        weight->term = terms.at(QUuid(obj["term_id"].toString()));
    }
    weight->fromConceptId = QUuid(obj["concept_from_id"].toString());
    weight->toConceptId = QUuid(obj["concept_to_id"].toString());
    weight->dbId = -1;
    return weight;
}

QJsonArray serializeTerms(const std::map<QUuid, std::shared_ptr<Term>>& terms) {
    QJsonArray termsArray;
    for (const auto& [id, term] : terms) {
        termsArray.append(serializeTerm(term));
    }
    return termsArray;
}

QJsonArray serializeConcepts(const std::map<QUuid, std::shared_ptr<Concept>>& concepts) {
    QJsonArray conceptsArray;
    for (const auto& [id, concept] : concepts) {
        conceptsArray.append(serializeConcept(concept));
    }
    return conceptsArray;
}

QJsonArray serializeWeights(const std::map<QUuid, std::shared_ptr<Weight>>& weights) {
    QJsonArray weightsArray;
    for (const auto& [id, weight] : weights) {
        weightsArray.append(serializeWeight(weight));
    }
    return weightsArray;
}

std::map<QUuid, std::shared_ptr<Term>> deserializeTerms(const QJsonArray& termsArray) {
    std::map<QUuid, std::shared_ptr<Term>> terms;
    for (const auto& jsonValue : termsArray) {
        auto term = deserializeTerm(jsonValue.toObject());
        terms[term->id] = term;
    }
    return terms;
}

std::map<QUuid, std::shared_ptr<Concept>> deserializeConcepts(
    const QJsonArray& conceptsArray,
    const std::map<QUuid, std::shared_ptr<Term>>& terms
) {
    std::map<QUuid, std::shared_ptr<Concept>> concepts;
    for (const auto& jsonValue : conceptsArray) {
        auto concept = deserializeConcept(jsonValue.toObject(), terms);
        concepts[concept->id] = concept;
    }
    return concepts;
}

std::map<QUuid, std::shared_ptr<Weight>> deserializeWeights(
    const QJsonArray& weightsArray,
    const std::map<QUuid, std::shared_ptr<Term>>& terms
) {
    std::map<QUuid, std::shared_ptr<Weight>> weights;
    for (const auto& jsonValue : weightsArray) {
        auto weight = deserializeWeight(jsonValue.toObject(), terms);
        weights[weight->id] = weight;
    }
    return weights;
}

}

bool JsonRepository::exportToJson(const FCM& fcm, const QString& path) {
    QJsonObject root;

    root["name"] = fcm.name;
    root["description"] = fcm.description;
    root["predictionParameters"] = serializePredictionParameters(fcm.predictionParameters);
    root["terms"] = serializeTerms(fcm.terms);
    root["concepts"] = serializeConcepts(fcm.concepts);
    root["weights"] = serializeWeights(fcm.weights);

    QJsonArray experimentsArray;

    for (const auto& exp : fcm.experiments) {
        QJsonObject e;
        e["predictionParameters"] = serializePredictionParameters(exp.predictionParameters);
        e["timestamp"] = exp.timestamp.toString(Qt::ISODate);
        e["terms"] = serializeTerms(exp.terms);
        e["concepts"] = serializeConcepts(exp.concepts);
        e["weights"] = serializeWeights(exp.weights);

        experimentsArray.append(e);
    }

    root["experiments"] = experimentsArray;

    QJsonDocument doc(root);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson());
    return true;
}

std::optional<FCM> JsonRepository::importFromJson(const QString& path) {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QByteArray data = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return {};
    }

    QJsonObject root = doc.object();

    FCM fcm;

    fcm.name = root["name"].toString();
    fcm.description = root["description"].toString();
    fcm.predictionParameters = deserializePredictionParameters(root["predictionParameters"].toObject());
    fcm.terms = deserializeTerms(root["terms"].toArray());
    fcm.concepts = deserializeConcepts(root["concepts"].toArray(), fcm.terms);
    fcm.weights = deserializeWeights(root["weights"].toArray(), fcm.terms);

    for (auto e : root["experiments"].toArray()) {
        auto obj = e.toObject();

        Experiment exp;
        exp.predictionParameters = deserializePredictionParameters(obj["predictionParameters"].toObject());
        exp.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        exp.dbId = -1;
        if (obj.contains("terms")) {
            exp.terms = deserializeTerms(obj["terms"].toArray());
            exp.concepts = deserializeConcepts(obj["concepts"].toArray(), exp.terms);
            exp.weights = deserializeWeights(obj["weights"].toArray(), exp.terms);
        }

        fcm.experiments.push_back(exp);
    }

    return fcm;
}
