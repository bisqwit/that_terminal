#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <map>
#include <tuple>
#include <algorithm>
#include <string_view>
#include <cstdio>

#include <gd.h>

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

static void PrintRanges(
    unsigned width, unsigned height,
    const unsigned char* bitmap_real,
    std::pair<unsigned,bool>(*find_real)(char32_t),
    const unsigned char* bitmap_fake,
    std::pair<unsigned,bool>(*find_fake)(char32_t))
{
    // start bitmap
    unsigned yc = 0, widest = 0, dfl_width = 256;
    std::vector<unsigned> pixels(dfl_width*width*(height+1));
    auto BitmapEndl = [&]() { ++yc; pixels.resize(dfl_width*width * ((height==5) ? 6 : height)*(yc+1)); };
    auto BitmapPrintCh = [&](unsigned xc, const unsigned char* bm, unsigned glyph, unsigned color)
    {
        unsigned hei = (height==5) ? 6 : height;
        unsigned bytes_per_row = (width+7)/8, bytes_per_char = bytes_per_row * height;
        if(xc+1 > widest) widest = xc+1;
        if(widest > dfl_width) fprintf(stderr, "xc=%u in font %ux%u\n", xc, width,height);
        bm += glyph * bytes_per_char;
        for(unsigned y=0; y<height; ++y)
        {
            unsigned widefont = bm[0];
            if(width >= 8)  {widefont |= (bm[1] << 8);
            if(width >= 16) {widefont |= (bm[2] << 16);
            if(width >= 24) {widefont |= (bm[3] << 24);}}}
            bm += bytes_per_row;

            unsigned* row = &pixels[((yc*hei + y) * dfl_width + xc) * width];
            for(unsigned x=0; x<width; ++x)
                if((widefont >> (width-1 - x)) & 1)
                    row[x] = color;
        }
    };
    unsigned bitmap_index = 0;
    auto BitmapFlush = [&]()
    {
        // end bitmap
        char buf[64];
        std::sprintf(buf, "coverage-%ux%u-%u.png", width,height, bitmap_index);
        //if(width == 8 && height == 16)
        fprintf(stderr, "Creating %s (%ux%u cells)\n", buf, widest,yc);
        {
            std::FILE* fp = std::fopen((std::string("../doc/")+buf).c_str(), "wb");
            unsigned hei = (height==5) ? 6 : height;
            if(width <= 6)
            {
                gdImagePtr im = gdImageCreateTrueColor(widest*width*2, yc*hei*2);
                for(unsigned y=0; y<yc*hei*2; ++y)
                    for(unsigned x=0; x<widest*width*2; ++x)
                        gdImageSetPixel(im, x,y, pixels[(y/2)*dfl_width*width + (x/2)]);
                gdImagePng(im, fp);
                gdImageDestroy(im);
            }
            else
            {
                gdImagePtr im = gdImageCreateTrueColor(widest*width, yc*hei);
                for(unsigned y=0; y<yc*hei; ++y)
                    for(unsigned x=0; x<widest*width; ++x)
                        gdImageSetPixel(im, x,y, pixels[y*dfl_width*width + x]);
                gdImagePng(im, fp);
                gdImageDestroy(im);
            }
            std::fclose(fp);
        }

        std::cout << "![Font "<<width<<'x'<<height<<" coverage](" << buf << ")\n";
        ++bitmap_index;
        yc = 0; widest = 0;
        pixels.clear(); pixels.resize(dfl_width*width*height);
    };

    bool title_printed = false;
    for(auto [begin,end,name]: blocks)
    {
        if(title_printed) { title_printed = false; BitmapEndl(); }
        unsigned span = 32;
        if(begin == 0xAC00) // Korean
        {
            span = 28;
        }
        if(begin == 0x4E00 || begin == 0x3400 || (begin >= 0x20000 && begin <= 0x30000)) // CJK ideographs
        {
            span = 64; // Limited by ok1/ok2 bitness
        }

        bool something = false;
        for(unsigned index=begin; index<end; index+=span)
        {
            unsigned long ok1 = 0, ok2 = 0;
            for(unsigned p=0; p<span; ++p)
            {
                if(index+p <= end && find_real(index+p).second) ok1 |= 1ul<<p;
                if(index+p <= end && find_fake(index+p).second) ok2 |= 1ul<<p;
            }
            if(ok1 || ok2)
            {
                if(!title_printed)
                {
                    if(yc*height >= 600)
                    {
                        BitmapFlush();
                    }
                    title_printed = true;
                    unsigned x=0;
                    for(char c: name)
                        BitmapPrintCh(x++, bitmap_fake, find_fake(c).first, 0xFFFFFF);
                    BitmapEndl();
                    BitmapEndl();
                }
                else
                {
                    if(yc*height >= 1024)
                    {
                        BitmapFlush();
                    }
                }
                char buf[32];
                std::sprintf(buf, "U+%04X..U+%04X ", index, std::min(index+span-1, end));
                std::u32string str = FromUTF8(buf);
                unsigned color_start = str.size();
                for(unsigned p=0; p<span; ++p)
                    if((ok1|ok2) & (1u << p))
                        str += char32_t(index+p);
                    else
                        str += isdouble(index+p) ? char32_t(0x3000) : U' ';
                str += U' ';

                for(std::size_t p=0; p<str.size(); ++p)
                {
                    if(p < color_start || p == str.size()-1)
                        BitmapPrintCh(p, bitmap_fake, find_fake(str[p]).first, 0xFFFFFF);
                    else if(ok1 & (1u << (p-color_start)))
                        BitmapPrintCh(p, bitmap_real, find_real(str[p]).first, 0xFFFFFF);
                    else if(ok2 & (1u << (p-color_start)))
                        BitmapPrintCh(p, bitmap_fake, find_fake(str[p]).first, 0xBB7733);
                    else
                        BitmapPrintCh(p,
                            (const unsigned char*)"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377",
                                         0,
                                         0x000022);
                }
                BitmapEndl();
                something = true;
            }
        }
        if(!something)
        {
            std::fprintf(stderr, "Font has nothing in 0x%X..0x%X range\n", begin,end);
        }
    }
    BitmapFlush();
}

static const std::map<unsigned,std::string> lore
{
#include "font-sections.inc"
};
int main()
{
    for(auto p: lore)
    {
        unsigned x = p.first / 256, y = p.first % 256;
        std::cout << "\n## Font " << x << 'x' << y << "\n\n" << p.second
                     << "\n\n";
        const auto& entry1 = authentic::fonts.find(p.first)->second;
        const auto& entry2 = pseudo::fonts.find(p.first)->second;
        PrintRanges(x,y,
                    std::get<0>(entry1), std::get<1>(entry1),
                    std::get<0>(entry2), std::get<1>(entry2));
    }
}
 