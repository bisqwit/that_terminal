#include <map>
#include <regex>
#include <string>
#include <ranges>
#include <fstream>
#include <ostream>
#include <functional>
#include <unordered_set>

#include "share.hh"
#include "ctype.hh"
#include "make_similarities.hh"

using namespace std::literals;

static constexpr auto Ropt = std::regex_constants::ECMAScript | std::regex_constants::optimize;

static auto UnicodeDataFileName()
{
    return FindShareFile("unicode/UnicodeData.txt", {"/usr/local/share", "/usr/share"});
    /* Try UnicodeData.txt in the following:

        <path>/share/unicode/
        <home>/.local/share/<prog>/unicode/
        /usr/local/share/<prog>/unicode/
        /usr/share/<prog>/unicode/
        /usr/share/unicode

     * Where
        <prog> = that_terminal
        <path> = pathname(argv[0])
     */
}

static std::string u8(char32_t code)
{
    return ToUTF8(std::u32string_view(&code,1));
}

static std::vector<std::string_view> Split(std::string_view str, std::string_view delimiter)
{
    std::vector<std::string_view> result;
    if(!str.empty())
        for(std::size_t pos = 0; ; )
        {
            std::size_t d = str.find(delimiter, pos);
            if(d == str.npos) { result.emplace_back(str.data()+pos, str.size()-pos); break; }
            result.emplace_back(str.data()+pos, d-pos);
            pos = d + delimiter.size();
        }
    return result;
}
template<typename T>
static std::string Join(const T& elems, std::string_view delim)
{
    std::string result;
    for(auto i = elems.begin(); i != elems.end(); ++i)
    {
        if(i != elems.begin()) result += delim;
        result += *i;
    }
    return result;
}
template<typename T, typename U>
static std::string Join(const std::map<T,U>& elems, std::string_view delim)
{
    std::string result;
    for(auto i = elems.begin(); i != elems.end(); ++i)
    {
        if(i != elems.begin()) result += delim;
        result += i->second;
    }
    return result;
}

/** Contents of UniData.txt in a parsed format */
struct UniData
{
    std::map<std::string, char32_t> codes; ///< Translation of names into codepoints
    std::map<char32_t, std::string> names; ///< Translation of codepoints into names

    /** Load from @param unipath file */
    void Load(std::filesystem::path unipath)
    {
        std::regex pattern("([0-9A-F]+);([^;]+);.*", Ropt);
        std::regex mathpat("(MATHEMATICAL.*) (CAPITAL|SMALL) ([A-Z])", Ropt);

        std::ifstream f(unipath);
        for(std::string line; std::getline(f, line), f; )
            if(std::smatch mat; std::regex_match(line, mat, pattern))
            {
                std::string name = mat[2];
                char32_t    code = std::stoi(mat[1], nullptr, 16);

                // Rename math symbols with "CAPITAL L" into with "LATIN CAPITAL LETTER L"
                // to get better matches.
                if(std::regex_match(name, mat, mathpat))
                {
                    name = std::string(mat[1]) + " LATIN " + std::string(mat[2]) + " LETTER " + std::string(mat[3]);
                }

                codes[name] = code;
                names[code] = name;
            }
    }
    /** Query whether the given name is defined.
     * @param s name to search for
     * @returns codepoint if known, 0 otherwise
     */
    char32_t Has(const std::string& s) const
    {
        if(auto i = codes.find(s); i != codes.end()) return i->second;
        return 0;
    }
    /** Query whether the given codepoint has a name.
     * @param c codepoint to search for
     * @returns True if the codepoint has a known name.
     */
    bool Has(char32_t c) const
    {
        return names.find(c) != names.end();
    }
};

void MakeSimilarities(std::ostream& out, std::filesystem::path unipath) // Same as old "similarities.dat"
{
    UniData d;
    d.Load(unipath);

    // Create special matching recipe for ASCII from FULLWIDTH
    // Do this BEFORE the MATHEMATICAL section
    // to avoid your ASCII letters looking completely silly.
    out << "// If this is fullwidth font, create ASCII characters from FULLWIDTH characters\n";

    std::regex fullhalf("(FULLWIDTH|HALFWIDTH) (.*)", Ropt);
    char32_t tmp;
    for(auto&[name,code]: d.codes)
        if(std::smatch mat; std::regex_match(name, mat, fullhalf) && (tmp = d.Has(mat[2])))
            out << "→ " << u8(tmp) << u8(code) << '\n';

    out << R"(
// Insert some manually crafted rules between pre-composed encircled or stylished letters
// Do this before the MATH section may alias doublestruck R (𝕉) with regular R
// when the font may in fact have doublestruck R (ℝ) in the letterlike section.
= ℂ𝔺
= ℊ𝒼
= ℋ𝔋
= ℌ𝔥
= ℍ𝔿
= ℎ𝑕
= ℐ𝒤
= ℑ𝔌
= ℒ𝒧
= ℓ𝓁
= ℕ𝕅
= ℙ𝕇
= ℚ𝕈
= ℛ𝒭
= ℜ𝔕
= ℝ𝕉
= ℤ𝕑
= K𝖪
= ℬ𝒝
= ℭ𝔆
= ℮𝕖
= ℯ𝑒
= ℳ𝒨
= ℴ𝓄
= ℹ𝐢
= ⅅ𝔻
= ⅆ𝕕
= ⅇ𝕖
= ⅈ𝕚
= ⅉ𝕛
= ⅀𝚺
= ℿ𝚷
= ℾ𝚪
= ℽ𝛄
= ℼ𝛑
= ℗Ⓟ
= ©Ⓒ
= ®Ⓡ

// Insert equality rules between symbols that are visually completely indiscernible
// First, ASCII-like characters
)";
    std::vector<std::u32string_view> equal_symbols{
        UR"(!ǃ)",
        UR"(#ⵌꖛ⌗⋕)",
        UR"(÷➗)",
        UR"(+➕ᚐ)",
        UR"(-−–➖)",
        UR"(.ꓸ)",
        UR"(,ꓹ)",
        UR"(ꓽ::׃꞉⁚ː∶։܃𝄈)",
        UR"(;;;ꓼ)",
        UR"(=ꘌ⚌゠᐀꓿)",
        UR"(/⟋╱⁄𝈺)",
        UR"(\⟍╲𝈻⧹⧵)",
        UR"(2շ)",
        UR"(3З𝟹ვ౩𝈆)",
        UR"(ɜзᴈ)",
        UR"(ƷӠ)",
        UR"(ʒӡ)",
        UR"(4Ꮞ)",
        UR"(8𐌚)",
        UR"(➊❶⓵➀①)",
        UR"(➋❷⓶➁②)",
        UR"(➌❸⓷➂③)",
        UR"(➍❹⓸➃④)",
        UR"(➎❺⓹➄⑤)",
        UR"(➏❻⓺➅⑥)",
        UR"(➐❼⓻➆⑦)",
        UR"(➑❽⓼➇⑧)",
        UR"(➒❾⓽➈⑨)",
        UR"(➓❿⓾➉⑩)",
        UR"(AΑАᎪᗅᗋ𐌀ꓐꓮ)",
        UR"(ĂӐᾸ)",
        UR"(ĀᾹ)",
        UR"(ÄӒ)",
        UR"(ÅÅ)",
        UR"(ÆӔ)",
        UR"(ʙвⲃ)",
        UR"(BΒВᏴⲂꕗ𐌁)",
        UR"(ƂБ)",
        UR"(ϭб)",
        UR"(CϹСᏟⲤⅭꓚ)",
        UR"(ϽƆↃꓛ)",
        UR"(DᎠ𐌃ᗞⅮⅮꓓ)",
        UR"(ꓷᗡ⫏)",
        UR"(EΕЕᎬⴹꗋ⋿ꓰ)",
        UR"(ꓱ∃Ǝⴺ)",
        UR"(ÈЀ)",
        UR"(ËЁ)",
        UR"(ĔӖ)",
        UR"(ƐԐ)",
        UR"(FϜ𝈓ߓᖴ𐌅ꓝ)",
        UR"(GᏀႺꓖ)",
        UR"(HΗНᎻⲎᕼꖾꓧ)",
        UR"(ʜнⲏ)",
        UR"(IΙІӀᏆⲒⅠꓲ)",
        UR"(ΪÏЇ)",
        UR"(JЈᒍلﻝᎫꓙ)",
        UR"(KΚКⲔᏦK𐌊ꓗ)",
        UR"(ḰЌ)",
        UR"(κᴋкⲕ)",
        UR"(LᏞᒪ𝈪Ⅼ˪ԼⅬԼⅬԼլꓡ)",
        UR"(MΜМᎷⲘϺ𐌑Ⅿꓟ)",
        UR"(ᴍмⲙ)",
        UR"(NΝⲚꓠ)",
        UR"(ͶИ)",
        UR"(ɴⲛ)",
        UR"(ͷи)",
        UR"(OΟОⲞ◯○⃝❍🌕߀ⵔՕ⚪⭕౦೦ꓳ)",
        UR"(ÖӦ)",
        UR"(ϴθѲӨƟᎾⲐ)",
        UR"(ΦФⲪ)",
        UR"(PΡРᏢⲢ𐌓ᑭ𐌛ꓑ)",
        UR"(ΠПⲠ)",
        UR"(QԚꝖႳႭⵕℚ)",
        UR"(RᎡ𝈖ᖇᏒꓣ)",
        UR"(Яᖆ)",
        UR"(SЅᏚՏႽꚂऽ𐒖ꕶꓢ)",
        UR"(ΣƩ∑⅀Ʃⵉ)",
        UR"(TΤТᎢⲦ⊤⟙ꔋ𝍮🝨⏉ߠꓔ)",
        UR"(U⋃ᑌ∪ՍՍꓴ)",
        UR"(VᏙᐯⴸ⋁𝈍ⅤⅤꓦ)",
        UR"(WԜᎳꓪ)",
        UR"(XΧХⲬ╳ⵝ𐌢Ⅹ𐌗Ⅹꓫ)",
        UR"(YΥҮⲨꓬ)",
        UR"(ΫŸ)",
        UR"(ZΖᏃⲌꓜ)",
        UR"(aа)",
        UR"(äӓ)",
        UR"(ăӑ)",
        UR"(æӕ)",
        UR"(əә)",
        UR"(ЬᏏ)",
        UR"(cϲсⲥⅽ)",
        UR"(ͻɔᴐↄ)",
        UR"(dⅾ)",
        UR"(eе)",
        UR"(ĕӗ)",
        UR"(ϵє)",
        UR"(ɛԑ)",
        UR"(gց)",
        UR"(гᴦⲅ)",
        UR"(iіⅰ)",
        UR"(ïї)",
        UR"(jϳј)",
        UR"(lⅼ)",
        UR"(ιɩ)",
        UR"(mⅿ)",
        UR"(ʌᴧ)",
        UR"(oοоסᴏⲟօ૦௦ഠ๐໐໐)",
        UR"(òὸ)",
        UR"(óό)",
        UR"(öӧ)",
        UR"(ɵөⲑ)",
        UR"(фⲫ)",
        UR"(pρрⲣ)",
        UR"(πпᴨⲡ)",
        UR"(яᴙ)",
        UR"(qԛ)",
        UR"(sѕ)",
        UR"(uս)",
        UR"(vᴠݍⅴ)",
        UR"(xⅹ)",
        UR"(yу)",
        UR"(~῀)",
        UR"(··•∙⋅・)",
        UR"(ᴛтⲧ)",
        UR"(૰。࿁)",
        UR"(ᐃ△🜂∆ΔᐃⵠΔꕔ)",
        UR"(ᐊᐊ◁⊲)",
        UR"(ᐁ▽🜄⛛∇ᐁ𝈔)",
        UR"(ᐅ▷⊳▻)",
        UR"(ᐱΛ𐤂⋀ⴷ𐌡Ʌ)",
        UR"(ᑎႶ⋂Ո∩𝉅ꓵ)",
        UR"(⨆∐ⵡ𝈈)",
        UR"(∏⨅ПΠ⊓)",
        UR"(⊏ⵎ𝈸)",
        UR"(コ⊐ߏ𝈹)",
        UR"(⎕□☐⬜◻▢⃞❑❒❐❏⧠⃢⌷ロ)",
        UR"(⛝⌧🝱)",
        UR"( 　         )"
    };
    for(auto w: equal_symbols)
        out << "= " << ToUTF8(w) << '\n';

    out << "\n// Create similarity rules between modified stylished symbols\n";
    if(1)
    {
        std::initializer_list<std::string_view> words
        {
            "BLACK",
            "HEAVY",
            "FULLWIDTH",
            "MATHEMATICAL BOLD",                  // 1D400,1D41A
            "MATHEMATICAL SANS-SERIF BOLD",       // 1D5D4,1D5EE
            "MATHEMATICAL SANS-SERIF BOLD ITALIC",// 1D63C,1D656
            "MATHEMATICAL BOLD ITALIC",           // 1D468,1D482
            "MATHEMATICAL ITALIC",                // 1D434,1D44E
            "MATHEMATICAL SANS-SERIF ITALIC",     // 1D608,1D622
            "",                                   // no modifier.
            "MATHEMATICAL SANS-SERIF",            // 1D5A0,1D5BA
            "MATHEMATICAL SCRIPT",                // 1D49C,1D4B6
            "MATHEMATICAL BOLD SCRIPT",           // 1D4D0,1D4EA
            "MATHEMATICAL FRAKTUR",               // 1D504,1D51E
            "MATHEMATICAL BOLD FRAKTUR",          // 1D56C,1D586
            "MATHEMATICAL DOUBLE-STRUCK",         // 1D538,1D552
            "MATHEMATICAL MONOSPACE",             // 1D670,1D68A
            "MATHEMATICAL",
            "WHITE",
            "LIGHT",
            "HALFWIDTH",
            "SMALL",
            "PARENTHESIZED",
            "CIRCLED",
            "TAG"
        };
        std::regex word_style_pat("(" + Join(words, "|") + ") (.*)", Ropt);
        std::map<std::string, std::map<std::string, char32_t>> symbols;
        for(const auto& [name,code]: d.codes)
        {
            symbols[name][""] = code;
            if(std::smatch mat; std::regex_match(name, mat, word_style_pat))
                symbols[mat[2]][mat[1]] = code;
        }
        for(auto&& [basename,group]: symbols)
            if(group.size() > 1)
            {
                out << "◆ ";
                for(auto w: words)
                    if(auto i = group.find(std::string(w)); i != group.end())
                        out << u8(i->second);
                out << '\n';
            }
    }

    // Convert the equal-symbols list into a searchable one
    std::map<char32_t, std::vector<char32_t>> equal_with;
    for(auto w: equal_symbols)
        for(char32_t code:  w)
        for(char32_t code2: w)
            if(code != code2)
                equal_with[code].push_back(code2);
    for(auto& w: equal_with)
    {
        std::sort(w.second.begin(), w.second.end());
        w.second.erase(std::unique(w.second.begin(), w.second.end()), w.second.end());
    }

    out << "// Then go through all symbols that are “WITH” something.\n"
           "// As a general rule, try to compose things that have more “WITHs”\n"
           "// from things that have less “WITHs”.\n";

    std::map<std::string, std::map<std::string, std::size_t>> with_lists;
    std::regex with_match("(.*) WITH (.*)", Ropt);
    for(const auto& [name,code]: d.codes)
        if(std::smatch mat; std::regex_match(name, mat, with_match))
        {
            std::string base = mat[1];
            std::string full = mat[2];
            std::vector<std::string_view> attrs = Split(full, " AND ");
            std::size_t len = attrs.size();
            auto& wl = with_lists[" WITH " + full];
            wl[""] = 0;
            for(std::size_t n = len; --n > 0; )
            {
                std::map<std::size_t, std::string_view> pick;
                std::function<void(std::size_t,std::size_t)> Do = [&](std::size_t index, std::size_t start)
                {
                    for(std::size_t a=start; a<len; ++a)
                    {
                        pick[index] = attrs[a];
                        if(index+1 == n)
                        {
                            wl[" WITH " + /*partial*/Join(pick, " AND ")] = pick.size();
                            //print "try make $partial from $full for $name for $base\n";
                        }
                        else
                            Do(index+1, a+1);
                    }
                };
                Do(0, 0);
            }
        }
    for(auto operation: std::initializer_list<std::string_view>{"→ ", "← "})
        for(auto& [full_with, p]: with_lists)
        {
            std::vector<std::pair<std::string,std::size_t>> partial_list(p.begin(), p.end());
            // Sort in descending order according to the score (second item)
            std::sort(partial_list.begin(), partial_list.end(),
                [](const auto& a, const auto& b) { return b.second > a.second; });
            // Find all symbols that have this "full with" list.
            std::regex pat("(.*)" + full_with, Ropt);
            for(auto& [name,code]: d.codes)
                if(name.size() >= full_with.size() && name.compare(name.size()-full_with.size(), full_with.size(), full_with) == 0)
                {
                    std::string_view mat1(name.data(), name.size()-full_with.size());
                    std::vector<std::tuple<char32_t, std::string_view, std::string_view>> rep_list, sub_list;
                    rep_list.emplace_back(code, name, mat1);
                    if(auto i = equal_with.find(code); i != equal_with.end())
                        for(auto code2: i->second)
                        {
                            const auto& name2 = d.names[code2];
                            std::smatch mat2;
                            std::regex_match(name2, mat2, with_match); // (.*) WITH.*
                            rep_list.emplace_back(code2, name2, std::string_view(&*mat2[1].first, mat2[1].second-mat2[1].first));
                        }
                    for(auto& [partial_with,dummy]: partial_list)
                        for(auto& rep: rep_list)
                        {
                            std::string sub_name{std::get<2>(rep)};
                            sub_name += partial_with;
                            //print "can we find $sub_name?\n";
                            if(auto code2 = d.Has(sub_name); code2)
                                sub_list.emplace_back(code2, sub_name, sub_name);
                        }
                    for(auto&& sub: sub_list) rep_list.push_back(std::move(sub));
                    if(rep_list.size() > 1)
                    {
                        out << operation;
                        for(auto& rep: rep_list) out << u8(std::get<0>(rep));
                        out << '\n';
                    }
                }
        }

    out << R"(
// Some symbols that act as last resort…
= Ⅱ║∥‖ǁ𝄁
= Ⅲ⫴⦀⫼𝍫ꔖ
= -‐‑–—−－‒―➖─━一╴╶╸╺╼╾╼╾
= ┄┅⋯┈┉╌╍
= ╎╏¦
= │┃|╿╽
= ═＝꓿
= ~⁓～
= <く𐌂ᐸᑉ
= ┌┍┎┏╭╒╓╔гᴦⲅ
= ┐┑┒┓╮╕╖╗
= └┕┖┗╰╘╙╚˪լ
= ┘┙┚┛╯╛╜╝
= ┬┭┮┯┰┱┲┳╤╥╦⊤
= ┴┵┶┷┸┹┺┻╧╨╩
= ├┝┞┟┠┡┢┣߅╞╟╠
= ┤┥┦┧┨┩┪┫╡╢╣
= ┼┽┾┿╀╁╂╃╄╅╆╇╈╉╊╋╪╫╬
= ▉⬛██▉▇
→ ガカ
→ グク
→ ギキ
→ ゲケ
→ ゴコ
→ パバハ
→ ピビヒ
→ ペベヘ
→ ポボホ
→ プブフ
→ ピビ
→ ペベ
→ ポボ
→ プブ
→ ザサ
→ ジシ
→ ズス
→ ゼセ
→ ゾソ
→ ダタ
→ ヂチ
→ ヅツ
→ デテ
→ ドト
→ がか
→ ぐく
→ ぎき
→ げけ
→ ごこ
→ ぱばは
→ ぴびひ
→ ぺべへ
→ ぽぼほ
→ ぷぶふ
→ ぱば
→ ぴび
→ ぺべ
→ ぽぼ
→ ぷぶ
→ ざさ
→ じし
→ ずす
→ ぜせ
→ ぞそ
→ だた
→ ぢち
→ づつ
→ でて
→ どと
)";
}

std::vector<std::pair<char32_t/*goal*/, char32_t/*recipe*/>> ParseSimilarities(std::istream& f) // Same as old "similarities.inc"
{
    std::vector<std::pair<char32_t, char32_t>> results;

    for(std::string line; std::getline(f, line), f; )
        if(!line.empty())
        {
            if(line.back() == '\r') { line.erase(line.size()-1); }
            if(auto codes = FromUTF8(line); !codes.empty())
            {
                std::size_t a=2, b=codes.size();
                switch(codes[0])
                {
                    case U'→':
                        // Generate symbol on left from symbol on right
                        for(std::size_t n=a, m=n+1; m<b; ++m)
                            results.emplace_back(codes[n], codes[m]);
                        break;
                    case U'←':
                        // Generate symbol on right from symbol on left
                        for(std::size_t n=b; n-- >= a; )
                            for(std::size_t m=n; --m >= a; )
                                results.emplace_back(codes[n], codes[m]);
                        break;
                    case U'◆':
                    {
                        // Generate any symbol from any other,
                        // with preference for close-by one
                        std::map<std::size_t, std::vector<std::pair<std::size_t,std::size_t>>> score;
                        for(std::size_t n=a; n<b; ++n)
                            for(std::size_t m=a; m<b; ++m)
                                if(m != n)
                                    score[std::abs(int(n-m))].emplace_back(n, m);
                        for(auto& [dummy,pairs]: score)
                            for(auto pair: pairs)
                                results.emplace_back(codes[pair.first], codes[pair.second]);
                        break;
                    }
                    case U'=':
                        // Generate any symbol from any other symbol
                        for(std::size_t n=a; n<b; ++n)
                            for(std::size_t m=a; m<b; ++m)
                                if(m != n)
                                    results.emplace_back(codes[n], codes[m]);
                        break;
                }
            }
        }

    // Improve the list using iconv's //TRANSLIT option (pregenerated using make-alias.php)
    auto [location, status] = FindShareFile(std::filesystem::path("fonts") / "alias.txt");
    if(std::filesystem::exists(status))
    {
        std::ifstream f2(location);
        auto delim = " \t"sv, hex = "0123456789ABCDEFabcdef"sv;
        for(std::string line; std::getline(f2, line), f2; )
            if(!line.empty() && line[0] != '#')
            {
                std::size_t space    = line.find_first_of(delim);
                if(space == line.npos) continue;
                std::size_t notspace = line.find_first_not_of(delim, space);
                if(notspace == line.npos) continue;
                std::size_t notdigit = line.find_first_not_of(hex, notspace);
                if(notdigit == line.npos) notdigit = line.size();

                results.emplace_back(std::stoi(std::string(line, 0, space), nullptr, 16),
                                     std::stoi(std::string(line, notspace, notdigit-notspace), nullptr, 16));
            }
    }

    // Delete duplicates
    std::unordered_set<std::uint_fast64_t> seen;
    results.erase(std::remove_if(results.begin(), results.end(), [&](auto& pair)
    {
        uint_fast64_t key = pair.first; key = (key << 32) | pair.second;
        if(seen.find(key) == seen.end())
        {
            seen.insert(key);
            return false;
        }
        return true;
    }), results.end());

    std::sort(results.begin(), results.end());
    return results;
}

std::vector<std::pair<char32_t/*goal*/, char32_t/*recipe*/>> ParseSimilarities() // Same as old "similarities.inc"
{
    auto [path, status] = FindCacheFile("similarities.dat", true);
    auto [unipath, unistatus] = UnicodeDataFileName();

    if(!std::filesystem::exists(status)
    || !std::filesystem::file_size(path)
    || (std::filesystem::exists(unipath)
     && std::filesystem::last_write_time(unipath) > std::filesystem::last_write_time(path)))
    {
        std::ofstream file(path);
        MakeSimilarities(file, unipath);
    }

    std::ifstream f(path);
    return ParseSimilarities(f);
}
