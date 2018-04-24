#pragma once
#include "State.hpp"

struct statecomp
{
    bool operator() (const State & lhs, const State & rhs) const;
};