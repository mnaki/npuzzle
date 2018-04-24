#pragma once

#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <fstream>
#include <cmath>

#include "State.hpp"
#include "statecomp.hpp"
#include "heuristics.hpp"

class Game
{
  public :

    int   total_inspected_states = 1;
    int   peak_in_memory_states = 1;
    int   moves_count = -1;
    int   abort_threshold = 0;
    State state;

  public :

    std::vector<State> solve(const State & goal_state, heuristic_fn * heuristic);
};
