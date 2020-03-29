#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <string_view>
#include <cstdio>

#define NS1
#define NS2
namespace authentic
{
#include "fonts-authentic.inc"
#include "fonts-list.inc"
}
namespace pseudo
{
#include "fonts.inc"
#include "fonts-list.inc"
}
#undef NS1
#undef NS2
#include "../ctype.hh"

static const std::tuple<unsigned,unsigned,std::string_view> blocks[]
{
#include "unicode-sections.inc"
};

/*
static void PrintRanges(const std::vector<bool>& real, const std::vector<bool>& fake)
{
    for(auto [begin,end,name]: blocks)
    {
        bool title_printed = false;
        auto Process = [&](const auto& vec, int round)
        {
            unsigned span = 32;
            if(begin == 0xAC00)
            {
                span = 28;
            }
            for(unsigned index=begin; index<end; index+=span)
            {
                if(index==0x00 || index==0x80) continue;
                unsigned long ok = 0;
                for(unsigned p=0; p<span; ++p)
                    if(index+p <= end && vec[index+p] && index+p != 0x7F)
                        ok |= 1ul<<p;
                if(ok)
                {
                    if(!title_printed)
                    {
                        std::cout << "\n#### " << name << "\n\n";
                        title_printed = true;
                    }
                    if(round == 2)
                    {
                        std::cout << "\nSupported only by approximation:\n\n";
                        round = 0;
                    }
                    char buf[32];
                    std::sprintf(buf, "    U+%04X..U+%04X ", index, std::min((index+(span-1))==0x7F ? 0x7E : (index+(span-1)), end));
                    std::cout << buf;
                    std::u32string str;
                    for(unsigned p=0; p<span; ++p)
                        if(ok & (1ul << p))
                            str += char32_t(index+p);
                        else
                            str += isdouble(index+p) ? char32_t(0x3000) : U' ';
                    std::cout << ToUTF8(str) << "\n";
                }
            }
        };
        Process(real, 1);
        Process(fake, 2);
    }
}
*/

static const std::map<unsigned,std::string> lore
{
#include "font-sections.inc"
};
int main()
{
/*
    std::map<unsigned, std::pair<std::vector<bool>,std::vector<bool>>> supports;
    for(auto p: authentic::fonts)
    {
        std::vector<bool>& supported = supports[p.first].first;
        supported.resize(0x110000);
        for(unsigned index=0; index<=0x10FFFF; ++index)
            if(p.second.second(index).second)
                supported[index] = true;
    }
    for(auto p: pseudo::fonts)
    {
        auto& pair = supports[p.first];
        std::vector<bool>& supported = pair.second;
        supported.resize(0x110000);
        for(unsigned index=0; index<=0x10FFFF; ++index)
            if(p.second.second(index).second && !pair.first[index])
                supported[index] = true;
    }
*/
    std::cout <<
R"(# Fonts supported by *that terminal*

Most of these fonts are Public Domain, and come from X11.
Exceptions are listed.

This lists the coverage of each font.

This page has the information embedded in images,
rendered using each font. In those images, white denotes glyphs
that are supported fully. Brown (darker color) denotes
glyphs that are supported only through approximation,
such as by removing accents.

Solid dark blue indicates that this glyph is not supported.

)";
    /*for(auto p: supports)
    {
        unsigned x = p.first / 256, y = p.first % 256;
        std::cout << "\n## Font " << x << 'x' << y << "\n\n" << lore.find(p.first)->second << '\n';
        PrintRanges(p.second.first, p.second.second);
    }*/
}
