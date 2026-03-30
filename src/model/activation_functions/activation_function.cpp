#include "activation_function.h"

TriangularFuzzyValue ActivationFunction::activate(TriangularFuzzyValue value) {
    return {
        activate(value.l),
        activate(value.m),
        activate(value.u)
    };
}
