#include "heuristics.hpp"

int heuristic_dijkstra(const State & state, const State & goal)
{
    (void)state;
    (void)goal;
    return 0;
}

int heuristic_ntiles(const State & state, const State & goal)
{
    int count = 0;

    for (int num : state.tiles) count += state.tiles[num] != goal.tiles[num];

    return count;
}

int heuristic_manhattan(const State & state, const State & goal)
{
    int distance = 0;

    for (int num : state.tiles)
    {
        struct Position goal_pos = state.find_tile(num);
        struct Position current_pos = goal.find_tile(num);
        distance += abs(current_pos.x - goal_pos.x) + abs(current_pos.y - goal_pos.y);
    }

    return distance;
}

int heuristic_euclidean(const State & state, const State & goal)
{
    int distance = 0;

    for (int num : state.tiles)
    {
        struct Position goal_pos = state.find_tile(num);
        struct Position current_pos = goal.find_tile(num);
        distance += sqrt(abs(current_pos.x - goal_pos.x) * abs(current_pos.x - goal_pos.x)) + sqrt(abs(current_pos.y - goal_pos.y) * abs(current_pos.y - goal_pos.y));
    }

    return distance;
}
