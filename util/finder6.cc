#include <string>
#include <atomic>
#include <cstdio>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <thread>
#include <random>
#include <mutex>
#include <map>
#include <cassert>
#include <string_view>

# define likely(x)       __builtin_expect(!!(x), 1)
# define unlikely(x)     __builtin_expect(!!(x), 0)

#include <x86intrin.h> // rdrand

static constexpr unsigned NUM_THREADS = 48;
static constexpr unsigned est_size = 753;
static constexpr unsigned min_mod  = 5000;//est_size;
static constexpr unsigned bound    = 10000;

#include "newhash.hh"

static inline std::uint32_t R32(const char* p)
{
    return *reinterpret_cast<const std::uint32_t*>(p);
}
static inline std::uint64_t R64(const char* p)
{
    return *reinterpret_cast<const std::uint64_t*>(p);
}

std::size_t Hash(std::string_view s, std::size_t start,
                                     std::size_t mul,
                                     std::size_t mul2,
                                     std::size_t add,
                                     std::size_t permut,
                                     std::size_t letters,
                                     unsigned shift)
{
    unsigned s1 = 1 + 4 * ((shift>> 0)&15);
    unsigned s2 = 1 + 4 * ((shift>> 4)&15);
    unsigned s3 = 1 + 4 * ((shift>> 8)&15);

    // mul2 contains (4+10)*4 = 56 bits
    std::size_t bits = start;
    for(std::size_t p=0; p<s.size(); ++p)
    {
        std::size_t m; unsigned c = s[p], s;
        if(c & 64)
        {
            c = (letters >> (c*2 - 'a'*2)) & 3;
            c = 0 + c*4;
            s = s2;
            m = mul;
        }
        else
        {
            c = c*4 + 16 - '0'*4;
            s = s1;
            m = add;
        }
        bits += ((mul2 >> c) & 15) * m;
        //bits += c * m;
        bits = (bits << s) | (bits >> (64-s));
    }
    bits += permut;
    bits = (bits << s3) | (bits >> (64-s3));
    return bits;
}

static constexpr const char HASH_ALGO[] = R"(
static constexpr std::uint_least64_t rotate(unsigned bits, std::uint_least64_t result)
{
    return ((result << bits) | (result >> (64 - bits)));
}
static constexpr unsigned hash(const char* s, std::size_t length)
{
    constexpr std::uint_least64_t a = 0x%1$zX, b = 0x%2$zX;
    constexpr std::uint_least64_t c = 0x%4$zX, d = 0x%5$zX;
    constexpr std::uint_least64_t lett = 0x%6$013zX; // maps a..z into 0..3 
    constexpr std::uint_least64_t map = 0x%3$014zX; // maps everything into 0..15
    unsigned sh = 0x%8$X, s1 = 1+4*((sh>>0)&15), s2 = 1+4*((sh>>4)&15), s3 = 1+4*((sh>>8)&15);
    std::uint_least64_t result = a;
    while(length--)
        if(unsigned z = *s++; z & 64)
            result = rotate(s2, result + b * (15 & (map >> 4*(lett >> (z*2 - 'a'*2)) & 3))));
        else
            result = rotate(s1, result + c * (15 & (map >> 4*(z + 4 - '0'))));
    return (rotate(s3, d + result)) %% %7$u;
}

#define docolors(o) \
)";

static inline unsigned __attribute__ ((target("rdrnd"))) __x86_rdrand32(void)
{
    unsigned int retries = 100;
    unsigned val=0;

    while (_rdrand32_step(&val) == 0)
        if (--retries == 0)
            std::__throw_runtime_error(__N("random_device::__x86_rdrand32(void)"));

    return val;
}
static inline unsigned long long __attribute__ ((target("rdrnd"))) __x86_rdrand64(void)
{
    unsigned int retries = 100;
    unsigned long long val=0;

    while (_rdrand64_step(&val) == 0)
        if (--retries == 0)
            std::__throw_runtime_error(__N("random_device::__x86_rdrand64(void)"));

    return val;
}

template<std::size_t N>
static bool TestMapValidity(std::size_t letters,
                            std::string_view (&words)[N],
                            unsigned (&colors)[N])
{
    std::tuple<std::string/*modded*/, std::string_view/*orig*/, unsigned/*color*/> data[N];
    for(std::size_t a=0; a<N; ++a)
    {
        std::get<1>(data[a]) = words[a];
        std::get<2>(data[a]) = colors[a];

        std::string& modded = std::get<0>(data[a]);
        modded = words[a];
        for(char& c: modded)
            if(c & 64) //>= 'a' && c <= 'z')
            {
                c = (letters >> (c*2 - 'a'*2)) & 3;
                //if(c == 3) return false;
            }
    }
    std::sort(data, data+N, [&](auto& a,auto& b) { return std::get<0>(a) < std::get<0>(b); });

    for(std::size_t a=0; a+1 < N; ++a)
        for(auto p = data+a+1; p != data+N && std::get<0>(*p) == std::get<0>(data[a]); ++p)
            if(std::get<2>(*p) != std::get<2>(data[a])  // color differs
            && std::get<1>(*p) != std::get<1>(data[a])) // original colorname differs
                return false;
    return true;
}

template<std::size_t N>
static unsigned PopCount_4bits(std::size_t value)
{
    // Count the number of distinct 4-bit masks in input
    unsigned bits = 0;
    #pragma omp simd reduction(|:bits)
    for(unsigned n=0; n<N; ++n)
        bits |= 1u << ((value >> (n*4)) & 15);
    return __builtin_popcount(bits);
}

template<unsigned est_size>
extern unsigned FastModDef(std::uint64_t value, unsigned mod); // FastMod

auto FastMod = FastModDef<est_size>;
/*unsigned FastMod(std::uint64_t value, unsigned mod)
{
    return value % mod;
}*/

static std::atomic<unsigned> best_dist = bound;
static std::atomic<unsigned> best_mod  = bound;
static std::mutex lock;
static std::string_view words[est_size];
static std::string_view orig_words[est_size];
static unsigned         colors[est_size];

static void Finder()
{
reseed:;
    std::seed_seq seq{ __x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64(),
                       __x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64()  };
    std::mt19937_64 e2(seq);
    std::fprintf(stderr, "reseed\n");

    //alignas(16) unsigned short used[bound];
    alignas(16) std::uint_least64_t used[(bound+127)/64];
    alignas(16) unsigned short      what[bound];
    alignas(16) std::size_t hash[est_size];
    std::size_t letters=0, letters_uses=0;
    std::size_t mul2=0, mul2_uses=0;
    for(unsigned run=0; run< (1ull << 10); ++run)
    {
        std::size_t start = e2();
        std::size_t mul   = e2();
        std::size_t add   = e2();
        std::size_t permut= e2();

        if(!mul2_uses--)
        {
            do {
                std::size_t mul3;
                // Verify that all four symbols have unique coding
                do {
                    mul2  = e2() % (1ull << (4*4));      // 16 bits
                } while(PopCount_4bits<4>(mul2) != 4);
                // Verify that all ten digits have unique coding
                do {
                    mul3  = e2() % (1ull << (4*10));     // 40 bits
                } while(PopCount_4bits<10>(mul3) != 10);
                mul2 |= mul3 << 16;                // total 56 bits

                //if(PopCount_4bits<14>(mul2) == 10) break;
                if(mul2 == 1023) break; // Only 0..3
                break;
            } while(1);
            mul2_uses = 4;//32768;
        }

        if(!letters_uses--)
        {
            do {
                letters   = e2() % (1ull << (26*2));

                // Zero the index for 'x'
                letters &= ~(3ull << (('x'-'a')*2));

            } while(!TestMapValidity(letters, words, colors));
            letters_uses = 7;
        }

        //unsigned shift = e2() % (1ull << (3*6)); // 3*6 = 24 bits
        fprintf(stderr, ".");
        //for(unsigned s_rounds = 0; s_rounds < 2048; ++s_rounds)
        for(unsigned shift = 0; shift < (1u << 3*4); ++shift)
        {
        //unsigned shift = e2() % (1ull << (3*6)); // 3*6 = 24 bits

        for(unsigned w=0; w<est_size; ++w)
        {
            hash[w] = Hash(words[w], start,mul,mul2, add,permut, letters, shift);
        }

        bool accepted = false;
        unsigned min = ~0u, max = 0;
        unsigned mod = min_mod;
        unsigned cap = std::min(bound, best_mod+64);
        /* Try different MOD values for the same set of hashes */
        for(; mod<=cap; ++mod)
        //constexpr unsigned mod = 8192;
        //for(bool r=true; r; r=false)
        {
            if(0) {fail:continue; }

            //std::memset(&used[0], 0, mod*sizeof(*used));
            std::memset(&used[0], 0, ((mod+63)/8+7)&~7);

            /*unsigned short hash2[est_size];
            #pragma omp simd
            for(unsigned w=0; w<words.size(); ++w)
                hash2[w] = hash[w] % mod; // Not SIMDable
            */

            min = ~0u;
            max = 0;

            /*
            #pragma omp simd reduction(min:min) reduction(max:max)
            for(unsigned w=0; w<words.size(); ++w)
            {
                min = std::min(min, hash2[w]);
                max = std::max(max, hash2[w]);
            }
            if((max-min+1) >= best) goto fail;*/

            //std::size_t fails = 0;
            //#pragma omp simd reduction(|:fails) reduction(min:min) reduction(max:max)
            unsigned best = best_dist;
            unsigned w = 0;
            unsigned c = FastMod(hash[w], mod);
            min = max = c;
            auto& usedword = used[c/64];
            auto  usedmask = 1ull << (c%64);
            usedword |= usedmask;
            what[c] = w;

            for(++w; w<est_size; ++w)
            {
                unsigned c = FastMod(hash[w], mod);
                min = std::min(c, min);
                max = std::max(c, max);
                if(unlikely((max-min+1) >= best)) goto fail;

                auto& usedword = used[c/64];
                auto  usedmask = 1ull << (c%64);
                if(usedword & usedmask)
                {
                    /* identify what last used this */
                    //for(unsigned w2 = w; w2-- > 0; )
                    if(likely(colors[what[c]] != colors[w]))
                        goto fail;
                    // Accept a hash collision, if the result
                    // still produces the same color result.
                    continue;
                }
                usedword |= usedmask;
                what[c] = w;
            }

            accepted = true;
            break;
        }
        if(!accepted) continue;

        unsigned dist = max-min+1;
        if(dist >= best_dist)
        {
            printf("//Bad: shift=0x%X, start=0x%zX,mul=0x%zX,add=0x%zX,perm=0x%zX, mod=%u  distance = %u  min=%u max=%u\n",
                shift,start,mul,add,permut, mod, dist,min,max);
            continue;
        }
        printf("//Testing\n");

        std::lock_guard<std::mutex> lk(lock);
        if(dist >= best_dist) continue;

        std::map<unsigned/*hash*/, std::vector<unsigned/*word index*/>> data;
        for(std::size_t w=0; w<est_size; ++w)
        {
            std::size_t c = Hash(words[w], start,mul,mul2, add,permut, letters, shift);
            data[FastMod(c, mod)].push_back(w);
        }

        best_dist = dist;
        best_mod  = mod;
        printf("//Good: shift=0x%X, start=0x%zX,mul=0x%zX,add=0x%zX,perm=0x%zX,letters=0x%zX, mod=%u  distance = %u  min=%u max=%u\n",
            shift,start,mul,add,permut,letters, mod, dist,min,max);

        printf(HASH_ALGO,
        // Parameters are in same order as for Hash():
  start,        // 1,
  mul,mul2,     // 2,3
  add,permut,   // 4,5
  letters,      // 6
  mod,          // 7
  shift         // 8
);
        for(const auto& [hash,indexes]: data)
        {
            std::string extra, name;
            unsigned    color = 0xDEADCAFE;
            for(auto w: indexes)
            {
                if(name.empty())
                    name = orig_words[w];
                else
                {
                    //fprintf(stderr, "\tgot a reuse (0x%06X = %s = %s)!\n",
                    //   color, name.c_str(), orig_words[w].data());
                    if(color != colors[w])
                    {
                        fprintf(stderr, "something is odd with <%s> and <%s>\n",
                            name.c_str(), orig_words[w].data());
                    }
                    if(extra.empty())
                        extra = "and \"";
                    else
                        extra += ", \"";
                    extra += orig_words[w];
                    extra += '"';
                }
                color = colors[w];
            }
            if(!extra.empty()) extra = "/* " + extra + " */";
            std::printf("        o(%4u, 0x%06X, \"%s\") %s\\\n",
                hash, color, name.c_str(), extra.c_str());
        }

        fflush(stdout);

        } // shift
   }
   goto reseed;
}

static std::string PreFilterColor(std::string_view s)
{
    std::string lc(s);
    /* Create a copy of the color name, with spaces erased and characters lowercased */
    auto first = lc.begin(), end = lc.end();
    for(auto begin = first; begin != end; ++begin)
        if(*begin != ' ')
        {
            *first = std::tolower(*begin);
            ++first;
        }
    lc.erase(first, end);
    return lc;
}

int main(int /*argc*/, char** argv)
{
    std::vector<std::tuple<unsigned,std::string,std::string>> pairs;
    for(std::ifstream f("/usr/share/X11/rgb.txt"); f.good(); )
    {
        std::string s;
        unsigned r,g,b;
        std::getline(f, s);
        int pos=0;
        if(std::sscanf(s.c_str(), "%u %u %u %n\n", &r,&g,&b,&pos) == 3)
        {
            s.erase(0, pos);
            std::string modified = PreFilterColor(s);

            unsigned color = (r<<16) + (g<<8) + b;
            //std::fprintf(stderr, "Got <%s> with 0x%06X\n", s.c_str(), color);
            pairs.emplace_back(color, std::move(modified), std::move(s));
        }
        //if(action_values.size() >= 4096) break;
    }
    // Sort the pairs so that identical colors are consecutive
    //std::sort(pairs.begin(), pairs.end());
    assert(pairs.size() == est_size);

    {unsigned c=0;
    for(auto& w: pairs) { std::tie(colors[c], words[c], orig_words[c]) = w; ++c; }
    }

    std::vector<std::thread> tasks;
    for(unsigned n=0; n<NUM_THREADS; ++n) tasks.emplace_back(Finder);
    for(auto& t: tasks) t.join();
}
