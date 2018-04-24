#include "Game.hpp"

std::vector<State> Game::solve(const State & goal_state, heuristic_fn * heuristic)
{
    auto misplaced_tiles = heuristic_ntiles(state, goal_state);
    abort_threshold = misplaced_tiles * 4000;

    
    std::cout << "Automatically generated puzzles may not always be resolvable" << std::endl
              << "We will abort after " << abort_threshold << " tries" << std::endl
              << std::endl << state.to_string() << std::endl;

    std::vector<State> path;
    std::set<State, statecomp> open_list;
    std::set<State, statecomp> closed_list;

    open_list.insert(state);
    std::vector<State> successors;
    std::cout << std::endl;

    while (!open_list.empty() && total_inspected_states < abort_threshold)
    {
        auto it_current_state = std::min_element(open_list.begin(), open_list.end(), [](const State & lhs, const State & rhs){ return lhs.g + lhs.h < rhs.g + rhs.h; });
        auto current_state = *it_current_state;
        open_list.erase(it_current_state);

        if (State::state_cmp(current_state, goal_state) == 0) // Goal
        {
            auto node = std::make_shared<State>(current_state);
            while (node != NULL)
            {
                path.push_back(*node);
                moves_count += 1;
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        total_inspected_states += 1;
        if (total_inspected_states % 1000 == 0)
            std::cout << "." << std::flush;

        successors = current_state.generate_successors();

        int memory_state_count = closed_list.size() + open_list.size() + successors.size();
        if (memory_state_count > peak_in_memory_states)
            peak_in_memory_states = memory_state_count;
        
        auto current_state_ptr = std::make_shared<State>(current_state);
        for (auto & successor : successors)
        {
            successor.g = current_state.g + 1;

            auto exisiting_one_open_it = std::find(open_list.begin(), open_list.end(), successor);
            if (exisiting_one_open_it != open_list.end() && exisiting_one_open_it->g <= successor.g)
                continue;

            auto exisiting_one_closed_it = std::find(closed_list.begin(), closed_list.end(), successor);
            if (exisiting_one_closed_it != closed_list.end() && exisiting_one_closed_it->g <= successor.g)
                continue;

            if (exisiting_one_open_it != open_list.end()) open_list.erase(exisiting_one_open_it);
            if (exisiting_one_closed_it != closed_list.end()) closed_list.erase(exisiting_one_closed_it);

            successor.parent = current_state_ptr;
            successor.h = heuristic(successor, goal_state);
            open_list.insert(successor);
        }
        closed_list.insert(current_state);
    }

    if (open_list.empty())
        std::cout << "Aborting. No solution found." << std::endl;
    else
        std::cout << "Too difficult. Probably not resolvable within reasonable time with given heuristic" << std::endl;

    std::cout << std::endl;

    return path;
}
