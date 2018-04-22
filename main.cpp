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
#include <sys/ioctl.h>

enum e_swipe_direction { SWIPE_NO = 0, SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT };

struct Position
{
    int x = 0;
    int y = 0;
};

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
    bool        checkResolvability();

    static inline int state_cmp(const State & lhs, const State & rhs);

    inline State &       swipeLeft(const Position & gap);
    inline State &       swipeUp(const Position & gap);
    inline State &       swipeRight(const Position & gap);
    inline State &       swipeDown(const Position & gap);
};

State::State() : State(1, 1)
{
    return;
}

State::~State()
{}

State::State(State const & rhs)
{
    *this = rhs;
}

bool State::operator<(State const & rhs) const
{
    return State::state_cmp(*this, rhs);
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
        swiped_number = rhs.swiped_number;
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

inline int State::state_cmp(const State & lhs, const State & rhs)
{
    for (int y = 0; y < lhs.height; y++)
    for (int x = 0; x < lhs.width; x++)
    if (lhs.tiles[y * lhs.width + x] != rhs.tiles[y * lhs.width + x])
        return lhs.tiles[y * lhs.width + x] - rhs.tiles[y * lhs.width + x];

    return 0;
}

inline bool State::operator==(State const & rhs) const
{
    return State::state_cmp(*this, rhs) == 0;
}

inline void printBreak(std::stringstream & ss, const struct winsize & size)
{
    for (int x = 0; x < size.ws_col; x++)
        ss << "█";
}

std::string State::to_string(void) const
{
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

    static const char * direction[] =
    {
        // Arrows are inverted because of reasons.
        [SWIPE_NO] = "  ",
        [SWIPE_UP] = "🢃",
        [SWIPE_DOWN] = "🢁",
        [SWIPE_LEFT] = "🢂",
        [SWIPE_RIGHT] = "🢀",
    };

    std::stringstream ss;

    for (int y = 0; y < height; y++)
    {
        ss << "\033[0m█ ";
        for (int x = 0; x < width; x++)
        {
                int tile_number = tiles[y * width + x];
                if (tile_number == 0)
                    ss << "\033[1;31m" << direction[swipe_direction];
                else if (tile_number == swiped_number)
                    ss << "\033[1;31m" << tile_number;
                else
                    ss << "\033[0m" << tile_number;

            if (x < width - 1 && (tiles[y * width + x + 1] == swiped_number || tiles[y * width + x + 1] == 0))
                ss << "\033[1;31m" << "\t█ ";
            else
                ss << "\t█ ";
        }

        ss << std::endl << "█";

        for (int i = 0; i < width; i++)
            ss << "\t█";

        ss << std::endl << std::endl << std::endl;
    }

    ss << std::endl << std::endl << std::endl << std::endl;

    return ss.str();
}

bool State::checkResolvability()
{
    int n = 1;
    int x = 0;
    int y = 0;
    int depth = 0;
    int swap_count = 0;
    std::vector<int> line;

    if ((width % 2 == 0 && height % 2 != 0) || (height % 2 == 0 && width % 2 != 0))
    {
        throw std::runtime_error("'width' and 'height' parity mismatch");
    }

    while (n < width * height + 1)
    {
        x = depth;
        y = depth;
        while (x < width - depth)
        {
            if (tiles[y * width + x] != 0)
                line.push_back(tiles[y * width + x]);
            x++;
            n++;
        }
        while (y < height - depth - 1)
        {
            if (tiles[(y + 1) * width + x - 1] != 0)
                line.push_back(tiles[(y + 1) * width + x - 1]);
            y++;
            n++;
        }
        while (x > depth + 1)
        {
            if (tiles[y * width + x - 2] != 0)
                line.push_back(tiles[y * width + x - 2]);
            x--;
            n++;
        }
        while (y > depth + 1)
        {
            if (tiles[(y - 1) * width + x - 1] != 0)
                line.push_back(tiles[(y - 1) * width + x - 1]);
            y--;
            n++;
        }
        depth++;
    }

    for (int i = 0; i < line.size(); i++)
    for (int j = i + 1; j < line.size(); j++)
    if (line[j] > line[i])
        swap_count++;

    if (line.size() % 2 != 0)
    {
        auto it = std::find(line.begin(), line.end(), 0);
        auto position = std::distance(line.begin(), it);
        swap_count += position / line.size();
    }

    return !(swap_count % 2 == 1);
}

// La fonction presuppose que le nombre recherché existe dans le tableau
inline struct Position find_tile(const State & state, int number)
{
    struct Position pos;
    for (int y = 0; y < state.height; y++)
    for (int x = 0; x < state.width; x++)
    if (state.tiles[y * state.width + x] == number)
    {
        pos.x = x;
        pos.y = y;
        return pos;
    }
    return pos;
}

inline State & State::swipeLeft(const Position & gap)
{
    swiped_number = tiles[gap.y * width + gap.x - 1];
    std::swap(tiles[gap.y * width + gap.x], tiles[gap.y * width + gap.x - 1]);
    swipe_direction = SWIPE_LEFT;
    return *this;
}

inline State & State::swipeUp(const Position & gap)
{
    swiped_number = tiles[(gap.y - 1) * width + gap.x];
    std::swap(tiles[gap.y * width + gap.x], tiles[(gap.y - 1) * width + gap.x]);
    swipe_direction = SWIPE_UP;
    return *this;
}

inline State & State::swipeRight(const Position & gap)
{
    swiped_number = tiles[gap.y * width + gap.x + 1];
    std::swap(tiles[gap.y * width + gap.x], tiles[gap.y * width + gap.x + 1]);
    swipe_direction = SWIPE_RIGHT;
    return *this;
}

inline State & State::swipeDown(const Position & gap)
{
    swiped_number = tiles[(gap.y + 1) * width + gap.x];
    std::swap(tiles[gap.y * width + gap.x], tiles[(gap.y + 1) * width + gap.x]);
    swipe_direction = SWIPE_DOWN;
    return *this;
}

inline void generate_successors(const State & state, std::vector<State> & successors)
{
    struct Position gap = find_tile(state, 0);

    if (gap.x > 0)
        successors.push_back(State(state).swipeLeft(gap));
    if (gap.y > 0)
        successors.push_back(State(state).swipeUp(gap));
    if (gap.x < state.width - 1)
        successors.push_back(State(state).swipeRight(gap));
    if (gap.y < state.height - 1)
        successors.push_back(State(state).swipeDown(gap));
}

struct statecomp
{
    bool operator() (const State & lhs, const State & rhs) const
    {
        return lhs < rhs;
    }
};

State generate_goal_state(const State & state)
{
    State goal(state);
    int n = 1;
    int depth = 0;
    int x = 0;
    int y = 0;

    while (n < state.width * state.height + 1)
    {
        x = depth;
        y = depth;
        while (x < state.width - depth)
        {
            goal.tiles[y * state.width + x] = n;
            x++;
            n++;
        }
        while (y < state.height - depth - 1)
        {
            goal.tiles[(y + 1) * state.width + x - 1] = n;
            y++;
            n++;
        }
        while (x > depth + 1)
        {
            goal.tiles[y * state.width + x - 2] = n;
            x--;
            n++;
        }
        while (y > depth + 1)
        {
            goal.tiles[(y - 1) * state.width + x - 1] = n;
            y--;
            n++;
        }
        depth++;
    }
    goal.tiles[y * state.width + x - 1] = 0;

    return goal;
}

State generate_random_puzzle(int width, int height)
{
    if (width < 3 || height < 3)
    {
        throw std::runtime_error("too small");
    }

    auto s = generate_goal_state(State(width, height));
    srand(time(NULL));
    int difficulty = (width + height);
    struct Position gap;
    int swipe_count = 0;
    enum e_swipe_direction last_move = SWIPE_NO;

    while (swipe_count < difficulty)
    {
        gap = find_tile(s, 0);
        if (rand() % 2 == 0 && last_move != SWIPE_RIGHT && gap.x >= 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[gap.y * width + gap.x - 1]); last_move = SWIPE_LEFT; swipe_count++;
        }
        gap = find_tile(s, 0);
        if (rand() % 2 == 0 && last_move != SWIPE_DOWN && gap.y >= 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[(gap.y - 1) * width + gap.x]); last_move = SWIPE_UP; swipe_count++;
        }
        gap = find_tile(s, 0);
        if (rand() % 2 == 0 && last_move != SWIPE_LEFT && gap.x < s.width - 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[gap.y * width + gap.x + 1]); last_move = SWIPE_RIGHT; swipe_count++;
        }
        gap = find_tile(s, 0);
        if (rand() % 2 == 0 && last_move != SWIPE_UP && gap.y < s.height - 1)
        {
            std::swap(s.tiles[gap.y * width + gap.x], s.tiles[(gap.y + 1) * width + gap.x]); last_move = SWIPE_DOWN; swipe_count++;
        }
    }
    return s;
}

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
        struct Position goal_pos = find_tile(state, num);
        struct Position current_pos = find_tile(goal, num);
        distance += abs(current_pos.x - goal_pos.x) + abs(current_pos.y - goal_pos.y);
    }

    return distance;
}

int heuristic_euclidean(const State & state, const State & goal)
{
    int distance = 0;

    for (int num : state.tiles)
    {
        struct Position goal_pos = find_tile(state, num);
        struct Position current_pos = find_tile(goal, num);
        distance += sqrt(abs(current_pos.x - goal_pos.x) * abs(current_pos.x - goal_pos.x)) + sqrt(abs(current_pos.y - goal_pos.y) * abs(current_pos.y - goal_pos.y));
    }

    return distance;
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
        int l = 0;
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

class Game
{
  public :

    int   total_opened = 9;
    int   max_conccurent_states = 0;
    int   move_count = 0;
    State state;

  public :

    std::vector<State> solve(const State & goal_state, heuristic_fn * heuristic);
};

#define MAX_REASONABLE_STATE_CHANGES 100000

std::vector<State> Game::solve(const State & goal_state, heuristic_fn * heuristic)
{
    total_opened = 0;
    max_conccurent_states = 0;
    move_count = -1;

    std::vector<State> path;
    std::set<State, statecomp> open_list;
    std::set<State, statecomp> closed_list;

    open_list.insert(state);
    std::vector<State> successors;

    while (!open_list.empty() && total_opened < MAX_REASONABLE_STATE_CHANGES)
    {
        auto it_current_state = std::min_element(open_list.begin(), open_list.end(), [](const State & lhs, const State & rhs){ return lhs.g + lhs.h < rhs.g + rhs.h; });
        auto current_state = *it_current_state;
        open_list.erase(it_current_state);
        total_opened += 1;
        if (State::state_cmp(current_state, goal_state) == 0) // Goal
        {
            auto node = std::make_shared<State>(current_state);
            while (node != NULL)
            {
                path.push_back(*node);
                move_count += 1;
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            max_conccurent_states = closed_list.size() + open_list.size();
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
            open_list.insert(successor);
        }
        closed_list.insert(current_state);
    }

    if (!open_list.empty())
        std::cout << "Probably not resolvable according to chosen heuristic" << std::endl;
    else if (total_opened >= MAX_REASONABLE_STATE_CHANGES)
        std::cout << "Failed to resolve using reasonable resource amount" << std::endl;
    else
        std::cout << "Failed to resolve" << std::endl;

    return path;
}


int main(int ac, char **av)
{
    try
    {
        if (ac < 3)
        {
            std::cout << "./npuzzle manhattan|euclidean|ntiles|dijkstra [puzzle.txt]|[WIDTH HEIGHT]" << std::endl;
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

        if (!game.state.checkResolvability() && ac < 4)
        {
            throw std::runtime_error("couldn't resolve");
        }

        heuristic_fn * h = select_heuristic(av);

        auto goal = generate_goal_state(game.state);
        auto path = game.solve(goal, h);

        for (auto & step : path)
        {
            std::cout << step.to_string() << std::endl;
        }

        std::cout << "game.total_opened          = " << game.total_opened << std::endl;
        std::cout << "game.max_conccurent_states = " << game.max_conccurent_states << std::endl;
        std::cout << "game.move_count            = " << game.move_count << std::endl;
    }
    catch (std::exception const & e)
    {
        std::cout << "Error: " << e.what() << "" << std::endl;
        exit(1);
    }
    return 0;
}