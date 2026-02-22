#pragma once

#include "fuzzifier.h"

class NumericFuzzifier : public Fuzzifier {
public:
    Term fuzzify(std::shared_ptr<FCM> fcm, double value) override;
};
