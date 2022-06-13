#include <iterator>
#include <cstdint>

template<unsigned mod>
static unsigned Mod(std::uint64_t value) __attribute__((noinline));
template<unsigned mod>
static unsigned Mod(std::uint64_t value)
{
    return value % mod;
}

typedef unsigned (*ModFunc)(std::uint64_t);

#define d(n) Mod<n>,
#define d5(n) d(n)d(n+1)d(n+2)d(n+3)d(n+4)
#define d20(n) d5(n)d5(n+5)d5(n+10)d5(n+15)
#define d100(n) d20(n)d20(n+20)d20(n+40)d20(n+60)d20(n+80)
#define d500(n) d100(n)d100(n+100)d100(n+200)d100(n+300)d100(n+400)
#define d2000(n) d500(n)d500(n+500)d500(n+1000)d500(n+1500)
ModFunc mod_table[] =
{
    d2000(1)
    d2000(2001)
    d2000(4001)
    d2000(6001)
    d2000(8001)
    d2000(10001)
    d2000(12001)
    d2000(14001)
    d2000(16001)
    d2000(18001)
};
extern const unsigned mod_table_size;
const unsigned mod_table_size = std::size(mod_table);


constexpr unsigned bound = 16384;

template<unsigned est_size>
unsigned FastModDef(std::uint64_t value, unsigned mod)
{
    switch(mod)
    {
        #define c(n) case n: if constexpr((n) < bound) return Mod<n>(value); else goto dfl;
        #define c5(n) c(n)c(n+1)c(n+2)c(n+3)c(n+4)
        #define c20(n) c5(n)c5(n+5)c5(n+10)c5(n+15)
        #define c100(n) c20(n)c20(n+20)c20(n+40)c20(n+60)c20(n+80)
        #define c500(n) c100(n)c100(n+100)c100(n+200)c100(n+300)c100(n+400)
        #define c2000(n) c500(n)c500(n+500)c500(n+1000)c500(n+1500)
        c2000(est_size)
        c2000(est_size+2000)
        c2000(est_size+4000)
        c2000(est_size+6000)
        c2000(est_size+8000)
        default: dfl: return value % mod;
    }
    __builtin_unreachable();
}

template unsigned FastModDef<753>(std::uint64_t, unsigned);
