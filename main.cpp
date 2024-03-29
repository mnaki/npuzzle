#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <fstream>
#include <cmath>

#include "main.hpp"

State generate_random_puzzle(int width, int height)
{
    if (width < 3 || height < 3)
    {
        throw std::runtime_error("too small");
    }

    auto s(State(width, height).sort_tiles());
    srand(time(NULL));
    int iterations = 4;
    struct Position gap;
    int swipe_count = 0;
    enum e_swipe_direction last_move = SWIPE_NO;

    while (swipe_count < iterations)
    {
        gap = s.find_tile(EMPTY_TILE);
        if (rand() % 2 == 0 && last_move != SWIPE_RIGHT && gap.x >= 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[gap.y * width + gap.x - 1]); last_move = SWIPE_LEFT; swipe_count++;
        }
        gap = s.find_tile(EMPTY_TILE);
        if (rand() % 2 == 0 && last_move != SWIPE_DOWN && gap.y >= 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[(gap.y - 1) * width + gap.x]); last_move = SWIPE_UP; swipe_count++;
        }
        gap = s.find_tile(EMPTY_TILE);
        if (rand() % 2 == 0 && last_move != SWIPE_LEFT && gap.x < s.width - 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[gap.y * width + gap.x + 1]); last_move = SWIPE_RIGHT; swipe_count++;
        }
        gap = s.find_tile(EMPTY_TILE);
        if (rand() % 2 == 0 && last_move != SWIPE_UP && gap.y < s.height - 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[(gap.y + 1) * width + gap.x]); last_move = SWIPE_DOWN; swipe_count++;
        }
    }
    return s;
}

std::vector<std::string> split(std::string const & str)
{
    std::string buf;
    std::stringstream ss(str);
    std::vector<std::string> tokens;

    while (ss >> buf)
    {
        tokens.push_back(buf);
    }

    return tokens;
}

heuristic_fn * select_heuristic(char **av)
{
         if (std::string(av[1]) == "manhattan") return &heuristic_manhattan;
    else if (std::string(av[1]) == "ntiles")    return &heuristic_ntiles;
    else if (std::string(av[1]) == "dijkstra")  return &heuristic_dijkstra;
    else if (std::string(av[1]) == "euclidean") return &heuristic_euclidean;
    
    throw std::runtime_error("invalid heuristic");
}

State parse_file(char **av)
{
    State state;
    std::ifstream file(av[2]);
    std::string str;
    std::vector<int> v;

    if (!file.good())
    {
        throw std::runtime_error("file error");
    }

    int found_comment = false;
    size_t size = 0;

    while (std::getline(file, str))
    {
        auto pos = str.find("#");

        if (pos != std::string::npos)
        {
            found_comment = true;
            while (found_comment && pos < str.size())
            {
                if ((str[pos]) == '\n' && str[pos])
                {
                    found_comment = false;
                }
                (str[pos]) = ' ';
                pos++;
            }
        }

        if (size == 0 && str[0] >= '0' && str[0] <= '9')
        {
            size = std::stoi(str);
        }
        else if (str[0])
        {
            for (auto & n : split(str))
            {
                v.push_back(std::stoi(n));
            }
        }
    }

    if (size == 0)
    {
        throw std::runtime_error("invalid file");
    }

    state = State(size, size);
    size_t x = 0;
    size_t y = 0;

    for (auto & num : v)
    {
        if (x == size)
        {
            x = 0;
            y++;
        }
        state.tiles[y * state.width + x] = num;
        x++;
    }

    return state;
}

int main(int ac, char **av)
{
    try
    {
        if (ac < 3)
        {
            std::cout << "./npuzzle ntiles|manhattan|euclidean|dijkstra [puzzle.txt]|[WIDTH HEIGHT]" << std::endl;

            std::cout << std::endl << "/!\\ Automatically generated puzzles may not always be resolvable /!\\" << std::endl;
            exit(0);
        }

        Game game;

        if (ac == 3)
        {
            game.state = parse_file(av);
        }
        else if (ac == 4)
        {
            game.state = generate_random_puzzle(std::stoi(av[2]), std::stoi(av[3]));
        }

        if (!game.state.checkResolvability())
        {
            if (ac < 4)
                throw std::runtime_error("That puzzle you fed me is not solvable.");
            else
                throw std::runtime_error("My autogenerated puzzle is not solvable. Try again.");
        }

        heuristic_fn * h = select_heuristic(av);

        auto goal = game.state.sort_tiles();
        auto path = game.solve(goal, h);

        for (auto & step : path)
        {
            std::cout << step.to_string() << std::endl;
        }

        std::cout << "game.abort_threshold = " << game.abort_threshold << std::endl;
        std::cout << "game.total_inspected_states = " << game.total_inspected_states << std::endl;
        std::cout << "game.peak_in_memory_states  = " << game.peak_in_memory_states << std::endl;
        std::cout << "game.moves_count            = ";
        if (game.moves_count != -1)
            std::cout << game.moves_count;
        else
            std::cout << "N/A";
        std::cout << std::endl;
    }
    catch (std::exception const & e)
    {
        std::cout << "Error: " << e.what() << "" << std::endl;
        exit(1);
    }
    return 0;
}