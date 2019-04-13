#include <chrono>
#include "color.hh"

static constexpr char persondata[] =
"                      #####     "
"      ######         #'''''###  "
"     #''''''##      #'''''''''# "
"    #'''''''''#     ###'.#.###  "
"    ###..#.###     #..##.#....# "
"   #..##.#....#    #..##..#...# "
"   #..##..#...#     ##...#####  "
"    ##...#####      ###.....#   "
"     ##.....#     ##'''##''###  "
"    #''##''#     #..''''##''#'# "
"   #''''##''#    #..'''######'.#"
"   #''''#####     #..####.##.#.#"
"    #...##.##     .#########''# "
"    #..'''###     #''######'''# "
"     #'''''#      #'''#  #'''#  "
"      #####        ###    ###   ";
static constexpr unsigned xcoordinates[2] = { 0, 16 };
static constexpr unsigned person_width    = 16;
static constexpr unsigned data_width      = 32, data_lines = 16;

static auto start_time   = std::chrono::system_clock::now();
static double walk_speed = 46.0; // pixels per second

static int PersonBaseX(unsigned window_width)
{
    window_width *= 8; // translate to pixels
    auto now_time = std::chrono::system_clock::now();

    // X coordinate where The Person is
    std::chrono::duration<double> time_elapsed = (now_time - start_time);
    unsigned walkway_width = window_width + std::max(person_width, window_width/5) + person_width;
    return unsigned(time_elapsed.count() * walk_speed) % walkway_width - person_width;
}
static int PersonFrame()
{
    auto now_time = std::chrono::system_clock::now();
    std::chrono::duration<double> time_elapsed = (now_time - start_time);
    return unsigned(time_elapsed.count() * 6) % 2;
}

unsigned PersonTransform(unsigned bgcolor, unsigned pixcolor, unsigned width, unsigned x, unsigned y)
{
    unsigned scrx = x;
    if(y >= data_lines)
    {
        return pixcolor;
    }
    unsigned basex = PersonBaseX(width);
    x -= basex;
    // Person outside view?
    if(x >= person_width)
    {
        return pixcolor;
    }

    unsigned frame_number = PersonFrame();
    unsigned frame_start  = xcoordinates[frame_number];
    char c = persondata[y*data_width + frame_start + x];
    switch(c)
    {
        case ' ': return Mix(bgcolor, pixcolor, 15,1,16);
        case '.': return Mix(pixcolor, 0x000000, 7,1,8);
        case '\'':return Mix(pixcolor, 0x000000, 6,2,8);
        case '#': return 0x000000;
    }
    return pixcolor;
}
