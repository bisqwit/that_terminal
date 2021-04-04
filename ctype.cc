#include <bitset>
#include <algorithm>
#include <cstring>
#include <array>

/*
#if __cplusplus <= 201800L
# include <codecvt>
# include <locale>
#endif
*/

#include "data/unidata.cc"
#include "ctype.hh"
#include "log2.hh"

template<std::size_t N>
class constexpr_bitset
{
public:
    using elem_t = unsigned long;
    static constexpr std::size_t WordSize = sizeof(elem_t) * 8;
    static constexpr std::size_t nwords   = (N + WordSize-1) / WordSize;
    static constexpr elem_t      full = ~elem_t{};
    elem_t data[nwords] {};
public:
    constexpr void set(std::size_t idx)
    {
        data[idx/WordSize] |= elem_t(1) << (idx % WordSize);
    }
    constexpr bool test(std::size_t idx) const
    {
        return data[idx/WordSize] & (elem_t(1) << (idx % WordSize));
    }

    constexpr void set_n(std::size_t idx, std::size_t count)
    {
        std::size_t widx    = idx / WordSize;
        std::size_t woffs   = idx % WordSize;
        std::size_t rembits = WordSize - woffs;

        while(count >= rembits)
        {
            data[widx++] |= full << woffs;
            count  -= rembits;
            woffs   = 0;
            rembits = WordSize;
        }
        if(count > 0)
        {
            data[widx] |= ~(full << count) << woffs;
        }
    }
};

template<unsigned nwords, unsigned bits_per_elem>
class bitval_array
{
public:
    using elem_t = unsigned long;
    static constexpr unsigned elems_per_mapword = sizeof(elem_t)*8 / bits_per_elem;
    static constexpr unsigned n_mapwords        = (nwords + elems_per_mapword-1) / elems_per_mapword;
public:
    elem_t data[n_mapwords] {};

    constexpr void set(std::size_t index, unsigned value)
    {
        data[index / elems_per_mapword] |= elem_t(value) << ((index % elems_per_mapword) * bits_per_elem);
    }
    constexpr unsigned get(std::size_t index) const
    {
        return get_from(data[get_idx(index)], index);
    }
    static constexpr unsigned get_idx(std::size_t index)
    {
        return index / elems_per_mapword;
    }
    static constexpr unsigned get_from(elem_t elem, std::size_t index)
    {
        return (elem >> ((index % elems_per_mapword) * bits_per_elem))
             % (1u << bits_per_elem);
    }
};

#define USE_MAP2

template<std::size_t N, std::size_t maxdim = 252, std::size_t maxdim2 = 84>
class compressed_bitset
{
    using reftype = constexpr_bitset<N>;
    using elem_t  = typename reftype::elem_t;
    static constexpr std::size_t WordSize = reftype::WordSize;
    static constexpr std::size_t nwords   = reftype::nwords;
    static constexpr elem_t      full     = reftype::full;

    using mapelem_t = elem_t;//unsigned char;
    static constexpr unsigned bits_per_elem     = log2ceil(maxdim + 2);
    static_assert( maxdim <= (1u<<bits_per_elem) );

    // For "nwords", specify whether that is
    //               fully 0
    //               fully 1
    //               something else: included in container
    elem_t container[maxdim] {};

    using map1_type = bitval_array<nwords, bits_per_elem>;

#ifdef USE_MAP2
    static constexpr unsigned bits_per_elem2    = log2ceil(maxdim2);
    static_assert( maxdim2 <= (1u<<bits_per_elem2) );
    typename map1_type::elem_t container2[maxdim2] {};
    bitval_array<map1_type::n_mapwords, bits_per_elem2> map2;
#else
    map1_type map1;
#endif

public:
    constexpr void init(reftype&& b)
    {
#ifdef USE_MAP2
        map1_type map1{};
#endif
        unsigned dim=0;
        for(unsigned a=0; a<nwords; ++a)
            if(b.data[a] == 0)
                map1.set(a, 0);
            else if(b.data[a] == full)
                map1.set(a, 1);
            else
            {
                container[dim] = b.data[a];
                for(unsigned p=0; p<=dim; ++p)
                    if(container[p] == b.data[a])
                    {
                        map1.set(a, p+2);
                        if(p == dim) ++dim;
                        break;
                    }
            }
#ifdef USE_MAP2
        // next, compress map1 similarly.

        unsigned dim2=0;
        for(unsigned a=0; a<map1_type::n_mapwords; ++a)
        {
            container2[dim2] = map1.data[a];
            for(unsigned p=0; p<=dim2; ++p)
                if(container2[p] == map1.data[a])
                {
                    map2.set(a, p);
                    if(p == dim2) ++dim2;
                    break;
                }
        }
#endif
    }

    bool test(std::size_t idx) const;// __attribute__((noinline));
};

template<std::size_t N, std::size_t maxdim, std::size_t maxdim2>
bool compressed_bitset<N,maxdim,maxdim2>::test(std::size_t idx) const
{
    if(idx >= N) return false;

#ifdef USE_MAP2
    unsigned w1 = map2.get(map1_type::get_idx(idx/WordSize));
    unsigned w  = map1_type::get_from(container2[w1], idx/WordSize);
#else
    unsigned w = map1.get(idx/WordSize);
#endif
    if(w & ~1u) return container[w-2] & (elem_t(1) << (idx % WordSize));
    else        return w&1;
}

constexpr unsigned cap = 0x10FFFE; // Co_table includes 0x10fffd

template<std::size_t maxdim, std::size_t maxdim2>
static constexpr auto BuildIntervals(std::initializer_list<std::pair<const std::pair<char32_t,char32_t>*, std::size_t>> arrays)
{
    constexpr_bitset<cap> set;
    for(auto a: arrays)
        for(std::size_t n=0; n<a.second; ++n)
            set.set_n(a.first[n].first, a.first[n].second - a.first[n].first + 1);
    compressed_bitset<cap,maxdim,maxdim2> result;
    result.init(std::move(set));
    return result;
}

#define B(c) /*std::pair<const std::pair<char32_t,unsigned>*, std::size_t>*/{&c##_table[0],std::size(c##_table)}
bool isupper(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<62,21>({B(Lu)});
    return intervals.test(c);
}
bool islower(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<69,24>({B(Ll)});
    return intervals.test(c);
}
bool isalpha(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<256,94>({B(Mc),B(Me),B(Mn),B(Lm),B(Lt),B(Lo),B(Ll),B(Lu)});
    return intervals.test(c);
}
bool isalnum(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<275,100>({B(Mc),B(Me),B(Mn),B(Lm),B(Lt),B(Lo),B(Ll),B(Lu),B(Nl),B(No),B(Nd)});
    return intervals.test(c);
}
bool isalnum_(char32_t c)
{
    static constexpr std::array cd2_table{ std::pair<char32_t,char32_t>{0x200C,0x200D} };
    static constexpr auto intervals = BuildIntervals<259,95>({B(Lm),B(Lt),B(Lo),B(Ll),B(Lu),B(Pc),B(Nl),B(Mn),B(Mc),B(Nd),B(cd2)});
    return intervals.test(c);
}
bool isdigit(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<11,24>({B(Nd)});
    return intervals.test(c);
}
bool isxdigit(char32_t c)
{
    static constexpr std::array hex_table{ std::pair<char32_t,char32_t>{U'a',U'f'}, std::pair<char32_t,char32_t>{U'A',U'F'} };
    static constexpr auto intervals = BuildIntervals<12,24>({B(Nd),B(hex)});
    return intervals.test(c);
}
bool ispunct(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<108,48>({B(Pf),B(Pi),B(Pc),B(Pd),B(Pe),B(Ps),B(Po)});
    return intervals.test(c);
}
static constexpr std::array bl_table{ std::pair<char32_t,char32_t>{U'\t',U'\t'} };
static constexpr std::array ws_table{ std::pair<char32_t,char32_t>{U'\r',U'\r'},
                                      std::pair<char32_t,char32_t>{U'\n',U'\n'},
                                      std::pair<char32_t,char32_t>{U'\f',U'\f'} };
bool isspace(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<6,6>({B(Zp),B(Zl),B(Zs),B(bl),B(ws)});
    return intervals.test(c);
}
bool isspace_punct(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<108,48>({B(Zp),B(Zl),B(Zs),B(bl),B(ws),B(Pf),B(Pi),B(Pc),B(Pd),B(Pe),B(Ps),B(Po)});
    return intervals.test(c);
}
bool isblank(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<6,6>({B(Zs),B(bl)});
    return intervals.test(c);
}
bool isctrl(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<19,15>({B(Co),B(Cs),B(Cf),B(Cc),B(Zl),B(Zp)});
    return intervals.test(c);
}
bool isprint(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<268,102>({B(Co),B(Nl),B(Mc),B(Me),B(Mn),B(Lm),B(Lt),B(Pf),B(No),B(Pi),
    B(Lo),B(So),B(Ll),B(Pc),B(Sk),B(Lu),B(Nd),B(Pd),B(Sm),B(Pe),B(Ps),B(Sc),B(Po),B(Zs)});
    return intervals.test(c);
}
bool isgraph(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<270,102>({B(Co),B(Nl),B(Mc),B(Me),B(Mn),B(Lm),B(Lt),B(Pf),B(No),B(Pi),
    B(Lo),B(So),B(Ll),B(Pc),B(Sk),B(Lu),B(Nd),B(Pd),B(Sm),B(Pe),B(Ps),B(Sc),B(Po)});
    return intervals.test(c);
}
bool isnotword(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<112,56>({B(Co),B(Cs),B(Zp),B(Zl),B(Pf),B(Cf),B(Pi),
    B(Pc),B(Pd),B(Pe),B(Ps),B(Po),B(Zs),B(Cc)});
    return intervals.test(c);
}
bool isdouble(char32_t c)
{
    static constexpr auto intervals = BuildIntervals<64,34>({B(width)});
    return intervals.test(c);
}


static bool casecomp(const std::tuple<char32_t,unsigned,int>& v, char32_t c)
{
    return c < std::get<0>(v);
}
template<typename I>
static char32_t caseconv(I begin, I end, char32_t c) __attribute__((noinline));

template<typename I>
static char32_t caseconv(I begin, I end, char32_t c)
{
    auto i = std::lower_bound(begin, end, c, casecomp);
    if(i == end || c > std::get<1>(*i))
        return c;
    return c + std::get<2>(*i);
}

char32_t tolower(char32_t c)
{
    return caseconv(std::begin(tolower_table), std::end(tolower_table), c);
}
char32_t toupper(char32_t c)
{
    return caseconv(std::begin(toupper_table), std::end(toupper_table), c);
}
char32_t totitle(char32_t c)
{
    return caseconv(std::begin(totitle_table), std::end(totitle_table), c);
}

std::u32string FromUTF8(std::string_view s)
{
    std::u32string result;
    unsigned cache = 0/*, bytesleft = 0*/;
    constexpr unsigned bytesleftshift = 2, bytesleftmask = (1u << bytesleftshift)-1;
    constexpr unsigned con = 0b00000000000000000101010101011011u;
    for(unsigned char c: s)
    {
    #if 0
        if(bytesleft > 0)           { cache = cache * 0x40 + (c & 0x3F); --bytesleft; }
        else if((c & 0xE0) == 0xC0) { cache = c & 0x1F; bytesleft=1; }
        else if((c & 0xF0) == 0xE0) { cache = c & 0x0F; bytesleft=2; }
        else if((c & 0xF8) == 0xF0) { cache = c & 0x07; bytesleft=3; }
        else                        { cache = c & 0x7F;              }
        if(!bytesleft)
        {
            result += char32_t(cache);
        }
    #else
        unsigned c0 = (cache & bytesleftmask);
        unsigned c1 = (((cache >> bytesleftshift) * 0x40u + (c & 0x3F)) << bytesleftshift) + (c0-1);
        unsigned c2 = ((con << ((c >> 4)*2)) & 0xFFFFFFFFu) >> 30; // number of trailing bytes (0-3) given this first byte
        c2         += ((c & (0x070F1F7Fu >> (c2*8))) << bytesleftshift);
        cache = c0 ? c1 : c2;

        if(!(cache & bytesleftmask))
        {
            char32_t c = cache >> bytesleftshift;
            if(__builtin_expect(result.empty(), false)
            || __builtin_expect(c < 0xDC00 || c > 0xDFFF, true)
            || __builtin_expect(result.back() < 0xD800 || result.back() > 0xDBFF, false))
                result += c;
            else
                result.back() = (result.back() - 0xD800u)*0x400u + (c - 0xDC00u) + 0x10000u;
        }
    #endif
    }
    return result;
}

std::string ToUTF8(std::u32string_view s)
{
    std::string result;
    alignas(16) static constexpr unsigned S[4] = {0x7F,0x3F,0x3F,0x3F}, q[4] = {0xFF,0,0,0}, o[4] = {0,0x80,0x80,0x80};
    for(char32_t c: s)
    {
        /**/
        unsigned n = (c >= 0x80u) + (c >= 0x800u) + (c >= 0x10000u);
        /*
        alignas(4) char rbuf[4] =
            { char((((0xF0E0C000u) >> (n*8)) & 0xFF) + (c >> 6*n)),
              char(0x80 + ((c >> 6*(n-1)) & 0x3F)),
              char(0x80 + ((c >> 6*(n-2)) & 0x3F)),
              char(0x80 + ((c >> 6*(n-3)) & 0x3F)) };
        result.append(rbuf, n+1);
        */
        /*
        alignas(16) unsigned w[4] = { (((0xF0E0C000u) >> (n*8u)) & 0xFFu), 0,0,0 };
        alignas(16) constexpr unsigned s[4] = {0x7F,0x3F,0x3F,0x3F}, o[4] = {0,0x80,0x80,0x80};
        #pragma omp simd
        for(unsigned m=0; m<4; ++m) w[m] += ((c >> ((n-m)*6u) & s[m]) + o[m]);// << (m*8u);
        result.append(w, w+n+1);*/
        unsigned val = 0xF0E0C000u >> (n*8u);
        alignas(16) unsigned w[4]={val,val,val,val};
        #pragma omp simd
        for(unsigned m=0; m<4; ++m)
            w[m] = ((w[m] & q[m]) + ((c >> ((n-m)*6u) & S[m]) + o[m])) << (m*8u);
        unsigned sum = 0;
        #pragma omp simd
        for(unsigned m=0; m<4; ++m) sum += w[m];
        alignas(4) char temp[4];
        for(unsigned m=0; m<4; ++m) temp[m] = sum >> (m*8);
        result.append(temp, n+1);
        /*
        unsigned val = ((((0xF0E0C000u) >> (n*8u)) & 0xFFu) + (c >> 6u*n))
                     + 0x80808000u
                     + (((c >> 6u*(n-1)) & 0x3Fu) << 8u)
                     + (((c >> 6u*(n-2)) & 0x3Fu) << 16u)
                     + (((c >> 6u*(n-3)) & 0x3Fu) << 24u);
        alignas(4) char rbuf[4] = {char(val),char(val>>8),char(val>>16),char(val>>24)};
        result.append(rbuf, n+1);
        */
        /**/
        /*
        if(c < 0x80)
        {
            result += c;
        }
        else if(c < 0x800)
        {
            result += char(0xC0 + (c>>6));
            result += char(0x80 + (c & 0x3F));
        }
        else if(c < 0x10000)
        {
            result += char(0xE0 + (c>>12));
            result += char(0x80 + ((c>>6) & 0x3F));
            result += char(0x80 + (c & 0x3F));
        }
        else
        {
            result += char(0xF0 + (c>>18));
            result += char(0x80 + ((c>>12) & 0x3F));
            result += char(0x80 + ((c>>6) & 0x3F));
            result += char(0x80 + (c & 0x3F));
        }
        */
    }
    return result;
}

template<typename C>
alignas(32) static const C spaces[32]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                           ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
template<std::size_t Nbytes, typename C, typename T = unsigned>
static inline bool compa(const void* a)
{
    const T* aa = (const T*)a;
    const T* bb = (const T*)spaces<C>;
    T result = ~T();
    #pragma omp simd reduction(&:result)
    for(unsigned n=0; n<Nbytes / sizeof(T); ++n)
        result &= (aa[n] == bb[n]);
    // If result==0, then a mismatch was found
    return result;
}

//#include <iostream>
std::size_t CountIndent(std::u32string_view text, std::size_t begin)
{
    typedef std::remove_const_t<std::remove_reference_t<decltype(text[0])>> C;
    std::size_t oldbegin = begin, size = text.size();
    const auto data = text.data();

/**/
#if defined(__clang__) || defined(__ICC)
    while(begin+32u/sizeof(C) <= size && compa<32u,C,unsigned long>(data+begin)) { begin += 32u/sizeof(C); }
    while(begin+8u/sizeof(C) <= size && std::memcmp(data+begin, spaces<C>, 8u)==0) { begin += 8u/sizeof(C); }
#else
    while(begin+16u/sizeof(C) <= size && compa<16u,C,unsigned long>(data+begin)) { begin += 16u/sizeof(C); }
    if(begin+8u/sizeof(C) <= size && std::memcmp(data+begin, spaces<C>, 8u)==0) { begin += 8u/sizeof(C); }
#endif
/**/

    //if(begin+16u <= size && !compa<16u,C,unsigned int>(data+begin)) { begin += 16u; }
    //if(begin+8u <= size && !compa<8u,unsigned long>(data+begin, spaces<C>)) { begin += 8u;  }
    //while(begin < size && data[begin]==' ') { begin += 1u; }
    std::size_t pos = text.find_first_not_of(C(' '), begin);
    if(pos == text.npos) pos = text.size();
    //std::cerr << "Indent in <" << ToUTF8(text.substr(oldbegin)) << "> (oldbegin="<<oldbegin<<",begin="<<begin<<",pos="<<pos<<" is " << (pos-oldbegin) << "\n";
    return pos - oldbegin;
}

static unsigned short cp437_uni(unsigned n)
{
    /* Minmax for 0..47: 161, 8976 */
    /* Minmax for 48..54: 9474, 9571 */
    /* Minmax for 55..121: 160, 9632 */
    static const unsigned short cp437_uni_uni_tab[122] =
    {
        199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 
        236, 196, 197, 201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 
        220, 162, 163, 165, 8359,402, 225, 237, 243, 250, 241, 209, 170, 
        186, 191, 8976,172, 189, 188, 161, 171, 187, 9474,9508,9569,9570,
        9558,9557,9571,9564,9563,9488,9492,9524,9516,9500,9472,9532,9566,
        9567,9562,9556,9577,9574,9568,9552,9580,9575,9576,9572,9573,9561,
        9560,9554,9555,9579,9578,9496,9484,9608,9604,9612,9616,9600,945, 
        223, 915, 960, 931, 963, 181, 964, 934, 920, 937, 948, 8734,966, 
        949, 8745,8801,177, 8805,8804,8992,8993,247, 8776,176, 8729,183, 
        8730,8319,178, 9632,160, 
    };
    return 
        (n<128)? (n)/*128*/
          : (n<189)
            ? (n<176)? cp437_uni_uni_tab[n-128]/*48*/
              : (n<179)? (n + 9441)/*3*/
                : (n<186)? cp437_uni_uni_tab[n-131]/*7*/
                         : ( 6*n + 8437)/*3*/
            : cp437_uni_uni_tab[n-134]/*67*/;
}

std::u32string FromCP437(std::string_view s)
{
    std::u32string result;
    for(unsigned char c: s)
        result += (char32_t)cp437_uni(c);
    return result;
}
