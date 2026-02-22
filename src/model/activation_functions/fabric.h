#pragma once

#include "activation_function.h"
#include "model/element_type.h"

#include <memory.h>
#include <QString>

class ActivationFunctionsFabric
{
public:
    std::shared_ptr<ActivationFunction> create(QString name, ElementType elementType, double fuzzinessDegree);
};
