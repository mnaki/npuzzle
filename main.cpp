#include <unistd.h>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <utility>
#include <sstream>
#include <ctime>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <deque>
#include <fstream>
#include <cmath>
#include <cstring>

enum e_swipe_direction { SWIPE_NO = 0, SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT };

class State
{
    public:

    int                    g = 0;
    int                    h = 0;
    short                  width = 0;
    short                  height = 0;
    std::vector<int>       tiles;
    std::shared_ptr<State> parent = NULL;
    e_swipe_direction      swipe_direction = SWIPE_NO;

    State();
    ~State();
    State(int width, int height);
    State(State const & rhs);
    State & operator=(State const & rhs);
    std::string to_string(void) const;
    bool operator==(State const & rhs) const;
};

State::State() :
    State(1, 1)
{
    return;
}

State::~State()
{}

State::State(State const & rhs)
{
    *this = rhs;
}

State & State::operator=(State const & rhs)
{
    if (this != &rhs)
    {
        g = rhs.g;
        h = rhs.h;
        width = rhs.width;
        height = rhs.height;
        tiles = rhs.tiles;
        parent = rhs.parent;
        swipe_direction = rhs.swipe_direction;
    }

    return *this;
}

State::State(int width, int height) :
    width(width),
    height(height),
    tiles(std::vector<int>(width * height))
{
    return;
}

typedef int (heuristic_fn)(const State & state, const State & goal);

inline int state_cmp(const State & lhs, const State & rhs)
{
    for (int y = 0; y < lhs.height; y++)
    for (int x = 0; x < lhs.width; x++)
    if (lhs.tiles[y*lhs.width+x] != rhs.tiles[y*lhs.width+x])
        return lhs.tiles[y*lhs.width+x] - rhs.tiles[y*lhs.width+x];

    return 0;
}

inline bool State::operator==(State const & rhs) const
{
    return state_cmp(*this, rhs) == 0;
}

std::string State::to_string(void) const
{
    static const char *direction[] =
    {
        [SWIPE_NO] = "   ",
        [SWIPE_UP] = " /\\",
        [SWIPE_DOWN] = " \\/",
        [SWIPE_LEFT] = " <-",
        [SWIPE_RIGHT] = " ->"
    };

    std::stringstream ss;

    ss << direction[swipe_direction] << std::endl << std::endl;
    for (int y = 0; y < height; y++)
    {
        ss << "\t";
        for (int x = 0; x < width; x++)
        {
            ss << tiles[y*width+x] << " |\t";
        }
        ss << std::endl << std::endl << std::endl;
    }

    return ss.str();
}

struct pos
{
    int x = 0;
    int y = 0;
};

bool is_solvable(const State & state)
{
    (void)state;
    return true;
    // TODO
    /*
    int inversions = 0;
    std::string line;

    for (int x = 0; x < state.width; x++)
    for (int y = 0; y < state.height; y++)
    {
        if (state.tiles[y*width+x] != 0)
        {
            line += std::to_string(state.tiles[y*width+x]);
        }
    }

    for (int i = 0; i < (int)line.size(); i++)
    for (int j = (int)line.size() - 1; j >= 0; j--)
    {
        if (line[j] > line[i])
        {
            inversions++;
        }
    }

    return inversions % 2 != 1;
    */
}

// La fonction presuppose que le nombre recherché existe dans le tableau
inline struct pos find_tile(const State & state, int number)
{
    struct pos pos;
    for (int y = 0; y < state.height; y++)
    for (int x = 0; x < state.width; x++)
    if (state.tiles[y*state.width+x] == number)
    {
        pos.x = x;
        pos.y = y;
        return pos;
    }
    return pos;
}

inline void generate_successors(const State & state, std::vector<State> & successors)
{
    struct pos gap = find_tile(state, 0);

    if (gap.x > 0)
    {
        State s(state); std::swap(s.tiles[gap.y*state.width+gap.x], s.tiles[gap.y*state.width+gap.x-1]); s.swipe_direction = SWIPE_LEFT; successors.push_back(s);
    }
    if (gap.y > 0)
    {
        State s(state); std::swap(s.tiles[gap.y*state.width+gap.x], s.tiles[(gap.y-1)*state.width+gap.x]); s.swipe_direction = SWIPE_UP; successors.push_back(s);
    }
    if (gap.x < state.width - 1)
    {
        State s(state); std::swap(s.tiles[gap.y*state.width+gap.x], s.tiles[gap.y*state.width+gap.x+1]); s.swipe_direction = SWIPE_RIGHT; successors.push_back(s);
    }
    if (gap.y < state.height - 1)
    {
        State s(state); std::swap(s.tiles[gap.y*state.width+gap.x], s.tiles[(gap.y+1)*state.width+gap.x]); s.swipe_direction = SWIPE_DOWN; successors.push_back(s);
    }
}

int TOTAL_OPENED;
int MAX_STATES;
int MOVE_COUNT;

std::vector<State> find(State & start_state, const State & goal_state, heuristic_fn * heuristic)
{
    std::cout << start_state.to_string() << std::endl;
    std::cout << goal_state.to_string() << std::endl;
    TOTAL_OPENED = 0;
    MAX_STATES = 0;
    MOVE_COUNT = -1;

    std::vector<State> path;
    std::deque<State> open_list;
    std::deque<State> closed_list;

    open_list.push_front(start_state);
    std::vector<State> successors;

    while (!open_list.empty() && TOTAL_OPENED < 100000)
    {
        auto it_current_state = std::min_element(open_list.begin(), open_list.end(), [](const State & lhs, const State & rhs){ return lhs.g + lhs.h < rhs.g + rhs.h; });
        auto current_state = *it_current_state;
        open_list.erase(it_current_state);
        TOTAL_OPENED += 1;
        if (state_cmp(current_state, goal_state) == 0) // Goal
        {
            auto node = std::make_shared<State>(current_state);
            while (node != NULL)
            {
                path.push_back(*node);
                MOVE_COUNT += 1;
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            MAX_STATES = closed_list.size() + open_list.size();
            return path;
        }
        successors.resize(0);
        generate_successors(current_state, successors);
        auto current_state_ptr = std::make_shared<State>(current_state);
        for (auto & successor : successors)
        {
            successor.g = current_state.g + 1;

            auto exisiting_one_open_it = std::find(open_list.begin(), open_list.end(), successor);
            if (exisiting_one_open_it != open_list.end() && (*exisiting_one_open_it).g <= successor.g)
                continue;

            auto exisiting_one_closed_it = std::find(closed_list.begin(), closed_list.end(), successor);
            if (exisiting_one_closed_it != closed_list.end() && (*exisiting_one_closed_it).g <= successor.g)
                continue;

            if (exisiting_one_open_it != open_list.end()) open_list.erase(exisiting_one_open_it);
            if (exisiting_one_closed_it != closed_list.end()) closed_list.erase(exisiting_one_closed_it);

            successor.parent = current_state_ptr;
            successor.h = heuristic(successor, goal_state);
            open_list.push_front(successor);
        }
        closed_list.push_front(current_state);
    }

    if (open_list.empty())
        std::cout << "Not solvable" << std::endl;
    else
        std::cout << "Timeout" << std::endl;

    return path;
}

State generate_goal_state(const State & state)
{
    State goal(state);
    int n = 1;
    int deepness = 0;
    int x = 0;
    int y = 0;

    while (n < state.width * state.height + 1)
    {
        x = deepness;
        y = deepness;
        while (x < state.width - deepness)
        {
            goal.tiles[y*state.width+x] = n;
            x++;
            n++;
        }
        while (y < state.height - deepness - 1)
        {
            goal.tiles[(y+1)*state.width+x-1] = n;
            y++;
            n++;
        }
        while (x > deepness + 1)
        {
            goal.tiles[y*state.width+x-2] = n;
            x--;
            n++;
        }
        while (y > deepness + 1)
        {
            goal.tiles[(y-1)*state.width+x-1] = n;
            y--;
            n++;
        }
        deepness++;
    }
    goal.tiles[y*state.width+x-1] = 0;

    return goal;
}

State generate_random_puzzle(int width, int height)
{
    auto s = generate_goal_state(State(width, height));
    srand(time(NULL));
    int difficulty = width + height;
    struct pos gap;
    int swipe_count = 0;
    enum e_swipe_direction last_move = SWIPE_NO;

    while (swipe_count < difficulty)
    {
        gap = find_tile(s, 0); if (rand() % 2 == 0 && last_move != SWIPE_RIGHT && gap.x >= 1)
        {
            swipe_count++; std::swap(s.tiles[gap.y*width+gap.x], s.tiles[gap.y*width+gap.x-1]); last_move = SWIPE_LEFT;
        }
        gap = find_tile(s, 0); if (rand() % 2 == 0 && last_move != SWIPE_DOWN && gap.y >= 1)
        {
            swipe_count++; std::swap(s.tiles[gap.y*width+gap.x], s.tiles[(gap.y-1)*width+gap.x]); last_move = SWIPE_UP;
        }
        gap = find_tile(s, 0); if (rand() % 2 == 0 && last_move != SWIPE_LEFT && gap.x < s.width - 1)
        {
            swipe_count++; std::swap(s.tiles[gap.y*width+gap.x], s.tiles[gap.y*width+gap.x+1]); last_move = SWIPE_RIGHT;
        }
        gap = find_tile(s, 0); if (rand() % 2 == 0 && last_move != SWIPE_UP && gap.y < s.height - 1)
        {
            swipe_count++; std::swap(s.tiles[gap.y*width+gap.x], s.tiles[(gap.y+1)*width+gap.x]); last_move = SWIPE_DOWN;
        }
    }
    return s;
}

inline int heuristic_dijkstra(const State & state, const State & goal)
{
    (void)state;
    (void)goal;
    return 0;
}

inline int heuristic_hamming(const State & state, const State & goal)
{
    int count = 0;
    for (int num : state.tiles) count += state.tiles[num] != goal.tiles[num];
    return count;
}

// Homemade heuristic
// Drink with moderation
inline int heuristic_maison(const State & state, const State & goal)
{
    int diff = 0;
    for (int num : state.tiles) diff += abs(state.tiles[num] - goal.tiles[num]);
    return diff;
}

inline int heuristic_manhattan(const State & state, const State & goal)
{
    int distance = 0;

    for (int num : state.tiles)
    {
        struct pos goal_pos = find_tile(state, num);
        struct pos actual_pos = find_tile(goal, num);
        distance += abs(actual_pos.x - goal_pos.x) + abs(actual_pos.y - goal_pos.y);
        // distance += ((actual_pos.x - goal_pos.x) * ((1 * (actual_pos.x > goal_pos.x)) + (-1 * (actual_pos.x < goal_pos.x)))) + ((actual_pos.y - goal_pos.y) * ((1 * (actual_pos.y > goal_pos.y)) + (-1 * (actual_pos.y < goal_pos.y))));
    }

    return distance;
}

inline int heuristic_euclidean(const State & state, const State & goal)
{
    int distance = 0;

    for (int num : state.tiles)
    {
        struct pos goal_pos = find_tile(state, num);
        struct pos actual_pos = find_tile(goal, num);
        distance += sqrt(abs(actual_pos.x - goal_pos.x) * abs(actual_pos.x - goal_pos.x)) + sqrt(abs(actual_pos.y - goal_pos.y) * abs(actual_pos.y - goal_pos.y));
    }

    return distance;
}

std::vector<std::string> split(std::string const & str)
{
    std::string buf;
    std::stringstream ss(str);
    std::vector<std::string> tokens;

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

heuristic_fn * select_heuristic(char **av)
{
    if (std::string(av[1]) == "manhattan") return &heuristic_manhattan;
    else if (std::string(av[1]) == "hamming") return &heuristic_hamming;
    else if (std::string(av[1]) == "dijkstra") return &heuristic_dijkstra;
    else if (std::string(av[1]) == "maison") return &heuristic_maison;
    else if (std::string(av[1]) == "euclidean") return &heuristic_euclidean;
    
    throw std::runtime_error("invalid heuristic");
}

int main(int ac, char **av)
{
    try
    {
        if (ac < 3)
        {
            std::cout << "./npuzzle manhattan|euclidean|hamming|maison|dijkstra [puzzle.txt]|[WIDTH HEIGHT]" << std::endl;
            return 1;
        }

        heuristic_fn * h = select_heuristic(av);

        State start_state;
        if (ac == 3)
        {
            std::ifstream file(av[2]);
            std::string str;
            std::vector<int> v;

            int found_comment = false;
            size_t size = 0;
            while (std::getline(file, str))
            {
                int l = 0;
                auto pos = str.find("#");
                if (pos != std::string::npos)
                {
                    found_comment = true;
                    while (found_comment && pos < str.size())
                    {
                        if ((str[pos]) == '\n')
                            found_comment = false;
                        (str[pos]) = '\0';
                        pos++;
                    }
                }
                else
                {
                    auto splitt = split(str);
                    if (splitt.size() == 1 && size == 0)
                    {
                        size = std::stoi(splitt[0]);
                        std::cout << "size [" << splitt[0] << "]" << std::endl;
                    }
                    else
                    {
                        for (auto & n : splitt)
                        {
                            v.push_back(std::stoi(n));
                            std::cout << "[" << n << "]" << std::endl;
                        }
                    }
                }
            }

            std::cout << "parse ok" << std::endl;
            std::cout << str << std::endl;
            start_state = State(size, size);
            size_t x = 0;
            size_t y = 0;

            for (auto & num : v)
            {
                if (x == size)
                {
                    x = 0;
                    y++;
                }
                start_state.tiles[y*start_state.width+x] = num;
                x++;
            }

            std::cout << "parse : " << start_state.to_string() << std::endl;
        }
        else if (ac == 4)
        {
            auto width = std::stoi(av[2]), height = std::stoi(av[3]);
            if (width < 3 || height < 3)
            {
                throw std::runtime_error("too small");
            }
            start_state = generate_random_puzzle(width, height);
        }

        if (!is_solvable(start_state))
        {
            throw std::runtime_error("not solvable");
        }

        // std::cout << "START" << start_state.to_string() << std::endl;

        auto goal = generate_goal_state(start_state);
        auto path = find(start_state, goal, h);

        for (auto & step : path)
            std::cout << step.to_string() << std::endl;

        std::cout << "TOTAL_OPENED = " << TOTAL_OPENED << std::endl;
        std::cout << "MAX_STATES   = " << MAX_STATES << std::endl;
        std::cout << "MOVE_COUNT   = " << MOVE_COUNT << std::endl;
    }
    catch (std::exception const & e)
    {
        std::cout << "Invalid map, " << e.what() << "" << std::endl;
        exit(1);
    }
    return 0;
}
