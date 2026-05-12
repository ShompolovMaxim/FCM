#pragma once

#include "activation_function.h"
#include "model/entities/element_type.h"

#include <memory.h>
#include <QString>

class ActivationFunctionsFactory
{
public:
    std::shared_ptr<ActivationFunction> create(QString name, ElementType elementType, double fuzzinessDegree);
};

