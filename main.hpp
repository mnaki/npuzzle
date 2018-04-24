#pragma once

#include "Game.hpp"
#include "State.hpp"

State sort_tiles(const State & state);
State generate_random_puzzle(int width, int height);
std::vector<std::string> split(std::string const & str);
heuristic_fn * select_heuristic(char **av);
State parse_file(char **av);