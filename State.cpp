#include "State.hpp"

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

int State::state_cmp(const State & lhs, const State & rhs)
{
    for (int y = 0; y < lhs.height; y++)
    for (int x = 0; x < lhs.width; x++)
    if (lhs.tiles[y * lhs.width + x] != rhs.tiles[y * lhs.width + x])
        return lhs.tiles[y * lhs.width + x] - rhs.tiles[y * lhs.width + x];

    return 0;
}

bool State::operator==(State const & rhs) const
{
    return State::state_cmp(*this, rhs) == 0;
}

std::string State::to_string(void) const
{
    static const char * direction[] =
    {
        // Arrows are inverted because of reasons.
        [SWIPE_NO] = "    ",
        [SWIPE_UP] = "🢃  🢃",
        [SWIPE_DOWN] = "🢁  🢁",
        [SWIPE_LEFT] = "🢂  🢂",
        [SWIPE_RIGHT] = "🢀  🢀",
    };

    std::stringstream ss;

    for (int y = 0; y < height; y++)
    {
        for (int i = 1; i <= width; i++)
            ss << "█▀▀▀▀▀▀▀";
        ss << "█" << std::endl;


        for (int x = 0; x < width; x++)
        {
            int tile_number = tiles[y * width + x];

            if (x == 0)
                ss << "█ ";

            if (tile_number == 0)
                ss << "\033[1;31m" << direction[swipe_direction];
            else if (tile_number == swiped_number)
                ss << "\033[1;31m" << tile_number;
            else
            {
                ss << "\033[0m";
                ss << tile_number;
            }

            if (x < width - 1 && (tiles[y * width + x + 1] == swiped_number || tiles[y * width + x + 1] == 0))
                ss << "\033[1;31m" << "\t█ ";
            else
                ss <<"\t█ ";
        }

        ss << std::endl << "\033[0m" << "█";
        for (int i = 0; i < width; i++)
            ss << "▄▄▄▄▄▄▄█";
        ss << std::endl;
    }

    ss << std::endl;

    return ss.str();
}

void State::loopTiles(std::vector<int> line, int & n, int & direction, int end, Position posOffset, Position pos) const
{
    while (direction < end)
    {
        if (tiles[(pos.y + posOffset.y) * width + (pos.x + posOffset.x)] != 0)
            line.push_back(tiles[(pos.y + posOffset.y) * width + (pos.x + posOffset.x)]);
        direction++;
        n++;
    }
}

bool State::checkResolvability() const
{
    int n = 1;
    Position pos { 0, 0 };
    int depth = 0;
    int swap_count = 0;
    std::vector<int> line;

    if ((width % 2 == 0 && height % 2 != 0) || (height % 2 == 0 && width % 2 != 0))
    {
        throw std::runtime_error("'width' and 'height' parity mismatch");
    }

    while (n < width * height + 1)
    {
        pos = {depth, depth};
        loopTiles(line, n, pos.x, width - depth, {0, 0}, pos);
        loopTiles(line, n, pos.y, width - depth - 1, {1, -1}, pos);
        loopTiles(line, n, pos.x, depth + 1, {0, -2}, pos);
        loopTiles(line, n, pos.y, depth + 1, {-1, -1}, pos);
        depth++;
    }

    for (uint i = 0; i < line.size(); i++)
    for (uint j = i + 1; j < line.size(); j++)
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

Position State::find_tile(int number) const
{
    struct Position pos;

    for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++)
    if (tiles[y * width + x] == number)
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

std::vector<State> State::generate_successors(void) const
{
    std::vector<State> successors(4);
    struct Position    gap(find_tile(EMPTY_TILE));

    if (gap.x > 0)
        successors.push_back(State(*this).swipeLeft(gap));
    if (gap.y > 0)
        successors.push_back(State(*this).swipeUp(gap));
    if (gap.x < width - 1)
        successors.push_back(State(*this).swipeRight(gap));
    if (gap.y < height - 1)
        successors.push_back(State(*this).swipeDown(gap));

    return successors;
}


State State::sort_tiles() const
{
    State goal(*this);
    int n = 1;
    int depth = 0;
    int x = 0;
    int y = 0;

    while (n < width * height + 1)
    {
        x = depth;
        y = depth;
        while (x < width - depth)
        {
            goal.tiles[y * width + x] = n;
            x++;
            n++;
        }
        while (y < height - depth - 1)
        {
            goal.tiles[(y + 1) * width + x - 1] = n;
            y++;
            n++;
        }
        while (x > depth + 1)
        {
            goal.tiles[y * width + x - 2] = n;
            x--;
            n++;
        }
        while (y > depth + 1)
        {
            goal.tiles[(y - 1) * width + x - 1] = n;
            y--;
            n++;
        }
        depth++;
    }
    goal.tiles[y * width + x - 1] = 0;

    return goal;
}
