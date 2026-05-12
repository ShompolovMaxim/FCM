#pragma once
#include "adapter.h"

#include "model/entities/term.h"

#include <QUuid>

class DefaultColorValueAdapter : public ColorValueAdapter {
public:
    DefaultColorValueAdapter(/*const std::shared_ptr<Term>& terms*/);

    QColor getColor(double value, double min = -1, double max = 1, bool conceptColor = true, bool fuzzyValues = false) override;
private:
    //const std::map<QUuid, std::shared_ptr<Term>>& terms;
};

