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
    params["activation"] = fcm.predictionParameters.activationFunction;
    params["metric"] = fcm.predictionParameters.metric;
    params["predict_to_static"] = fcm.predictionParameters.predictToStatic;
    params["threshold"] = fcm.predictionParameters.threshold;
    params["steps_less_threshold"] = fcm.predictionParameters.stepsLessThreshold;
    params["fixed_steps"] = fcm.predictionParameters.fixedSteps;

    root["predictionParameters"] = params;

    QJsonArray termsArray;

    for (const auto& [id, term] : fcm.terms)
    {
        QJsonObject t;

        t["id"] = static_cast<qint64>(term.id);
        t["name"] = term.name;
        t["description"] = term.description;

        t["numeric_value"] = term.value;

        t["tr_value_l"] = term.fuzzyValueL;
        t["tr_value_m"] = term.fuzzyValueM;
        t["tr_value_h"] = term.fuzzyValueU;

        t["color_r"] = term.color.red();
        t["color_g"] = term.color.green();
        t["color_b"] = term.color.blue();

        termsArray.append(t);
    }

    root["terms"] = termsArray;

    QJsonArray conceptsArray;

    for (const auto& [id, concept] : fcm.concepts)
    {
        QJsonObject c;

        c["id"] = static_cast<qint64>(concept->id);
        c["name"] = concept->name;
        c["description"] = concept->description;

        c["value"] = concept->value;

        c["first_step"] = static_cast<qint64>(concept->startStep);

        c["x_pos"] = concept->pos.x();
        c["y_pos"] = concept->pos.y();

        conceptsArray.append(c);
    }

    root["concepts"] = conceptsArray;

    QJsonArray weightsArray;

    for (const auto& [id, weight] : fcm.weights)
    {
        QJsonObject w;

        w["id"] = static_cast<qint64>(weight.id);
        w["name"] = weight.name;
        w["description"] = weight.description;

        w["value"] = weight.value;

        w["concept_from_id"] = static_cast<qint64>(weight.fromConceptId);
        w["concept_to_id"] = static_cast<qint64>(weight.toConceptId);

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
    fcm.predictionParameters.activationFunction = params["activation"].toString();
    fcm.predictionParameters.metric = params["metric"].toString();
    fcm.predictionParameters.predictToStatic = params["predict_to_static"].toBool();
    fcm.predictionParameters.threshold = params["threshold"].toDouble();
    fcm.predictionParameters.stepsLessThreshold = params["steps_less_threshold"].toInt();
    fcm.predictionParameters.fixedSteps = params["fixed_steps"].toInt();

    for (auto t : root["terms"].toArray())
    {
        auto obj = t.toObject();

        Term term{
            static_cast<size_t>(obj["id"].toInt()),
            obj["name"].toString(),
            obj["description"].toString(),
            obj["numeric_value"].toDouble(),
            obj["tr_value_l"].toDouble(),
            obj["tr_value_m"].toDouble(),
            obj["tr_value_h"].toDouble(),
            QColor(
                obj["color_r"].toInt(),
                obj["color_g"].toInt(),
                obj["color_b"].toInt()
                )
        };

        fcm.terms[term.id] = term;
    }

    for (auto c : root["concepts"].toArray())
    {
        auto obj = c.toObject();

        Concept concept{
            static_cast<size_t>(obj["id"].toInt()),
            obj["name"].toString(),
            obj["description"].toString(),
            obj["value"].toDouble(),
            QPointF(
                obj["x_pos"].toDouble(),
                obj["y_pos"].toDouble()
                ),
            static_cast<size_t>(obj["first_step"].toInt())
        };

        fcm.concepts[concept.id] = std::make_shared<Concept>(concept);
    }

    for (auto w : root["weights"].toArray())
    {
        auto obj = w.toObject();

        Weight weight{
            static_cast<size_t>(obj["id"].toInt()),
            obj["name"].toString(),
            obj["description"].toString(),
            obj["value"].toDouble(),
            static_cast<size_t>(obj["concept_from_id"].toInt()),
            static_cast<size_t>(obj["concept_to_id"].toInt())
        };

        fcm.weights[weight.id] = weight;
    }

    return fcm;
}
