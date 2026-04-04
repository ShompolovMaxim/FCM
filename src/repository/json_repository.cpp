#include "json_repository.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

bool JsonRepository::exportToJson(const FCM& fcm, const QString& path)
{
    QJsonObject root;

    root["name"] = fcm.name;
    root["description"] = fcm.description;

    QJsonObject params;
    params["algorithm"] = fcm.predictionParameters.algorithm;
    params["use_fuzzy_values"] = fcm.predictionParameters.useFuzzyValues;
    params["activation"] = fcm.predictionParameters.activationFunction;
    params["metric"] = fcm.predictionParameters.metric;
    params["predict_to_static"] = fcm.predictionParameters.predictToStatic;
    params["threshold"] = fcm.predictionParameters.threshold;
    params["steps_less_threshold"] = fcm.predictionParameters.stepsLessThreshold;
    params["fixed_steps"] = fcm.predictionParameters.fixedSteps;

    root["predictionParameters"] = params;

    QJsonArray termsArray;

    for (const auto& [id, term] : fcm.terms) {
        QJsonObject t;

        t["id"] = term->id.toString(QUuid::WithoutBraces);
        t["name"] = term->name;
        t["description"] = term->description;

        t["numeric_value"] = term->value;

        t["tr_value_l"] = term->fuzzyValue.l;
        t["tr_value_m"] = term->fuzzyValue.m;
        t["tr_value_h"] = term->fuzzyValue.u;

        t["color_r"] = term->color.red();
        t["color_g"] = term->color.green();
        t["color_b"] = term->color.blue();

        t["type"] = elementTypeToString(term->type);

        termsArray.append(t);
    }

    root["terms"] = termsArray;

    QJsonArray conceptsArray;

    for (const auto& [id, concept] : fcm.concepts) {
        QJsonObject c;

        c["id"] = concept->id.toString(QUuid::WithoutBraces);
        c["name"] = concept->name;
        c["description"] = concept->description;

        if (concept->term) {
            c["term_id"] = concept->term->id.toString(QUuid::WithoutBraces);
        }

        c["first_step"] = static_cast<qint64>(concept->startStep);

        c["x_pos"] = concept->pos.x();
        c["y_pos"] = concept->pos.y();

        conceptsArray.append(c);
    }

    root["concepts"] = conceptsArray;

    QJsonArray weightsArray;

    for (const auto& [id, weight] : fcm.weights) {
        QJsonObject w;

        w["id"] = weight->id.toString(QUuid::WithoutBraces);
        w["name"] = weight->name;
        w["description"] = weight->description;

        if (weight->term) {
            w["term_id"] = weight->term->id.toString(QUuid::WithoutBraces);
        }

        w["concept_from_id"] = weight->fromConceptId.toString(QUuid::WithoutBraces);
        w["concept_to_id"] = weight->toConceptId.toString(QUuid::WithoutBraces);

        weightsArray.append(w);
    }

    root["weights"] = weightsArray;

    QJsonArray experimentsArray;

    for (const auto& exp : fcm.experiments) {
        QJsonObject e;

        QJsonObject expParams;
        expParams["algorithm"] = exp.predictionParameters.algorithm;
        expParams["use_fuzzy_values"] = exp.predictionParameters.useFuzzyValues;
        expParams["activation"] = exp.predictionParameters.activationFunction;
        expParams["metric"] = exp.predictionParameters.metric;
        expParams["predict_to_static"] = exp.predictionParameters.predictToStatic;
        expParams["threshold"] = exp.predictionParameters.threshold;
        expParams["steps_less_threshold"] = exp.predictionParameters.stepsLessThreshold;
        expParams["fixed_steps"] = exp.predictionParameters.fixedSteps;

        e["predictionParameters"] = expParams;
        e["timestamp"] = exp.timestamp.toString(Qt::ISODate);

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

std::optional<FCM> JsonRepository::importFromJson(const QString& path)
{
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

    auto params = root["predictionParameters"].toObject();

    fcm.predictionParameters.algorithm = params["algorithm"].toString();
    fcm.predictionParameters.useFuzzyValues = params["use_fuzzy_values"].toBool();
    fcm.predictionParameters.activationFunction = params["activation"].toString();
    fcm.predictionParameters.metric = params["metric"].toString();
    fcm.predictionParameters.predictToStatic = params["predict_to_static"].toBool();
    fcm.predictionParameters.threshold = params["threshold"].toDouble();
    fcm.predictionParameters.stepsLessThreshold = params["steps_less_threshold"].toInt();
    fcm.predictionParameters.fixedSteps = params["fixed_steps"].toInt();

    for (auto t : root["terms"].toArray()) {
        auto obj = t.toObject();

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
            obj["color_b"].toInt()
            );

        term->type = elementTypeFromString(obj["type"].toString());
        term->dbId = -1;

        fcm.terms[term->id] = term;
    }

    for (auto c : root["concepts"].toArray()) {
        auto obj = c.toObject();

        auto concept = std::make_shared<Concept>();

        concept->id = QUuid(obj["id"].toString());
        concept->name = obj["name"].toString();
        concept->description = obj["description"].toString();

        if (obj.contains("term_id")) {
            QUuid termId(obj["term_id"].toString());
            concept->term = fcm.terms.at(termId);
        }

        concept->pos = QPointF(
            obj["x_pos"].toDouble(),
            obj["y_pos"].toDouble()
            );

        concept->startStep = static_cast<size_t>(obj["first_step"].toInt());
        concept->dbId = -1;

        fcm.concepts[concept->id] = concept;
    }

    for (auto w : root["weights"].toArray()) {
        auto obj = w.toObject();

        auto weight = std::make_shared<Weight>();

        weight->id = QUuid(obj["id"].toString());
        weight->name = obj["name"].toString();
        weight->description = obj["description"].toString();

        if (obj.contains("term_id")) {
            QUuid termId(obj["term_id"].toString());
            weight->term = fcm.terms.at(termId);
        }

        weight->fromConceptId = QUuid(obj["concept_from_id"].toString());
        weight->toConceptId = QUuid(obj["concept_to_id"].toString());

        weight->dbId = -1;

        fcm.weights[weight->id] = weight;
    }

    for (auto e : root["experiments"].toArray()) {
        auto obj = e.toObject();

        Experiment exp;

        auto expParams = obj["predictionParameters"].toObject();

        exp.predictionParameters.algorithm = expParams["algorithm"].toString();
        exp.predictionParameters.useFuzzyValues = expParams["use_fuzzy_values"].toBool();
        exp.predictionParameters.activationFunction = expParams["activation"].toString();
        exp.predictionParameters.metric = expParams["metric"].toString();
        exp.predictionParameters.predictToStatic = expParams["predict_to_static"].toBool();
        exp.predictionParameters.threshold = expParams["threshold"].toDouble();
        exp.predictionParameters.stepsLessThreshold = expParams["steps_less_threshold"].toInt();
        exp.predictionParameters.fixedSteps = expParams["fixed_steps"].toInt();

        exp.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        exp.dbId = -1;

        fcm.experiments.push_back(exp);
    }

    return fcm;
}
