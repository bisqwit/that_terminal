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

static constexpr unsigned NUM_THREADS = 44;

/* Utility that finds a hashing for given words for jsf.hh in my editor */



std::size_t Hash(std::string_view s, std::size_t start,
                                     std::size_t mul,
                                     std::size_t mul2,
                                     std::size_t mul3,
                                     std::size_t add,
                                     std::size_t permut,
                                     std::size_t letters,
                                     unsigned shift)
{
#if 0
    std::size_t result = start;
    #pragma omp simd reduction(+:result)
    for(std::size_t p=0; p < s.size(); ++p)
    {
        unsigned sh = (shift+p*8)%64;
        std::size_t val = mul*(unsigned char)s[p] + add;
        val = (val << sh) | (val >> (64-sh));
        result += val*permut;
    }
    return result;
#endif
#if 1
    std::size_t result = start;
    //unsigned s1 = 1 + 8 * ((shift>>0)&7);
    //unsigned s2 = 1 + 8 * ((shift>>3)&7);
    unsigned s1 = 1 + 16 * ((shift>>0)&3);
    unsigned s2 = 1 + 16 * ((shift>>2)&3);
    unsigned s3 = 1 + 16 * ((shift>>4)&3);
    //unsigned s4 = 1 + 16 * ((shift>>6)&3);

    // Alphabet: A-Z0-9 space (total: 37)
    //std::size_t digits = 0, spaces = 0;
    /*static const unsigned char cset[]
    {
    //    0,1,2,3,4,5,6,7,8,9, // digits
        0,0,1,2,0,           // abcde
        0,0,0,0,0,           // fghij
        1,1,2,2,1,          // kplmn
        0,0,2,1,3,           // opqrs
        0,0,0,0,0,           // tvwxy
        0                    // z
    };*/
    for(unsigned char c: s)
    {
        if(c==' ')continue;
        if(c <= '9')
        {
            c -= '0';
            c = (mul3 >> (c*6)) & 63;
        }
        else
        {
            c -= 'a';
            c = (letters >> (c*2)) & 3;
            c = (mul2 >> (c*8)) & 255;
        }
        //c = cset[c==' ' ? 0 : c<='9' ? c-'0'+1 : (c-'a'+11)];
        /*
        c = (c>='0' && c<='9') ? c-'0'
           : c==' ' ? (mul2 >> (26*2))
           : (10+((mul2 >> (2*(c-'a'))) & 3));
          */

        /*if(c < 10)
            digits = digits*10 + c;
        else*/
        {
            /*if(c >= '0' && c <= '9') c = c+1 - '0';
            else if(c >= 'A' && c <= 'Z') c = c-'A' + 11;
            else if(c >= 'a' && c <= 'z') c = c-'a' + 11;
            else if(c == ' ') c = 0;
            */

            result ^= result >> s1;
            result *= permut;//0xff51afd7ed558ccd;
            result ^= result >> s2;
            result *= add;//0xc4ceb9fe1a85ec53;
            result ^= result >> s3;
            result *= mul;

            //result ^= result >> s4;
            //result *= mul2;
            result += c;
            //result += add;
            /*
            result = result*mul + add;
            */
            //result = (result << shift) | (result >> (64-shift));
            /*
            result ^= c*permut;
            */
        }
    }
    return result;// + digits*mul2 /*+ spaces*permut*/;
#endif
}

static unsigned __attribute__ ((target("rdrnd"))) __x86_rdrand32(void)
{
    unsigned int retries = 100;
    unsigned val=0;

    while (_rdrand32_step(&val) == 0)
        if (--retries == 0)
            std::__throw_runtime_error(__N("random_device::__x86_rdrand32(void)"));

    return val;
}
static unsigned long long __attribute__ ((target("rdrnd"))) __x86_rdrand64(void)
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
    std::vector<std::string> modded(words, words+N);
    for(auto& w: modded)
        w.erase(std::remove_if(w.begin(), w.end(), [=](char c){return c==' ';}), w.end());

    std::vector<std::string> copy = modded;

    for(std::size_t a=0; a<modded.size(); ++a)
        for(char& c: modded[a])
            if(c >= 'a' && c <= 'z')
            {
                c = (letters >> ((c-'a')*2)) & 3;
                if(c == 3) return false;
            }

    for(std::size_t a=0; a<modded.size(); ++a)
        for(std::size_t b=a+1; b<modded.size(); ++b)
            if(colors[a] != colors[b]
            && modded[a] == modded[b]
            && copy[a] != copy[b])
                return false;

    return true;
}
static bool TestMul2Validity(std::size_t value)
{
    unsigned char c1 = value, c2 = value >> 8, c3 = value >> 16;
    return !(c1 == c2 || c1 == c3 || c2 == c3);
}
static bool TestMul3Validity(std::size_t value)
{
    unsigned bits = 0;
    #pragma omp simd reduction(|:bits)
    for(unsigned n=0; n<10; ++n)
        bits |= 1ull << ((value >> (n*6)) & 63);
    return __builtin_popcount(bits) == 10;
}

int main(int /*argc*/, char** argv)
{
    bool ucase = std::toupper(argv[1][0]) == 'U';

    std::vector<std::pair<unsigned,std::string>> pairs;
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
            //std::fprintf(stderr, "Got <%s> with 0x%06X\n", s.c_str(), color);
            pairs.emplace_back(color, std::move(s));
        }
        //if(action_values.size() >= 4096) break;
    }
    // Sort the pairs so that identical colors are consecutive
    //std::sort(pairs.begin(), pairs.end());
    constexpr unsigned est_size = 753;
    assert(pairs.size() == est_size);

    std::string_view words[est_size];
    unsigned         colors[est_size];
    {unsigned c=0;
    for(auto& w: pairs) words[c++] = w.second;
    c=0;
    for(auto& w: pairs) colors[c++] = w.first;}

    std::mutex lock;

/*
Best so far:

//Good: shift=0x37, start=0xC5959135F5F4EE52,mul=0xD43C17959D1B8457,add=0x3B4236FA60071A28,perm=0x9833CB76814DF6DB, mod=7709  distance = 7643  min=27 max=7669

*/
    constexpr unsigned bound = 3500; // mod=5699 found already

    std::atomic<unsigned> best_dist;
    best_dist = bound;

    std::vector<std::thread> tasks;
    for(unsigned n=0; n<NUM_THREADS; ++n) tasks.emplace_back([&]{

    std::seed_seq seq{ __x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64(),
                       __x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64(),__x86_rdrand64()  };
    std::mt19937_64 e2(seq);

    //alignas(16) unsigned short used[bound];
    alignas(16) std::uint_least64_t used[(bound+127)/64];
    alignas(16) unsigned short      what[bound];
    alignas(16) std::size_t hash[est_size];
    std::size_t letters=0, letters_uses=0;
    for(;;)
    {
        std::size_t start = e2();
        std::size_t mul   = e2();
        std::size_t add   = e2();
        std::size_t permut= e2();
        std::size_t mul2;
        std::size_t mul3;
        do {
            mul2  = e2() % (1ull << (8*4));
        } while(!TestMul2Validity(mul2));
        do {
            mul3  = e2() % (1ull << (6*10));
        } while(!TestMul3Validity(mul3));
        if(!letters_uses--)
        {
            do {
                letters   = e2() % (1ull << (26*2));
            } while(!TestMapValidity(letters, words, colors));
            letters_uses = 32768;
        }

        unsigned shift = e2()%64; // 6 bits
        //unsigned shift = e2()%256; // 8 bits

        //unsigned shift = e2()%16; // 4 bits
        //for(unsigned shift=0; shift<64; ++shift)
        for(unsigned w=0; w<est_size; ++w) hash[w] = Hash(words[w], start,mul,mul2,mul3, add,permut, letters, shift);

        bool accepted = false;
        unsigned min = ~0u, max = 0;
        unsigned mod = est_size;
        unsigned cap = std::min(bound, best_dist + 64);
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
            unsigned c = hash[w] % mod;
            min = max = c;
            auto& usedword = used[c/64];
            auto  usedmask = 1ull << (c%64);
            usedword |= usedmask;
            what[c] = w;

            for(++w; w<est_size; ++w)
            {
                unsigned c = hash[w] % mod;
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
            std::size_t c = Hash(words[w], start,mul,mul2,mul3, add,permut, letters, shift);
            data[c % mod].push_back(w);
        }

        best_dist = dist;
        printf("//Good: shift=0x%X, start=0x%zX,mul=0x%zX,add=0x%zX,perm=0x%zX,letters=0x%zX, mod=%u  distance = %u  min=%u max=%u\n",
            shift,start,mul,add,permut,letters, mod, dist,min,max);

        printf(R"(
static constexpr std::uint_least64_t a = 0x%zX;
static constexpr std::uint_least64_t b = 0x%zX;
static constexpr std::uint_least64_t c = 0x%zX;
static constexpr std::uint_least64_t d = 0x%zX;
static constexpr std::uint_least64_t e = 0x%zX;
static constexpr std::uint_least64_t f = 0x%zX;
static constexpr std::uint_least64_t g = 0x%zX;
static constexpr unsigned mod          = %u;
static constexpr unsigned shift        = 0x%X;

static constexpr unsigned hash(const char* s, std::size_t length)
{
    std::uint_least64_t result = a;
    unsigned s1 = 1 + 16 * ((shift>>0)&3);
    unsigned s2 = 1 + 16 * ((shift>>2)&3);
    unsigned s3 = 1 + 16 * ((shift>>4)&3);
    while(length--)
        if(unsigned char z = *s++; z != ' ')
        {
            if(z <= '9')
                { z -= '0'; z = 24+z*4; z = f>>z; z &= 15; }
            else
                { z -= 'a'; z = z*2;    z = g>>z; z &= 3;
                            z = z*8;    z = e>>z; z &= 255; }
            result = (result ^ (result >> s1)) * b;
            result = (result ^ (result >> s2)) * c;
            result = (result ^ (result >> s3)) * d;
            result += z;
        }
    return result %% mod;
}

#define docolors(o) \
)",
  start, permut,add,mul,mul2,mul3,letters, mod,shift
);
        for(const auto& [hash,indexes]: data)
        {
            std::string extra, name;
            unsigned    color = 0xDEADCAFE;
            for(auto w: indexes)
            {
                if(name.empty())
                    name = words[w];
                else
                {
                    fprintf(stderr, "\tgot a reuse (0x%06X = %s = %s)!\n",
                        color, name.c_str(), words[w].data());
                    if(color != colors[w])
                    {
                        fprintf(stderr, "something is odd with <%s> and <%s>\n",
                            name.c_str(), words[w].data());
                    }
                    if(extra.empty())
                        extra = "and \"";
                    else
                        extra += ", \"";
                    extra += words[w];
                    extra += '"';
                }
                color = colors[w];
            }
            if(!extra.empty()) extra = "/* " + extra + " */";
            std::printf("        o(%4u, 0x%06X, \"%s\") %s\\\n",
                hash, color, name.c_str(), extra.c_str());
        }

        fflush(stdout);
    }});
    for(auto& t: tasks) t.join();
}
