#pragma once

#include <vector>
#include "State.hpp"

typedef int (heuristic_fn)(const State & state, const State & goal);

int heuristic_dijkstra(const State & state, const State & goal);
int heuristic_ntiles(const State & state, const State & goal);
int heuristic_manhattan(const State & state, const State & goal);
int heuristic_euclidean(const State & state, const State & goal);