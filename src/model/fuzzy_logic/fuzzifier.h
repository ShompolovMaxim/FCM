#pragma once

#include "model/entities/fcm.h"

#include <QListWidget>
#include <map>
#include <memory>

class Fuzzifier {
public:
    Fuzzifier() {}
    virtual ~Fuzzifier() = default;

    virtual Term fuzzify(std::shared_ptr<FCM> fcm, double value) = 0;
};
