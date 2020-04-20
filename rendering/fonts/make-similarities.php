<?php
$names = Array();
$codes = Array();

$fp = fopen('UnicodeData.txt', 'r');
while($s = fgets($fp,4096))
  if(preg_match('/^([0-9A-F]+);([^;]+)/', $s, $mat))
  {
    $name = $mat[2];
    // Rename math symbols with "CAPITAL L" into with "LATIN CAPITAL LETTER L"
    // to get better matches.
    if(preg_match('/(MATHEMATICAL.*) (CAPITAL|SMALL) ([A-Z])$/', $name, $m))
    {
      $name = "{$m[1]} LATIN {$m[2]} LETTER {$m[3]}";
    }
    
    $codes[$name] = hexdec($mat[1]);
    $names[hexdec($mat[1])] = $name;
  }

$lines = Array();

function ToUTF8($code)
{
  return iconv('ucs4', 'utf8', pack('N',$code));
}

// Create special matching recipe for ASCII from FULLWIDTH
// Do this BEFORE the MATHEMATICAL section
// to avoid your ASCII letters looking completely silly.
print "// If this is fullwidth font, create ASCII characters from FULLWIDTH characters\n";
foreach($codes as $name=>$code)
{
  if(preg_match('/^(FULLWIDTH|HALFWIDTH) (.*)/', $name, $mat) && isset($codes[$mat[2]]))
    printf("→ %s%s\n", ToUTF8($codes[$mat[2]]), ToUTF8($codes[$name]));
}
print "// Insert some manually crafted rules between pre-composed encircled or stylished letters\n";
print "// Do this before the MATH section may alias doublestruck R (𝕉) with regular R\n";
print "// when the font may in fact have doublestruck R (ℝ) in the letterlike section.\n";
print '= ℂ𝔺
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
';
print "// Insert equality rules between symbols that are visually completely indiscernible\n";
print "// First, ASCII-like characters\n";
$equal_symbols = '!ǃ
#ⵌꖛ⌗⋕
÷➗
+➕ᚐ
-−–➖
.ꓸ
,ꓹ
ꓽ::׃꞉⁚ː∶։܃𝄈
;;;ꓼ
=ꘌ⚌゠᐀꓿
/⟋╱⁄𝈺
\⟍╲𝈻⧹⧵
2շ
3З𝟹ვ౩𝈆
ɜзᴈ
ƷӠ
ʒӡ
4Ꮞ
8𐌚
➊❶⓵➀①
➋❷⓶➁②
➌❸⓷➂③
➍❹⓸➃④
➎❺⓹➄⑤
➏❻⓺➅⑥
➐❼⓻➆⑦
➑❽⓼➇⑧
➒❾⓽➈⑨
➓❿⓾➉⑩
AΑАᎪᗅᗋ𐌀ꓐꓮ
ĂӐᾸ
ĀᾹ
ÄӒ
ÅÅ
ÆӔ
ʙвⲃ
BΒВᏴⲂꕗ𐌁
ƂБ
ϭб
CϹСᏟⲤⅭꓚ
ϽƆↃꓛ
DᎠ𐌃ᗞⅮⅮꓓ
ꓷᗡ⫏
EΕЕᎬⴹꗋ⋿ꓰ
ꓱ∃Ǝⴺ
ÈЀ
ËЁ
ĔӖ
ƐԐ
FϜ𝈓ߓᖴ𐌅ꓝ
GᏀႺꓖ
HΗНᎻⲎᕼꖾꓧ
ʜнⲏ
IΙІӀᏆⲒⅠꓲ
ΪÏЇ
JЈᒍلﻝᎫꓙ
KΚКⲔᏦK𐌊ꓗ
ḰЌ
κᴋкⲕ
LᏞᒪ𝈪Ⅼ˪ԼⅬԼⅬԼլꓡ
MΜМᎷⲘϺ𐌑Ⅿꓟ
ᴍмⲙ
NΝⲚꓠ
ͶИ
ɴⲛ
ͷи
OΟОⲞ◯○⃝❍🌕߀ⵔՕ⚪⭕౦೦ꓳ
ÖӦ
ϴθѲӨƟᎾⲐ
ΦФⲪ
PΡРᏢⲢ𐌓ᑭ𐌛ꓑ
ΠПⲠ
QԚꝖႳႭⵕℚ
RᎡ𝈖ᖇᏒꓣ
Яᖆ
SЅᏚՏႽꚂऽ𐒖ꕶꓢ
ΣƩ∑⅀Ʃⵉ
TΤТᎢⲦ⊤⟙ꔋ𝍮🝨⏉ߠꓔ
U⋃ᑌ∪ՍՍꓴ
VᏙᐯⴸ⋁𝈍ⅤⅤꓦ
WԜᎳꓪ
XΧХⲬ╳ⵝ𐌢Ⅹ𐌗Ⅹꓫ
YΥҮⲨꓬ
ΫŸ
ZΖᏃⲌꓜ
aа
äӓ
ăӑ
æӕ
əә
ЬᏏ
cϲсⲥⅽ
ͻɔᴐↄ
dⅾ
eе
ĕӗ
ϵє
ɛԑ
gց
гᴦⲅ
iіⅰ
ïї
jϳј
lⅼ
ιɩ
mⅿ
ʌᴧ
oοоסᴏⲟօ૦௦ഠ๐໐໐
òὸ
óό
öӧ
ɵөⲑ
фⲫ
pρрⲣ
πпᴨⲡ
яᴙ
qԛ
sѕ
uս
vᴠݍⅴ
xⅹ
yу
~῀
··•∙⋅・
ᴛтⲧ
૰。࿁
ᐃ△🜂∆ΔᐃⵠΔꕔ
ᐊᐊ◁⊲
ᐁ▽🜄⛛∇ᐁ𝈔
ᐅ▷⊳▻
ᐱΛ𐤂⋀ⴷ𐌡Ʌ
ᑎႶ⋂Ո∩𝉅ꓵ
⨆∐ⵡ𝈈
∏⨅ПΠ⊓
⊏ⵎ𝈸
コ⊐ߏ𝈹
⎕□☐⬜◻▢⃞❑❒❐❏⧠⃢⌷ロ
⛝⌧🝱
 　         ';
foreach(explode("\n", $equal_symbols) as $line)
  print "= $line\n";

print "// Create similarity rules between modified stylished symbols\n";
$words = Array('BLACK',
               'HEAVY',
               'FULLWIDTH',
               'MATHEMATICAL BOLD',                  // 1D400,1D41A
               'MATHEMATICAL SANS-SERIF BOLD',       // 1D5D4,1D5EE
               'MATHEMATICAL SANS-SERIF BOLD ITALIC',// 1D63C,1D656
               'MATHEMATICAL BOLD ITALIC',           // 1D468,1D482
               'MATHEMATICAL ITALIC',                // 1D434,1D44E
               'MATHEMATICAL SANS-SERIF ITALIC',     // 1D608,1D622
               '',                                   // no modifier.
               'MATHEMATICAL SANS-SERIF',            // 1D5A0,1D5BA
               'MATHEMATICAL SCRIPT',                // 1D49C,1D4B6
               'MATHEMATICAL BOLD SCRIPT',           // 1D4D0,1D4EA
               'MATHEMATICAL FRAKTUR',               // 1D504,1D51E
               'MATHEMATICAL BOLD FRAKTUR',          // 1D56C,1D586
               'MATHEMATICAL DOUBLE-STRUCK',         // 1D538,1D552
               'MATHEMATICAL MONOSPACE',             // 1D670,1D68A
               'MATHEMATICAL',
               'WHITE',
               'LIGHT',
               'HALFWIDTH',
               'SMALL',
               'PARENTHESIZED',
               'CIRCLED',
               'TAG');
$symbols = Array();
foreach($codes as $name=>$code)
{
  $symbols[$name][''] = $code;
  if(preg_match('/^('.join('|',$words).') (.*)/', $name, $mat))
    $symbols[$mat[2]][$mat[1]] = $code;
}
foreach($symbols as $basename => $group)
  if(count($group) > 1)
  {
    print "◆ ";
    foreach($words as $w) if(isset($group[$w])) print ToUTF8($group[$w]);
    print "\n";
  }

// Convert the equal-symbols list into a searchable one
$equal_with = Array();
foreach(explode("\n", $equal_symbols) as $line)
{
  $eq = unpack('N*',iconv('utf8','ucs4',$line));
  foreach($eq as $code)
    foreach($eq as $code2)
      if($code != $code2)
        $equal_with[$code][$code2] = $code2;
}

print "// Then go through all symbols that are “WITH” something.\n";
print "// As a general rule, try to compose things that have more “WITHs”\n";
print "// from things that have less “WITHs”.\n";
$with_lists = Array();
foreach($codes as $name=>$code)
  if(preg_match('/(.*) WITH (.*)/', $name, $mat))
  {
    $base = $mat[1];
    $full = $mat[2];
    $attrs = explode(' AND ', $full);
    $len = count($attrs);
    $with_lists[" WITH $full"][""] = 0;
    for($n=$len-1; $n>0; --$n)
    {
      $pick = Array();
      $do = function($index,$start)use(&$attrs,&$pick,$n,$len,$code,$name,$base,&$do,&$full,&$with_lists)
      {
        for($a=$start; $a<$len; ++$a)
        {
          $pick[$index] = $attrs[$a];
          if($index+1 == $n)
          {
            $partial = join($pick, ' AND ');
            $with_lists[" WITH $full"][" WITH $partial"] = count($pick);
            #print "try make $partial from $full for $name for $base\n";
          }
          else
          {
            $do($index+1, $a+1);
          }
        }
      };
      $do(0, 0);
    }
  }

foreach(Array("→ ", "← ") as $operation)
  foreach($with_lists as $full_with => $partial_list)
  {
    arsort($partial_list);
    // Find all symbols that have this "full with" list.
    foreach($codes as $name => $code)
      if(preg_match("/(.*)$full_with\$/", $name, $mat))
      {
        $rep_list = Array();
        $rep_list[] = Array($code, $name, $mat[1]);
        if(isset($equal_with[$code]))
        {
          foreach($equal_with[$code] as $code2)
          {
            $name2 = $names[$code2];
            preg_match("/(.*) WITH.*\$/", $name2, $mat2);
            $rep_list[] = Array($code2, $name2, @$mat2[1]);
          }
        }
        $sub_list = Array();
        foreach($partial_list as $partial_with=>$dummy)
        {
          foreach($rep_list as $rep)
          {
            $sub_name = "{$rep[2]}$partial_with";
            #print "can we find $sub_name?\n";
            if(isset($codes[$sub_name]))
            {
              #if(count($rep_list) > 1) print "guu\n";
              $sub_list[] = Array($codes[$sub_name], $sub_name, $rep[2]);
            }
          }
        }
        foreach($sub_list as $sub) $rep_list[] = $sub;
        if(count($rep_list) > 1)
        {
          print $operation;
          foreach($rep_list as $rep)
            print ToUTF8($rep[0]);
          print "\n";
        }
      }
  }

print '// Some symbols that act as last resort…
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
';

exit;
print "<?php\n";
?>
  $identical = Array
  (
    // Autogenerated list 1, of fullwidths
<?php
    foreach($lines as $s)
      if(preg_match('@^Array.0xFF.. .*, 0x[3-7]./@', $s))
        print $s;
?>
    // End autogenerated list 1
    // Math: 
    
    Array(33 /* ! */, 451 /* ǃ */),
    Array(35 /* # */,                               0x2d4c,0xa59b,0x2317,0x22d5),
    Array(43 /* + */, 0x2795 /* ➕ */, 0x1690 /* ᚐ */),
    Array(45 /* - */, 0x2212 /* − */, 0x2013 /* – */, 0x2796 /* ➖ */),
    Array(47 /* / */,                                               0x27cb,0x338,0x2571,0x2044,0x1d23a),
    Array(92 /* \ */,                                               0x20e5,0x27cd,0x2572,0x1d23b,0x29f9,0x29f5),
    Array(51 /* 3 */,                0x417 /* З */, 0x1d7f9,0x10d5,0xc69,0x1d206),
    Array(52 /* 4 */,                               0x13CE /* Ꮞ */),
    Array(56 /* 8 */,                                               0x1031a),
    Array(58 /* : */,                               0x5c3,0xa789,0x205a,0x2d0,0x2236,0x589,0x703,0x1d108),
    Array(59 /* ; */, 0x37E /* ; */, ),
    Array(61 /* = */,                                               0xa60c,0x268c,0x30a0,0x1400),
    Array(65 /* A */, 0x391 /* Α */, 0x410 /* А */, 0x13AA /* Ꭺ */, 0x15c5, 0x15cb,0x10300),
    Array(66 /* B */, 0x392 /* Β */, 0x412 /* В */, 0x13F4 /* Ᏼ */, 0x2C82 /* Ⲃ */, 0xa557,0x10301),
    Array(67 /* C */, 0x3F9 /* Ϲ */, 0x421 /* С */, 0x13DF /* Ꮯ */, 0x2CA4 /* Ⲥ */),
    Array(68 /* D */,                               0x13A0 /* Ꭰ */, 0x10303,0x15de,0x216e),
    Array(69 /* E */, 0x395 /* Ε */, 0x415 /* Е */, 0x13AC /* Ꭼ */, 0x2d39,0xa5cb,0x22ff),
    Array(70 /* F */, 0x3DC /* Ϝ */,                                0x1d213,0x7d3,0x15b4,0x10305,),
    Array(71 /* G */,                               0x13C0 /* Ꮐ */, 0x10ba,),
    Array(72 /* H */, 0x397 /* Η */, 0x41D /* Н */, 0x13BB /* Ꮋ */, 0x2C8E /* Ⲏ */, 0x157c,0xa5be),
    Array(73 /* I */, 0x399 /* Ι */, 0x406 /* І */, 0x4C0 /* Ӏ */, 0x13C6 /* Ꮖ */, 0x2C92 /* Ⲓ */),
    Array(74 /* J */,                0x408 /* Ј */,                 0x148d,0x644,0xfedd,0x13ab),
    Array(75 /* K */, 0x39A /* Κ */, 0x41A /* К */, 0x2C94 /* Ⲕ */, 0x13E6 /* Ꮶ */, 0x212A /* K */, 0x1030a,),
    Array(76 /* L */,                               0x13DE /* Ꮮ */, 0x14aa,0x1d22a,0x216c,0x2ea,0x53c,),
    Array(77 /* M */, 0x39C /* Μ */, 0x41C /* М */, 0x13B7 /* Ꮇ */, 0x2C98 /* Ⲙ */, 0x3fa,0x10311,),
    Array(78 /* N */, 0x39D /* Ν */,                                0x2C9A /* Ⲛ */,),
    Array(79 /* O */, 0x39F /* Ο */, 0x41E /* О */, 0x2C9E /* Ⲟ */,                 0x25ef,0x25cb,0x20dd,0x274d,0x1f315,0x2d54,0x555,0x26aa,0x2b55),
    Array(80 /* P */, 0x3A1 /* Ρ */, 0x420 /* Р */, 0x13E2 /* Ꮲ */, 0x2CA2 /* Ⲣ */, 0x10313,0x146d,0x1031b),
    Array(81 /* Q */,                                                               0x10ad,0x51a,0xa756,0x10b3,0x2d55,0x211a),
    Array(82 /* R */,                               0x13A1 /* Ꭱ */, 0x1d216, 0x1587, 0x13d2),
    Array(83 /* S */,                0x405 /* Ѕ */, 0x13DA /* Ꮪ */,                 0x54f,0x10bd,0xa682,0x93d,0x10496,0xa576),
    Array(84 /* T */, 0x3A4 /* Τ */, 0x422 /* Т */, 0x13A2 /* Ꭲ */, 0x2CA6 /* Ⲧ */, 0x22a4,0x27d9,0xa50b,0x1d36e,0x1f768,0x23c9,0x7e0),
    Array(85 /* U */,                                                               0x22c3,0x144c,0x222a,0x54d),
    Array(86 /* V */,                               0x13D9 /* Ꮩ */,                 0x142f,0x2d38,0x22c1,0x1d20d,0x2164,),
    Array(                           0x51C /* Ԝ */,                 0x2CB0 /* Ⲱ */, 0x460 /* Ѡ */),
    Array(87 /* W */,                0x51C /* Ԝ */, 0x13B3 /* Ꮃ */),
    Array(88 /* X */, 0x3A7 /* Χ */, 0x425 /* Х */,                 0x2CAC /* Ⲭ */, 0x2573,0x2d5d,0x10322,0x2169,0x10317,),
    Array(89 /* Y */, 0x3A5 /* Υ */, 0x4AE /* Ү */,                 0x2CA8 /* Ⲩ */, ),
    Array(90 /* Z */, 0x396 /* Ζ */,                0x13C3 /* Ꮓ */, 0x2C8C /* Ⲍ */, ),
    Array(97  /* a */,                0x430 /* а */),
    Array(99  /* c */, 0x3F2 /* ϲ */, 0x441 /* с */,                0x2CA5 /* ⲥ */),
    Array(101 /* e */,                0x435 /* е */),
    Array(105 /* i */,                0x456 /* і */),
    Array(106 /* j */, 0x3F3 /* ϳ */, 0x458 /* ј */),
    Array(111 /* o */, 0x3BF /* ο */, 0x43E /* о */, 0x5E1 /* ס */, 0x1D0F /* ᴏ */, 0x2C9F /* ⲟ */),
    Array(112 /* p */, 0x3C1 /* ρ */, 0x440 /* р */,                                0x2CA3 /* ⲣ */),
    Array(113 /* q */,                               0x51B /* ԛ */),
    Array(115 /* s */,                0x455 /* ѕ */),
    Array(118 /* v */,                                              0x1D20 /* ᴠ */),
    Array(             0x3C9 /* ω */, 0x51D /* ԝ */,                0x2CB1 /* ⲱ */, 0x2375 /* ⍵ */, 0x461 /* ѡ */),
    Array(119 /* w */, 0x3C9 /* ω */,                0x1D21 /* ᴡ */),
    Array(120 /* x */,                0x445 /* х */,                0x2CAD /* ⲭ */),
    Array(121 /* y */,                0x443 /* у */,                0x2CA9 /* ⲩ */),
    Array(122 /* z */,                                              0x1D22 /* ᴢ */, 0x2C8D /* ⲍ */),
    Array(126 /* ~ */,                                              0x1FC0 /* ῀ */),
    Array(               0x26A /* ɪ */, 0x2C93 /* ⲓ */, 305 /* ı */),
    Array(0x3BA /* κ */, 0x1D0B /* ᴋ */, 0x432 /* к */, 0x2C95 /* ⲕ */),
    Array(               0x299  /* ʙ */, 0x432 /* в */, 0x2C83 /* ⲃ */),
    Array(               0x29C  /* ʜ */, 0x43D /* н */, 0x2C8F /* ⲏ */),
    Array(               0x1D0D /* ᴍ */, 0x43c /* м */, 0x2C99 /* ⲙ */),
    Array(               0x274  /* ɴ */,                0x2C9B /* ⲛ */),
    Array(               0x1D1B /* ᴛ */, 0x442 /* т */, 0x2CA7 /* ⲧ */),
    Array(               239    /* ï */, 0x457 /* ї */),
    Array(               0x1E30 /* Ḱ */, 0x40C /* Ќ */),
    Array(               200    /* È */, 0x400 /* Ѐ */),
    Array(               203    /* Ë */, 0x401 /* Ё */),
    Array(0x3AA /* Ϊ */, 207    /* Ï */, 0x407 /* Ї */),
    Array(0x3AB /* Ϋ */, 376    /* Ÿ */),
    Array(0x3A8 /* Ψ */, 0x2CAE /* Ⲯ */),
    Array(0x3C8 /* ψ */, 0x2CAF /* ⲯ */),
    Array(),
    Array(0x393 /* Γ */,                 0x413 /* Г */, 0x13B1 /* Ꮁ */, 0x2C84 /* Ⲅ */),
    Array(0x3CC /* ό */, 243    /* ó */),
    Array(0x3F4 /* ϴ */, 0x3B8  /* θ */, 0x472 /* Ѳ */, 0x4E8 /* Ө */, 415 /* Ɵ */, 0x13BE /* Ꮎ */, 0x2C90 /* Ⲑ */),
    Array(               258    /* Ă */, 0x4D0 /* Ӑ */, 0x1fb8 /* Ᾰ */),
    Array(               256    /* Ā */,                0x1fb9 /* Ᾱ */),
    Array(               259    /* ă */, 0x4D1 /* ӑ */),
    Array(               196    /* Ä */, 0x4D2 /* Ӓ */),
    Array(               228    /* ä */, 0x4D3 /* ӓ */),
    Array(               198    /* Æ */, 0x4D4 /* Ӕ */),
    Array(               230    /* æ */, 0x4D5 /* ӕ */),
    Array(               276    /* Ĕ */, 0x4D6 /* Ӗ */),
    Array(               277    /* ĕ */, 0x4D7 /* ӗ */),
    Array(               214    /* Ö */, 0x4E6 /* Ӧ */),
    Array(               246    /* ö */, 0x4E7 /* ӧ */),
    Array(0x3A6 /* Φ */,                 0x424 /* Ф */, 0x2CAA /* Ⲫ */),
    Array(                               0x444 /* ф */, 0x2CAB /* ⲫ */),
    Array(0x387 /* · */, 183    /* · */),
    Array(0x3F5 /* ϵ */,                 0x454 /* є */),
    Array(0x37B /* ͻ */, 0x254 /* ɔ */, 0x1D10 /* ᴐ */),
    Array(               0x259 /* ə */,  0x4D9 /* ә */),
    Array(               0x25c /* ɜ */,  0x437 /* з */, 0x1D08 /* ᴈ */),
    Array(               0x275 /* ɵ */,  0x4E9 /* ө */, 0x2C91 /* ⲑ */),
    Array(               339   /* œ */,  0x276 /* ɶ */),
    Array(               0x292 /* ʒ */,  0x4E1 /* ӡ */),
    Array(               386 /* Ƃ */,  0x411 /* Б */),
    Array(0x3FD /* Ͻ */, 390 /* Ɔ */),
    Array(0x3A3 /* Σ */, 425 /* Ʃ */,                   0x2211/*∑*/, 0x2140,0x1a9,0x2d49),
    Array(0x3C5 /* υ */, 651 /* ʋ */),
    Array(               439 /* Ʒ */, 0x4E0 /* Ӡ */),
Array(0x20, 0x3000, 0xA0,0x2000,0x2001,0x2002,0x2003,0x2004,0x2005,0x2006,0x2009), // Make SPACE from IDEOGRAPHIC SPACE (also several other spaces)
    // Autogenerated list excluding fullwidths we did earlier
<?php
    foreach($lines as $s)
      if(!preg_match('@^Array.0xFF.. .*, 0x[3-7]./@', $s))
        print $s;
?>
    // End autogenerated list
    Array(0x3ED /* ϭ */, 0x431 /* б */),
    Array(0x3A0 /* Π */,                 0x41F /* П */, 0x2CA0 /* Ⲡ */),
    Array(0x3C0 /* π */, 0x43F /* п */, 0x1D28 /* ᴨ */, 0x2CA1 /* ⲡ */),
    Array(0x376 /* Ͷ */, 0x418 /* И */),
    Array(0x377 /* ͷ */, 0x438 /* и */),
    Array(0x3B9 /* ι */, 617 /* ɩ */),
    Array(242 /* ò */, 0x1F78 /* ὸ */),
    Array(243 /* ó */, 0x1F79 /* ό */),
    Array(                           0x44F /* я */, 0x1D19 /* ᴙ */),
    Array(                           0x433 /* г */, 0x1D26 /* ᴦ */, 0x2C85 /* ⲅ */),
    Array(               652 /* ʌ */, 0x1D27 /* ᴧ */),
    Array(               603 /* ɛ */, 0x511 /* ԑ */),
    Array(               400 /* Ɛ */, 0x510 /* Ԑ */),
    Array(               0x42C /* Ь */, 0x13CF /* Ꮟ */),
    Array(               197 /* Å */, 0x212B /* Å */),
    Array(0x278A /*➊*/, 0x2776/*❶*/, 0x24F5/*⓵*/, 0x2780/*➀*/, 0x2460/*①*/, 48/*0*/),
    Array(0x278B /*➋*/, 0x2777/*❷*/, 0x24F6/*⓶*/, 0x2781/*➁*/, 0x2461/*②*/, 49/*1*/),
    Array(0x278C /*➌*/, 0x2778/*❸*/, 0x24F7/*⓷*/, 0x2782/*➂*/, 0x2462/*③*/, 50/*2*/),
    Array(0x278D /*➍*/, 0x2779/*❹*/, 0x24F8/*⓸*/, 0x2783/*➃*/, 0x2463/*④*/, 51/*3*/),
    Array(0x278E /*➎*/, 0x277A/*❺*/, 0x24F9/*⓹*/, 0x2784/*➄*/, 0x2464/*⑤*/, 52/*4*/),
    Array(0x278F /*➏*/, 0x277B/*❻*/, 0x24FA/*⓺*/, 0x2785/*➅*/, 0x2465/*⑥*/, 53/*5*/),
    Array(0x2790 /*➐*/, 0x277C/*❼*/, 0x24FB/*⓻*/, 0x2786/*➆*/, 0x2466/*⑦*/, 54/*6*/),
    Array(0x2791 /*➑*/, 0x277D/*❽*/, 0x24FC/*⓼*/, 0x2787/*➇*/, 0x2467/*⑧*/, 55/*7*/),
    Array(0x2792 /*➒*/, 0x277E/*❾*/, 0x24FD/*⓽*/, 0x2788/*➈*/, 0x2468/*⑨*/, 56/*8*/),
    Array(0x2793 /*➓*/, 0x277F/*❿*/, 0x24FE/*⓾*/, 0x2789/*➉*/, 0x2469/*⑩*/, 57/*9*/),   
    Array(247 /* ÷ */, 0x2797 /* ➗ */),
    Array(0x42F /* Я */, 0x1586),
    Array(0x394 /* Δ */, 0xa554 /* ꕔ */),
    Array(0x3c,0x304f,0x10302,0x1438,0x1449),
  );
