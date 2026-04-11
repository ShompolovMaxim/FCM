#pragma once

#include "mode.h"

#include "model/entities/fcm.h"
#include "model/fuzzy_logic/linear_fuzzifier.h"

#include <vector>

#include <QUuid>

class ModelsJoiner {
public:
    ModelsJoiner();

    std::shared_ptr<FCM> join(const std::shared_ptr<FCM> baseFCM, const std::vector<std::shared_ptr<FCM>>& fcms, const JoinMode& joinMode, const QString& resultName);

private:
    std::shared_ptr<Fuzzifier> fuzzifier = std::make_shared<LinearFuzzifier>();
};
