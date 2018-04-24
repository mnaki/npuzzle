#include "statecomp.hpp"

bool statecomp::operator() (const State & lhs, const State & rhs) const
{
    return lhs < rhs;
}
