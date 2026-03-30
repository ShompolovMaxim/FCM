#include "json_repository.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

bool JsonRepository::exportToJson(const FCM& fcm, const QString& path) {
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

        t["id"] = static_cast<qint64>(term->id);
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

        c["id"] = static_cast<qint64>(concept->id);
        c["name"] = concept->name;
        c["description"] = concept->description;

        if (concept->term)
            c["term_id"] = static_cast<qint64>(concept->term->id);
        else
            c["term_id"] = QJsonValue();

        c["first_step"] = static_cast<qint64>(concept->startStep);

        c["x_pos"] = concept->pos.x();
        c["y_pos"] = concept->pos.y();

        conceptsArray.append(c);
    }

    root["concepts"] = conceptsArray;

    QJsonArray weightsArray;

    for (const auto& [id, weightPtr] : fcm.weights) {
        QJsonObject w;

        w["id"] = static_cast<qint64>(weightPtr->id);
        w["name"] = weightPtr->name;
        w["description"] = weightPtr->description;

        if (weightPtr->term)
            w["term_id"] = static_cast<qint64>(weightPtr->term->id);
        else
            w["term_id"] = QJsonValue();

        w["concept_from_id"] = static_cast<qint64>(weightPtr->fromConceptId);
        w["concept_to_id"] = static_cast<qint64>(weightPtr->toConceptId);

        weightsArray.append(w);
    }

    root["weights"] = weightsArray;

    QJsonDocument doc(root);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson());
    return true;
}

std::optional<FCM> JsonRepository::importFromJson(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return {};

    QByteArray data = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return {};

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

    for (auto t : root["terms"].toArray())
    {
        auto obj = t.toObject();

        auto term = std::make_shared<Term>(Term{
            static_cast<size_t>(obj["id"].toInt()),
            obj["name"].toString(),
            obj["description"].toString(),
            obj["numeric_value"].toDouble(),
            obj["tr_value_l"].toDouble(),
            obj["tr_value_m"].toDouble(),
            obj["tr_value_h"].toDouble(),
            QColor(obj["color_r"].toInt(), obj["color_g"].toInt(), obj["color_b"].toInt()),
            elementTypeFromString(obj["type"].toString())
        });

        fcm.terms[term->id] = term;
    }

    for (auto c : root["concepts"].toArray())
    {
        auto obj = c.toObject();

        std::shared_ptr<Term> termPtr = nullptr;

        if (!obj["term_id"].isNull()) {
            size_t termId = static_cast<size_t>(obj["term_id"].toInt());
            termPtr = fcm.terms.at(termId);
        }

        auto concept = std::make_shared<Concept>(Concept{
            static_cast<size_t>(obj["id"].toInt()),
            obj["name"].toString(),
            obj["description"].toString(),
            termPtr,
            QPointF(
                obj["x_pos"].toDouble(),
                obj["y_pos"].toDouble()
                ),
            static_cast<size_t>(obj["first_step"].toInt())
        });

        fcm.concepts[concept->id] = concept;
    }

    for (auto w : root["weights"].toArray())
    {
        auto obj = w.toObject();

        std::shared_ptr<Term> termPtr = nullptr;

        if (!obj["term_id"].isNull()) {
            size_t termId = static_cast<size_t>(obj["term_id"].toInt());
            termPtr = fcm.terms.at(termId);
        }

        auto weight = std::make_shared<Weight>(Weight{
            static_cast<size_t>(obj["id"].toInt()),
            obj["name"].toString(),
            obj["description"].toString(),
            termPtr,
            static_cast<size_t>(obj["concept_from_id"].toInt()),
            static_cast<size_t>(obj["concept_to_id"].toInt())
        });

        fcm.weights[weight->id] = weight;
    }

    return fcm;
}
