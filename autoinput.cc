#include <mutex>
#include <thread>
#include <string_view>
#include <random>
#include <list>

#include "settings.hh"
#include "autoinput.hh"
#include "ctype.hh"
#include "clock.hh"

//#define TURBOHACK

namespace
{
    std::mutex lock;
    std::list<AutoInputResponse> data;
    bool       terminate = false;
    bool       finished  = true;
}

static constexpr unsigned char Delay_CharClassTable[] =
{
//Delay_CharClassTable:
//        ; D, H, I, J, M are fast ctrls
//        ; K is CtrlK
//        ; A,B,E,F,W,Y, Z are other ctrls
//
// ; Ctrl keys:
//;  @A   BC   DE   FG   HI   JK   LM   NO   PQ   RS   TU   VW   XY   Z[   \]   ^_
// ; Normal keys
//;   !   "#   $%   &'   ()   *+   ,-   ./   01   23   45   67   89   :;   <=   >?
//;  @A   BC   DE   FG   HI   JK   LM   NO   PQ   RS   TU   VW   XY   Z[   \]   ^_
//;  `a   bc   de   fg   hi   jk   lm   no   pq   rs   tu   vw   xy   z{   |}   ~<del>
    0x0C,0xC0,0x2C,0xC0,0x22,0x14,0x01,0x00,0x00,0x00,0x00,0x02,0x02,0x22,0x88,0x88,//00h
    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x66,0x66,0x66,0x66,0x66,0x88,0x88,0x88,//20h
    0x86,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x68,0x88,0x88,//40h
    0x86,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x68,0x88,0xA8 //60h
};

#include <iostream>
void AutoInputProvider(std::u32string& s)
{
    std::mt19937 rnd;
    auto random = [&rnd](unsigned size){ return std::uniform_int_distribution<>(0, (size)-1)(rnd); };

    unsigned cur_speed = 16;
    int      modifier  = 5;
    int      repeats   = 0;
    char32_t prev_key  = 0;
    bool     had_ctrlk = false;

    auto ResetSeed = [&]()
    {
        rnd.seed(rnd.default_seed);
        modifier  = 5;
        repeats   = 0;
        prev_key  = 0;
        had_ctrlk = false;
    };

    ResetSeed();

#ifdef TURBOHACK
    bool TimeHackActive = true;
#endif

    auto Do_Delay_Resize = [&](std::pair<unsigned,unsigned> found_delay,
                               unsigned fx,unsigned fy,
                               unsigned sx,unsigned sy,
                               bool resize_first)
    {
        unsigned time = 0;
        if(found_delay.first != ~0u)
        {
            time = found_delay.second * 250; // milliseconds
            s[found_delay.first+1] = 0;
        }

        unsigned vcw = VidCellWidth;
        unsigned vch = VidCellHeight;
        unsigned ww = WindowWidth;
        unsigned wh = WindowHeight;
        if(time == 0 && fx==vcw && fy==vch && sx==ww && sy==wh)
        {
            // Nothing to do
            return;
        }

        unsigned step = 13; // 240fps = 4.166 ms per frame, 120fps = 8.3 ms per frame
        unsigned eat_time = std::max(std::min(time, 800u), 200u);

        fprintf(stderr, "Resizing - combined with delay %c%u ms (using %u ms for resize)\n",
            resize_first ? '+' : '-',
            time, eat_time);

#ifndef TURBOHACK
        if(!resize_first)
        {
            unsigned eat = (eat_time/step)*step;
            if(eat > time) { time -= eat;
                             data.emplace_back(std::string(""));
                             SleepFor(time/1e3); } else time = 0;
        }
        for(unsigned g=step, t=0; t<eat_time; t+=g, time = time>g ? time-g : 0)
        {
            double factor = t*1.0/eat_time;
            unsigned do_fx = vcw + int(fx-vcw)*factor;
            unsigned do_fy = vch + int(fy-vch)*factor;
            unsigned do_sx = ww  + int(sx-ww)*factor;
            unsigned do_sy = wh  + int(sy-wh)*factor;
            if(1)
            {
                std::lock_guard<std::mutex> lk(lock);
                data.push_back(std::array{do_fx,do_fy,do_sx,do_sy});
            }
            SleepFor(g/1e3);
        }
#endif
        // Instant
        if(true)
        {
            std::lock_guard<std::mutex> lk(lock);
            data.push_back(std::array{fx,fy,sx,sy});
        }
        data.emplace_back(std::string(""));
        if(time > 0)
        {
#ifdef TURBOHACK
            if(TimeHackActive) time = std::max(150u,std::min(200u, time / 20)); //HACK
#endif
            SleepFor(time/1e3);
        }
        ResetSeed();
    };

    for(std::size_t pos = 0; pos < s.size() && !terminate; ++pos)
    {
#ifdef TURBOHACK
        if(GetTime() >= 2*60+40) TimeHackActive = false;
#endif

        //std::cerr << s[pos] << '\n';
        switch(s[pos])
        {
            case U'\U00007FFD':
            {
                unsigned font = s[++pos];
                unsigned size = s[++pos];

                unsigned fx = font%32, fy = font/32;
                unsigned sx = size%1024, sy = size/1024;

                /* Check if there is delay soon hereafter */
                std::pair<unsigned, unsigned> found_delay{ ~0u, 0 };

                for(unsigned find = pos+1; find < s.size(); ++find)
                {
                    if(s[find] == U'\U00007FFE')
                    {
                        found_delay = {find, s[find+1]};
                        break;
                    }
                    if(s[find] == U'') continue; // ok
                    if(s[find] == U'') continue; // ok
                    if(s[find] == U'' && s[find+1] == 'd') { ++find;
                                                              if(s[find+1]=='\r') ++find;
                                                              continue; } // ok
                    if(s[find] == U'\033' && s[find+1]=='f') { ++find; continue; }
                    break;
                }

                Do_Delay_Resize(found_delay, fx,fy,sx,sy, true);
                break;
            }
            case U'\U00007FFE':
            {
                unsigned delay = s[++pos] * 250; // milliseconds
                if(!delay) break;

                /* Check if there is resize soon hereafter */
                std::pair<unsigned, unsigned> found_resize{ ~0u, 0 };
                std::pair<unsigned, unsigned> resize_type{};

                for(unsigned find = pos+1; find < s.size(); ++find)
                {
                    if(s[find] == U'\U00007FFD')
                    {
                        resize_type = {s[find+1], s[find+2]};
                        found_resize = {pos-1, delay/250};
                        break;
                    }
                    if(s[find] == U'') continue; // ok
                    if(s[find] == U'') continue; // ok
                    if(s[find] == U'' && s[find+1] == 'd') { ++find;
                                                              if(s[find+1]=='\r') ++find;
                                                              continue; } // ok
                    if(s[find] == U'\033' && s[find+1]=='f') { ++find; continue; }
                    break;
                }

                if(found_resize.first != ~0u)
                {
                    auto[font,size] = resize_type;
                    unsigned fx = font%32, fy = font/32;
                    unsigned sx = size%1024, sy = size/1024;
                    Do_Delay_Resize(found_resize, fx,fy,sx,sy, false);
                }
                else
                {
                    fprintf(stderr, "Performing delay: %u ms\n", delay);
                    if(delay)
                    {
#ifdef TURBOHACK
                        if(TimeHackActive) delay = std::max(150u,std::min(200u, delay / 20)); //HACK
#endif
                        SleepFor(delay/1e3);
                    }
                    ResetSeed();
                }
                break;
            }
            case U'\U00007FFF':
            {
                cur_speed = s[++pos];
                fprintf(stderr, "Input speed changed to %u\n", cur_speed);
                break;
            }
            default:
            {
                bool allow_merge  = false;
                unsigned use_speed = cur_speed;
                unsigned category = 0; // special key
                if(s[pos] == U'\003'
                || (s[pos] == U'x' && had_ctrlk))
                {
                    // Reset seed always when ^C is pressed
                    ResetSeed();
                }
                if(s[pos] < 0x80)
                    category = (Delay_CharClassTable[s[pos]/2] >> (4-4*(s[pos]%2))) & 15;
                unsigned factor = 20;
                if(s[pos] == prev_key)
                {
                    if(repeats >= 2) factor = 15; else { factor = 70; ++repeats; }
                }
                else switch(repeats = 0, category / 2)
                {
                    case 0: // 0 special key
                        factor = 70;
                        break;
                    case 1: // 2 fast ctrl
                        factor = 13;
                        if(s[pos] == U'\033')
                        {
                            had_ctrlk = true;
                        }
                        break;
                    case 2: // 4 ctrl k
                        factor = 35 + random(10);
                        had_ctrlk = true;
                        break;
                    case 3: // 6 alphanumeric
                        modifier = std::min(std::max(modifier + int(random(3)-1) * int(random(3)-1), 1), 18);
                        factor = (random(modifier*5) + 20 + random(46) + random(93)) / 3;
                        allow_merge = true;
                        if(had_ctrlk)
                        {
                            factor = 35 + random(10);
                            had_ctrlk = false;
                        }
                        break;
                    case 4: // 8 slow key
                        factor = 1000 / (10 + random(51));
                        allow_merge = true;
                        break;
                    case 5: // A pondering
                        factor = 224;
                        break;
                    case 6: // C arrow
                        // TODO: If previous key was ctrl-Z, give factor=20
                        factor = 1000 / (8 + random(66)); // max: 125ms, min: 13ms
                        break;
                }
#ifdef TURBOHACK
                unsigned u=use_speed;
                #define use_speed u
                if(TimeHackActive) use_speed = std::min(2u, use_speed);
#endif
                unsigned delay = factor * use_speed / 16;
                if(use_speed == 1) delay /= 50;

                SleepFor(delay/1e3);
                prev_key = s[pos];
                std::string str = ToUTF8(std::u32string(1, s[pos]));

                {
                std::lock_guard<std::mutex> lk(lock);
                if(allow_merge && !data.empty() && std::holds_alternative<std::string>(data.back()))
                    std::get<std::string>(data.back()) += std::move(str);
                else
                    data.emplace_back(std::move(str));
                }

                break;
            }
        }
    }
}

AutoInputResponse GetAutoInput()
{
    std::lock_guard<std::mutex> lk(lock);
    if(data.empty())
    {
        return 0u;
    }
    auto result = std::move(data.front());
    data.pop_front();
    return result;
}

#include <fstream>

void AutoInputStart()
{
    std::string s;
    try {
        std::ifstream t("inputter.dat", std::ios::binary);
        t.seekg(0, std::ios::end);
        s.reserve(t.tellg());
        t.seekg(0, std::ios::beg);
        s.assign( std::istreambuf_iterator<char>(t),
                  std::istreambuf_iterator<char>() );
    }
    catch(...)
    {
    }
    if(!s.empty())
    {
        finished = false;
        //s = "rm -f 'gruu.cc' '.#gruu.cc'\nstrace -otmptmp joe -tab 4 gruu.cc\nu" + s;
        std::thread t([](std::u32string data){
            AutoInputProvider(data);
            finished = true;
        }, FromUTF8(s));
        t.detach();
    }
}
void AutoInputEnd()
{
    terminate = true;
}

bool AutoInputActive()
{
    return finished == false;
}
