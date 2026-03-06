#pragma once

class StopCondition {
public:
    virtual ~StopCondition() = default;

    virtual bool finished() = 0;
};
