#pragma once

#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <fstream>
#include <cmath>

#define EMPTY_TILE 0
#include "Position.hpp"

enum e_swipe_direction { SWIPE_NO = 0, SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT };

class State
{
  public :

    int                    g = 0;
    int                    h = 0;
    short                  width = 0;
    short                  height = 0;
    std::vector<int>       tiles;
    std::shared_ptr<State> parent = NULL;
    e_swipe_direction      swipe_direction = SWIPE_NO;
    int                    swiped_number = 0;

    State();
    ~State();
    State(int width, int height);
    State(State const & rhs);

    std::string to_string(void) const;
    State &     operator=(State const & rhs);
    bool        operator==(State const & rhs) const;
    bool        operator<(State const & rhs) const;

    bool        checkResolvability() const;

    static int    state_cmp(const State & lhs, const State & rhs);

    inline State &       swipeLeft(const Position & gap);
    inline State &       swipeUp(const Position & gap);
    inline State &       swipeRight(const Position & gap);
    inline State &       swipeDown(const Position & gap);

    std::vector<State> generate_successors(void) const;
    Position           find_tile(int number) const;
    State              sort_tiles() const;
};
