#include <string>
#include <atomic>
#include <cstdio>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <bitset>
#include <memory>
#include <unordered_set>
#include <set>

#include "oneshot_poolalloc.hh"

/* Utility that finds a hashing for given words for jsf.hh in my editor */

std::size_t Hash(const std::string& s, std::size_t start=0,
                                       std::size_t mul=0xc6a4a7935bd1e995LLU,
                                       unsigned shift=17)
{
    std::size_t result = start;
    for(unsigned char c: s)
    {
        result = (result << shift) | (result >> (64-shift));
        result ^= c;
        result *= mul;
    }
    return result;
}

int main(int argc, char** argv)
{
    std::map<std::string, unsigned> action_values;

    bool     ucase  = std::toupper(argv[1][0]) == 'U';

    for(std::ifstream f("/usr/share/X11/rgb.txt"); f.good(); )
    {
        std::string s;
        unsigned r,g,b;
        std::getline(f, s);
        int pos=0;
        if(std::sscanf(s.c_str(), "%u %u %u %n\n", &r,&g,&b,&pos) == 3)
        {
            s.erase(0, pos);
            if(ucase)
                for(char& c: s) c = std::toupper(c);
            else
                for(char& c: s) c = std::tolower(c);
            unsigned color = (r<<16) + (g<<8) + b;
            action_values[s] = color;
            std::printf("Got <%s> with 0x%06X\n", s.c_str(), color);
        }
        //if(action_values.size() >= 4096) break;
    }

    std::vector<std::string> words;
    for(const auto& w: action_values) words.push_back(w.first);

    std::atomic<std::size_t> best_dist;
    best_dist = ~0ul;

    constexpr unsigned est_size = 800;
    constexpr unsigned bound = 11932; // mod=11931 found already

    std::vector<std::size_t> hash;
    for(const auto& s: words) hash.push_back(Hash(s));

    constexpr unsigned a=0;
//    constexpr unsigned mod = bound;
    constexpr unsigned div = 1, add = 0;
    #pragma omp parallel for collapse(3) schedule(dynamic,20000)
    //#pragma omp parallel for collapse(3) schedule(dynamic, 128000)
    for(unsigned mod=bound; mod>=est_size; --mod)
    for(unsigned a=0; a<64; ++a)
    for(unsigned mul=1; mul<20000; ++mul)
    //for(unsigned div=1; div<2; ++div)
    //for(unsigned add=0; add<div; ++add)
//    for(unsigned add=0; add<mod; ++add)
    {
      if(0) {fail:continue; }

      std::bitset<bound> used;
      for(std::size_t c: hash)
      {
          //if(a) c ^= (c >> a);
          c *= mul;
          c = (c << a) | (c >> (64-a));
          c += add;
          c /= div;
          c %= mod;
          //assert(c < bound);
          if(used.test(c)) goto fail;
          used.set(c);
      }

      std::size_t min=~0ull, max=0;
      for(std::size_t c: hash)
      {
          //if(a) c ^= (c >> a);
          c *= mul;
          c = (c << a) | (c >> (64-a));
          c += add;
          c /= div;
          c %= mod;
          min = std::min(min, c);
          max = std::max(max, c);
      }

      std::size_t dist = max-min+1;
      if(dist < best_dist)
      {
          #pragma omp critical
          {
              if(dist < best_dist)
              {
                  std::map<std::string, unsigned long> data;
                  for(const auto& s: words)
                  {
                      std::size_t c = Hash(s);
                      //if(a) c ^= (c >> a);
                      c *= mul;
                      c = (c << a) | (c >> (64-a));
                      c += add;
                      c /= div;
                      c %= mod;
                      data.emplace(s, c);
                  }

                  best_dist = dist;
                  printf("//Good: a=%u,add=%u,div=%u,mul=%u,mod=%u  distance = %lu  min=%lu max=%lu\n", a,add,div,mul,mod, dist, min,max);

/**/
                  printf(R"(
    std::size_t c = Hash(lc);
    c *= %u;
    c = (c << u) | (c >> (64-u));
    //if(%u) c ^= (c >> %u);
    c = ((c + %u) / %u) %% %u;
    switch(c)
    {
)",
  mul, a,a,add,div,mod
);
                  for(const auto& p: data)
                  {
                      unsigned color = action_values.find(p.first)->second;
                      printf("        case %lu: return 0x%06X; //%s\n",
                          p.second, color, p.first.c_str());
                  }
                  printf("    }\n");
/**/
                  fflush(stdout);
              }
          }
      }
    }
}


template<unsigned n> thread_local unsigned BytePool<n>::head = 0;
template<unsigned n> thread_local unsigned BytePool<n>::used = 0;
template<unsigned n> thread_local char     BytePool<n>::buffer[n] = {};
