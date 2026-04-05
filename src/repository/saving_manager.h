#pragma once

#include "model/entities/fcm.h"
#include "models_repository.h"

#include <QList>
#include <map>
#include <optional>

class SavingManager {
public:
    explicit SavingManager(ModelsRepository repository);

    bool saveAs(FCM &fcm);
    bool saveFCM(FCM &fcm);
    bool deleteFCM(int fcmDbId);

    std::optional<FCM> getFCM(const QString &modelName);
    QList<QString> getModelsNames();

private:
    ModelsRepository repo;
    std::map<int, int> currentExperimentIds;

    bool saveExperiments(FCM &fcm);
    bool saveCurrentExperiment(FCM &fcm, bool applyDeletedIds);
    bool saveExperiment(Experiment &exp, int modelId, const FCM *deletedElementsSource = nullptr);

    bool saveTerms(Experiment &exp, int experimentId);
    bool saveConcepts(Experiment &exp, int experimentId);
    bool saveWeights(Experiment &exp, int experimentId);
    bool deleteExperiments(FCM &fcm);
    bool deleteCurrentElements(const FCM &fcm);
    bool deleteExperimentElements(int experimentId);
    std::optional<int> getCurrentExperimentId(const FCM &fcm);
    void resetFCMDbIds(FCM &fcm);
};
