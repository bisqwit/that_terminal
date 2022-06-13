<?php

require 'read_font.php';
require 'font_gen2.php';

$authentic_only = ($argc == 1) ? 0 : (int)$argv[1];

/*
 grep ^ENCODING data/6x9.bdf|sed 's/ENCODING //'|tr '\012' ,

 For unifont, 8 wide:
 grep -B4 DWIDTH.8 data/unifont-csur.bdf|grep ^ENCODING|sed 's/ENCODING //'|tr '\012' , > tmptmp1
 For unifont, 16 wide:
 grep -B4 DWIDTH.16 data/unifont-csur.bdf|grep ^ENCODING|sed 's/ENCODING //'|tr '\012' , > tmptmp2

 For misaki, 8 wide:
 grep -B4 DWIDTH.4 data/misakig2.bdf|grep ^ENCODING|sed 's/ENCODING //'|tr '\012' , > tmptmp1
 For misaki, 8 wide:
 grep -B4 DWIDTH.8 data/misakig2.bdf|grep ^ENCODING|sed 's/ENCODING //'|tr '\012' , > tmptmp2
*/

function ReadBDFset($name,$x,$y, $keep_index = false)
{
  $tmp = array_keys(Read_BDF("data/$name",$x,$y));
  //
  if($keep_index) return array_flip($tmp);
  else            return array_combine($tmp, $tmp);
}
$chars_4x5       = ReadBDFset('4x5.bdf',4,5);
$chars_4x6       = ReadBDFset('4x6.bdf',4,6);
$chars_5x7       = ReadBDFset('5x7.bdf',5,7);
$chars_5x8       = ReadBDFset('5x8.bdf',5,8);
$chars_6x9       = ReadBDFset('6x9.bdf',6,9);
$chars_6x10      = ReadBDFset('6x10.bdf',6,10);
$chars_6x12      = ReadBDFset('6x12.bdf',6,12);
$chars_6x13      = ReadBDFset('6x13.bdf',6,13);
$chars_7x13      = ReadBDFset('7x13.bdf',7,13);
$chars_7x14      = ReadBDFset('7x14.bdf',7,14);
$chars_8x13      = ReadBDFset('8x13.bdf',8,13);
$chars_9x15      = ReadBDFset('9x15.bdf',9,15);
$chars_9x18      = ReadBDFset('9x18.bdf',9,18);
$chars_10x20     = ReadBDFset('10x20.bdf',10,20);
#$chars_10x10ja   = ReadBDFset('f10.bdf',10,10);
$chars_12x12ja   = ReadBDFset('f12.bdf',12,12);
$chars_14x14ja   = ReadBDFset('f14.bdf',14,14);
$chars_12x13ja   = ReadBDFset('12x13ja.bdf',12,13);
$chars_18x18ja   = ReadBDFset('18x18ja.bdf',18,18);
$chars_18x18ko   = ReadBDFset('18x18ko.bdf',18,18);
$chars_10x20cn   = ReadBDFset('gb24st.bdf', 10,20);
$chars_9x15cn    = ReadBDFset('gb16st.bdf',  9,15);
$chars_16x15cn   = ReadBDFset('gb16st.bdf', 16,15);
$chars_24x20cn   = ReadBDFset('gb24st.bdf', 24,20);
$chars_24x24cn   = ReadBDFset('cmex24m.bdf',24,24);
$chars_unicsur8  = ReadBDFset('unifont-csur.bdf', 8,16);
$chars_unicsur16 = ReadBDFset('unifont-csur.bdf',16,16);
$chars_misaki4   = ReadBDFset('misakig2.bdf', 4,8);
$chars_misaki8   = ReadBDFset('misakig2.bdf', 8,8);
$chars_ib8x8     = ReadBDFset('ib8x8u.bdf',  8,8);
$chars_ie8x14    = ReadBDFset('ie8x14u.bdf', 8,14);
$chars_iv8x16    = ReadBDFset('iv8x16u.bdf', 8,16);
$chars_monak12   = ReadBDFset('monak12.bdf', 12,12); // Note: var-width
$chars_monak14   = ReadBDFset('monak14.bdf', 14,14); // Note: var-width
#$chars_monak16   = ReadBDFset('monak16.bdf', 16,16); // Note: var-width
$chars_mona6x12r = ReadBDFset('mona6x12r.bdf', 6,12);// Note: var-width
$chars_mona7x14r = ReadBDFset('mona7x14r.bdf', 7,14);// Note: var-width
$chars_vga8x19   = ReadBDFset('vga8x19.bdf', 8,19);

$chars_cp437     = Array(); // CP437 with extra U+FFFD
for($n=0; $n<256; ++$n) $chars_cp437[BDFtranslateToUnicode($n,'IBM','CP437')] = $n;
for($n=0; $n<256; ++$n) $chars_cp437[65533] = 256;

$chars_cp850     = Array(); // CP850 with extra U+FFFD
for($n=0; $n<256; ++$n) $chars_cp850[BDFtranslateToUnicode($n,'IBM','CP850')] = $n;
for($n=0; $n<256; ++$n) $chars_cp850[65533] = 256;

function TransLow($index, $size)
{
  $translation = Array
  (
    0x0000,0x263A,0x263B,0x2665,
    0x2666,0x2663,0x2660,0x2022,
    0x25D8,0x25CB,0x25D9,0x2642,
    0x2640,0x266A,0x266B,0x263C,
    0x25BA,0x25C4,0x2195,0x203C,
    0x00B6,0x00A7,0x25AC,0x21A8,
    0x2191,0x2193,0x2192,0x2190,
    0x221F,0x2194,0x25B2,0x25BC,
  );
  foreach(Array(
    0x2302, // 7F house
    // 81..8F follows CP/M plus character set (Amstrad CPC e.g.) but shifted 91..9F
    // ⎺ ╵ ╶ └ ╷ │ ┌ ├ ╴ ┘ ─ ┴ ┐ ┤ ┬ ┼
    0x23BA,0x2575,0x2576,0x2514,0x2577,0x2502,0x250C,0x251C,
    0x2574,0x2518,0x2500,0x2534,0x2510,0x2524,0x252C,0x253C,
    // 95..9F follows CP/M plus character set (Amstrad CPC e.g.) but shifted 85..8F
    // ⎽ ╧ ╟ ╚ ╤ ║ ╔ ╠ ╢ ╝ ═ ╩ ╗ ╣ ╦ ╬
    // Note that 91,92,94,98 are truncated *double* lines, which are not supported by unicode
    0x25BD,0x2567,0x255F,0x255A,0x2564,0x2551,0x2554,0x2560,
    0x2562,0x255D,0x2550,0x2569,0x2557,0x2563,0x2566,0x256C
  ) as $k=>$v) $translation[$k+127] = $v;
  switch($size)
  {
    case '4x5':
      $translation = Array
      (
        0x25A0, // 00 ∎ little black square
        0x230A, // 01 ι or maybe APL symbol ⌊
        0x03BA, // 02 κ
        0x03B8, // 03 θ
        0x2303, // 04 house? ⌃
        0x00AC, // 05 right angle
        0x03B5, // 06 ε
        0x03C0, // 07 π
        0x03BB, // 08 λ
        0x03B3, // 09 γ
        0x03B4, // 0A δ
        0x2308, // 0B maybe ⌠, maybe APL symbol ⌈
        0x00B1, // 0C ±
        0x2363, // 0D + with two dots??
        0x22EE, // 0E ⋮ (vertical ellipsis)
        0x2202, // 0F maybe ð
        0x2282, // 10 ⊂ (bendy left)
        0x2283, // 11 ⊃ (bendy right)
        0x2229, // 12 ∩ (bendy up)
        0x222A, // 13 ∪ (bendy down)
        0x2200, // 14 ∀ (upsidedown A)
        0x2203, // 15 sort of like 3
        0x1E8D, // 16 x with two dots??  ⍣ ẍ
        0x2277, // 17 Zig-zaggy
        0x25C4, // 18 ◄ (black arrow left)
        0x25BA, // 19 ► (black arrow right)
        0x2260, // 1A ≠
        0x22C4, // 1B sort of like diamond
        0x2264, // 1C ≤
        0x2265, // 1D ≥
        0x2261, // 1E ≡ (three horizontal lines)
        0x2228, // 1F ∨ (logical v) 
      );
      $translation[0x7F] = 0x222B; // ∫ integral
      for($n=0x80; $n<0xA0; ++$n) unset($translation[$n]);
      break;
    case '8x10':
    case '8x12':
      // Different symbols, but does not matter, they are *also* mapped correctly
      break;
    case '8x8':
    case '8x14':
    case '8x16':
      // Matches above, but also mapped correctly
      break;
    default:
      break;
  }
  return $translation[$index];
}

function AddChar($utfchar, $n, $size, &$characters, $setname, $authentic_only)
{
  if(!isset($characters[$utfchar]))
  {
    $characters[$utfchar] = Array($setname, $n);
  }
  if($utfchar < 32 || ($utfchar >= 127 && $utfchar < 160))
  {
    $utfchar = TransLow($utfchar, $size);
    if(!isset($characters[$utfchar]))
    {
      $characters[$utfchar] = Array($setname, $n);
    }
  }
  // If we have a narrow font, take sans-serif characters from that 
  if(!$authentic_only)
  {
    if($setname == 'chars_unicsur8'
    || $setname == 'chars_FullGreek_Terminus14_psf_gz'
    || $setname == 'chars_FullCyrSlav_Terminus20x10_psf_gz'
    || $setname == 'chars_FullCyrAsia_Terminus22x11_psf_gz'
    || $setname == 'chars_Uni3_Terminus24x12_psf_gz'
    || $setname == 'chars_CyrKoi_Terminus28x14_psf_gz'
    || $setname == 'chars_FullGreek_Terminus32x16_psf_gz'
    || $setname == 'chars_mona7x14r'
    || $setname == 'chars_mona6x14r'
    || $setname == 'chars_10x20cn'
    || $setname == 'chars_9x15cn'
      )
    {
      # For light fonts that are in presence of bold fonts in same size,
      # use those characters to create mathematical sans-serif symbols.
      if($utfchar >= 0x41 && $utfchar <= 0x5A) { $characters[$utfchar+0x1D5A0-0x41] = Array($setname,$n); $characters[$utfchar+0x1D670-0x41] = Array($setname,$n); }
      if($utfchar >= 0x61 && $utfchar <= 0x7A) { $characters[$utfchar+0x1D5BA-0x61] = Array($setname,$n); $characters[$utfchar+0x1D68A-0x61] = Array($setname,$n); }
      if($utfchar >= 0x30 && $utfchar <= 0x39) { $characters[$utfchar+0x1D7E2-0x30] = Array($setname,$n); $characters[$utfchar+0x1D7E2-0x30] = Array($setname,$n); }
    }
    if($setname == 'chars_misaki8'
    || $setname == 'chars_unicsur16'
      )
    {
      # However, if the font is a double-width font,
      # use the full-width symbols instead of ASCII symbols
      # to make those sans-serif symbols.
      if($utfchar >= 0xFF21 && $utfchar <= 0xFF3A) { $characters[$utfchar+0x1D5A0-0xFF21] = Array($setname,$n); $characters[$utfchar+0x1D670-0x41] = Array($setname,$n); }
      if($utfchar >= 0xFF41 && $utfchar <= 0xFF5A) { $characters[$utfchar+0x1D5BA-0xFF41] = Array($setname,$n); $characters[$utfchar+0x1D68A-0x61] = Array($setname,$n); }
      if($utfchar >= 0xFF10 && $utfchar <= 0xFF19) { $characters[$utfchar+0x1D7E2-0xFF10] = Array($setname,$n); $characters[$utfchar+0x1D7E2-0x30] = Array($setname,$n); }
    }
  }
}
global $identical;
$identical = json_decode(file_get_contents('similarities.inc'));
function AddApproximations(&$characters)
{
  // List of identical-looking characters that are not recognized by iconv as such
  global $identical;
  for(;;)
  {
    $changes = false;
    foreach($identical as $pair)
      if(!isset($characters[$pair[0]])
      &&  isset($characters[$pair[1]]))
      {
        $characters[$pair[0]] = $characters[$pair[1]];
        $changes = true;
      }
    if(!$changes) break;
  }
}

function CreateTranslation($size, $sets, $authentic_only = false)
{
  $pat = str_replace('__', '_',
         str_replace('chars_', '_',
         str_replace('_psf_gz', 'P',
         str_replace('iso', 'I',
         str_replace('_cp', '_',
         str_replace('latin', 'L',
         str_replace('Uni', 'u',
         str_replace('VGA', 'V',
         str_replace('Hebrew', 'Hb',
         str_replace('Arabic', 'Ar',
         str_replace('Unifont', 'U',
         str_replace('FullGreek', 'G',
         str_replace('FullCyrSlav', 'CS',
         str_replace('FullCyrAsia', 'CA',
         str_replace('Terminus', 'T',
         str_replace('TerminusBold', 'TB',
         join('_', $sets)))))))))))))))));
  $cache_fn = "data/.tran-$pat";
  if($authentic_only)
  {
    $cache_fn = "data/.1tran-$pat";
  }
  if(file_exists($cache_fn) && filesize($cache_fn) > 0)
  {
    file_put_contents("php://stderr", "Cache hit: $cache_fn for {$size}\n");
    return unserialize(file_get_contents('compress.zlib://'.$cache_fn));
  }
  file_put_contents("php://stderr", "Cache miss: $cache_fn for {$size}\n");

  #$dfl_setname = 'cp437';
  #foreach($sets as $setname) { $dfl_setname = $setname; break; }

  // Check which characters we have
  $characters = Array();
  foreach($sets as $setname)
  {
    if($setname == 'unk')
    {
      // cp437 hack that manually adds U+FFFD
      $characters[0xFFFD] = Array('unk', 256);
    }
    elseif(preg_match('/^chars_/', $setname))
    {
      global $$setname;
      foreach($$setname as $utfchar=>$index)
      {
        AddChar($utfchar, $index, $size, $characters, $setname, $authentic_only);
      }
    }
    else
    {
      for($n=0; $n<256; ++$n)
      {
        $s = @iconv($setname, 'ucs4', chr($n));
        if($s !== false && iconv_strlen($s, 'ucs4') == 1)
        {
          $cno = unpack('N', $s);
          $utfchar = $cno[1];
          // This $utfchar is found in $setname at index $n
          AddChar($utfchar, $n, $size, $characters, $setname, $authentic_only);
        }
      }
    }
  }
  if(!$authentic_only)
  {
    AddApproximations($characters);
  }

  $sets_to_try = Array();
  foreach($sets as $setname)
  {
    if(preg_match('/^chars_/', $setname))
    {
      foreach(Array(
          'cp850',
          // ASCII, A0-FF, but also
          // 192 2500 250C 2514 251C 252C 253C 2551 2557 255D 2563 2569 2580 2588 2592-2593 

          'cp857',
          // Compared to cp850, adds 11E-11F 131 15F 2502 2510 2518 2524 2534 2550
          
          'cp437',
          // Compared to cp850,

          #'iso-8859-1',
          // ISO-8859-1 does not have anything CP850 doesn't have
          
          'iso-8859-3',
          // Unique: 109-10B 11D-121 125-127 131 135 15D-15F 16D 17C 2D9
          
          'iso-8859-4',
          // Unique: 101 105 10D 111-113 117-119 123 129-12B 12F 137-138 13C 146 14B-14D 157 161 167-16B 173 17E 2D9
          
          'iso-8859-7',
          // Unique: 37A 385-386 389-38A 38E-3A1 3A4-3CE 2018-2019 20AF

          'iso-8859-5',
          // Unique: 401-40C 40F-44F 452-45C 45F
          
          'iso-8859-8',
          // Unique: 5D1-5EA 200F
          
          'iso-8859-6',
          // Unique: 60C 61F 622-63A 641-652
          
          'iso-8859-11',
          // Unique: E01-E3A E40-E5B
          
          'iso-8859-15',
          // Unique: 153 161 17D-17E
          
          'iso-8859-14',
          // Unique: 10A-10B 121 175-178 1E03 1E0B 1E1F 1E41 1E57 1E61 1E6B 1E81-1E85 1EF3
          
          #'iso-8859-10',
          // Has nothing iso-8859-4 doesn't have
          
          'koi8r',
          // Has 410-44F, already covered by iso-8859-5
          // Has 2219-221A 2264-2265 2321 2502 2510 2518 2524 2534 2550-256C 2584 258C 2591-2593
          
          'shift-jis',
          'euckr',
          'big5'
        ) as $test_set)
        $sets_to_try[$test_set] = $test_set;
    }
    elseif($setname != 'unk')
    {
      $sets_to_try[$setname] = $setname;
    }
  }

  // Check which characters are missing
  $approximations = Array();
  if(!$authentic_only)for($n=0; $n<=0x10FFFF; ++$n)
    if(!isset($characters[$n]))
    {
      $trans = Array();
      // Check if some of the $setname's included in this font
      // has a good approximation for it
      foreach($sets_to_try as $setname)
      {
        $s = @iconv('ucs4', $setname.'//TRANSLIT', pack('N', $n));
        if($s !== false && strlen($s) > 0 && $s != '?')
        {
          $score = 2 + (iconv_strlen($s, $setname) == 1 && ord($s[0]) >= 128); // >= 128 gives 3
          if(empty($trans) || $score > $trans[0])
          {
            // Convert this symbol back to unicode.
            // Verify that this symbol actually exists in the font.
            $uninum = unpack('N', @iconv($setname, 'ucs4', $s));
            if(empty($uninum)) print "$setname $n -> $s bad\n";
            if(!empty($uninum) && isset($characters[$uninum[1]]))
            {
              $where = $characters[$uninum[1]];
              $trans = [$score, $where[0]/*setname*/, $where[1]/*index*/];
              if($score == 3) break;
            }
          }
        }
      /*
            break;
          default:
            $s = @iconv('utf-32', $setname.'//TRANSLIT', pack('V', $n));
            if($s !== false)
            {
              $score = 2;
              if($s == '?')       $score = 0;
              if(strlen($s) != 1) $score = 0;
              if(empty($trans) || $score > $trans[0])
                $trans = [$score, $s, $setname];
            }
      */
      }

      if(!empty($trans) && $trans[0] > 0)
      {
        $approximations[$n] = Array($trans[1], $trans[2]);
      }
      #else
      #  $approximations[$n] = Array($dfl_setname, ord('?'));
    }

  foreach($approximations as $k=>$v)
    $characters[$k] = $v;
  ksort($characters);

  file_put_contents($cache_fn, gzencode(serialize($characters)));
  
  #print_r($characters);
  return $characters;
}

$specs = Array
(
  '4x5' => ['chars_4x5'=>'4x5.bdf'],
  '4x6' => ['chars_4x6'=>'4x6.bdf'],
  '4x8' => ['chars_misaki4'=>'misakig2.bdf'],
  '5x7' => ['chars_5x7'=>'5x7.bdf'],
  '5x8' => ['chars_5x8'=>'5x8.bdf'],
  '6x9' => ['chars_6x9'=>'6x9.bdf'],
  '6x10' => ['chars_6x10'=>'6x10.bdf'],
  '6x12' => ['chars_6x12'=>'6x12.bdf',
             'cp_dum1' => 'Uni2-Terminus12x6.psf.gz',
             #'cp_dum2' => 'Uni3-Terminus12x6.psf.gz',
             #'cp_dum3' => 'FullCyrAsia-Terminus12x6.psf.gz',
             #'cp_dum4' => 'FullCyrSlav-Terminus12x6.psf.gz',
             #'cp_dum5' => 'FullGreek-Terminus12x6.psf.gz',
             'chars_mona6x12r'=>'mona6x12r.bdf'
            ],
  '6x13' => ['chars_6x13'=>'6x13.bdf'],
  '7x13' => ['chars_7x13'=>'7x13.bdf'],
  '7x14' => ['chars_7x14'=>'7x14.bdf',
             'chars_mona7x14r'=>'mona7x14r.bdf'
            ],
  '8x8' => ['chars_ib8x8'=>'ib8x8u.bdf',
            #'cp437'=>'8x8.inc',
            #'cp850'=>'850-8x8.asm',
            #'cp852'=>'852-8x8.asm',
            'cp_dum5' => 'FullCyrSlav-VGA8.psf.gz',
            'cp_dum6' => 'FullGreek-VGA8.psf.gz',
            'cp_dum7' => 'Arabic-VGA8.psf.gz',
            #'cp_dum8' => 'Hebrew-VGA8.psf.gz',
            'iso-8859-1'=>'iso01.f08.psf.gz',
            #'iso-8859-2'=>'iso02.f08.psf.gz',
            #'iso-8859-3'=>'iso03.f08.psf.gz',
            #'iso-8859-4'=>'iso04.f08.psf.gz',
            #'iso-8859-5'=>'iso05.f08.psf.gz', #Fixes cyrillic
            #'iso-8859-6'=>'iso06.f08.psf.gz',
            'iso-8859-7'=>'iso07.f08.psf.gz', #Fixes greek
            'iso-8859-8'=>'iso08.f08.psf.gz',
            #'cp860'=>'860-8x8.asm',
            #'cp863'=>'863-8x8.asm',
            #'cp865'=>'865-8x8.asm',
            #'cp857'=>'cp857-8x8.psf.gz',
            #'iso-8859-9'=>'iso09.f08.psf.gz',
            #'iso-8859-10'=>'iso10.f08.psf.gz',
            'latin1' => 'lat1-08.psf.gz',
            'latin2' => 'lat2-08.psf.gz',
            #'latin4' => 'lat4-08.psf.gz',
            #'latin9' => 'lat9-08.psf.gz',
            'cp_dum1' => 'Uni1-VGA8.psf.gz',
            #'cp_dum2' => 'Uni2-VGA8.psf.gz',
            'chars_misaki8' => 'misakig2.bdf',
           ],

  '8x10' => ['chars_cp437'=>'8x10.inc', // vga8x19 has cp437 + U+FFFD at 256
            'latin1' => 'lat1-10.psf.gz',
            'latin2' => 'lat2-10.psf.gz',
            'latin4' => 'lat4-10.psf.gz',
            'latin9' => 'lat9-10.psf.gz',
           ],

  '8x12'=> ['chars_cp850'=>'8x12.inc', // vga8x19 has cp437 + U+FFFD at 256
            'latin1' => 'lat1-12.psf.gz',
            'latin2' => 'lat2-12.psf.gz',
            'latin4' => 'lat4-12.psf.gz',
            'latin9' => 'lat9-12.psf.gz',
           ],

  '8x13' => ['chars_8x13'=>'8x13.bdf'],

  '8x14'=> ['chars_ie8x14'=>'ie8x14u.bdf',
            #'cp437'=>'8x14.inc',
            #'cp850'=>'850-8x14.asm',
            #'cp852'=>'852-8x14.asm',
            'cp_dum1' => 'Uni1-VGA14.psf.gz',
            'cp_dum2' => 'Uni2-VGA14.psf.gz',
            #'cp_dum5' => 'FullCyrSlav-VGA14.psf.gz',
            #'cp_dum6' => 'FullGreek-VGA14.psf.gz',
            #'cp_dum7' => 'Arabic-VGA14.psf.gz',
            #'cp_dum8' => 'Hebrew-VGA14.psf.gz',
            'cp_dum3'  => 'Uni3-TerminusBoldVGA14.psf.gz',
            'iso-8859-1'=>'iso01.f14.psf.gz',
            #'iso-8859-2'=>'iso02.f14.psf.gz',
            #'iso-8859-3'=>'iso03.f14.psf.gz',
            #'iso-8859-4'=>'iso04.f14.psf.gz',
            #'iso-8859-5'=>'iso05.f14.psf.gz',
            #'iso-8859-6'=>'iso06.f14.psf.gz',
            'iso-8859-7'=>'iso07.f14.psf.gz',
            'iso-8859-8'=>'iso08.f14.psf.gz',
            #'cp860'=>'860-8x14.asm',
            #'cp863'=>'863-8x14.asm',
            #'cp865'=>'865-8x14.asm',
            #'cp857'=>'cp857-8x14.psf.gz',
            #'iso-8859-9'=>'iso09.f14.psf.gz',
            #'iso-8859-10'=>'iso10.f14.psf.gz',
            'latin1' => 'lat1-14.psf.gz',
            'latin2' => 'lat2-14.psf.gz',
            #'latin4' => 'lat4-14.psf.gz',
            #'latin9' => 'lat9-14.psf.gz',
            'cp_term'=> 'FullGreek-Terminus14.psf.gz'
           ],

  '8x15'=> ['chars_cp437'=>'8x15.inc'],

  '8x16'=> ['chars_iv8x16'=>'iv8x16u.bdf',
            'cp_dum1' => 'Uni1-VGA16.psf.gz',
            #'cp437'=>'8x16.inc',
            #'cp850'=>'850-8x16.asm',
            #'cp852'=>'852-8x16.asm',
            #'cp860'=>'860-8x16.asm',
            #'cp863'=>'863-8x16.asm',
            #'cp865'=>'865-8x16.asm',
            'iso-8859-1'=>'iso01.f16.psf.gz',
            #'iso-8859-2'=>'iso02.f16.psf.gz',
            #'iso-8859-3'=>'iso03.f16.psf.gz',
            #'iso-8859-4'=>'iso04.f16.psf.gz',
            #'iso-8859-5'=>'iso05.f16.psf.gz',
            #'iso-8859-6'=>'iso06.f16.psf.gz',
            'iso-8859-7'=>'iso07.f16.psf.gz',
            'iso-8859-8'=>'iso08.f16.psf.gz',
            #'iso-8859-9'=>'iso09.f16.psf.gz',
            #'iso-8859-10'=>'iso10.f16.psf.gz',
            #'cp857'=>'cp857-8x16.psf.gz',
            'cp_dum2' => 'Uni2-VGA16.psf.gz',
            'cp_dum5' => 'FullCyrSlav-VGA16.psf.gz',
            'cp_dum3' => 'FullCyrAsia-TerminusBoldVGA16.psf.gz',
            #'cp_dum6' => 'FullGreek-VGA16.psf.gz',
            'cp_dum7' => 'Arabic-VGA16.psf.gz',
            'cp_dum8' => 'Hebrew-VGA16.psf.gz',
            'cp_dum3' => 'Uni3-TerminusBoldVGA16.psf.gz',
            'latin1' => 'lat1-16.psf.gz',
            'latin2' => 'lat2-16.psf.gz',
            #'latin4' => 'lat4-16.psf.gz',
            #'latin9' => 'lat9-16.psf.gz',
            'chars_unicsur8' => 'unifont-csur.bdf'
           ],

  '8x19'=> ['chars_vga8x19'=>'vga8x19.bdf'],

  '9x15' => ['chars_9x15'=>'9x15.bdf'
            ],
  '9x16' => ['chars_9x15cn'=>'gb16st.bdf' # Doesn't contribute
            ],
  '9x18' => ['chars_9x18'=>'9x18.bdf'],

  #'10x10' => ['chars_10x10ja'=>'f10.bdf'], # Seems autogenerated, bad quality

  '10x20' => ['chars_10x20'=>'10x20.bdf',
            'cp_dum1' => 'Uni2-TerminusBold20x10.psf.gz',
            #'cp_dum2' => 'Uni3-TerminusBold20x10.psf.gz',
            #'cp_dum5' => 'FullCyrSlav-TerminusBold20x10.psf.gz',
            #'cp_dum3' => 'FullCyrAsia-TerminusBold20x10.psf.gz',
            #'cp_dum6' => 'FullGreek-TerminusBold20x10.psf.gz'
            'cp_term' => 'FullCyrSlav-Terminus20x10.psf.gz',
             ],
  '10x24' => ['chars_10x20cn' => 'gb24st.bdf'],

  '12x13' => ['chars_12x13ja'=>'12x13ja.bdf'],
  '12x12' => ['chars_monak12'=>'monak12.bdf',
              'chars_12x12ja'=>'f12.bdf'],
  '14x14' => ['chars_monak14'=>'monak14.bdf',
              'chars_14x14ja'=>'f14.bdf'
             ],

  '11x22'=> ['cp_dum1' => 'Uni2-TerminusBold22x11.psf.gz',
            'cp_dum2' => 'Uni3-TerminusBold22x11.psf.gz',
            'cp_dum5' => 'FullCyrSlav-TerminusBold22x11.psf.gz',
            'cp_dum3' => 'FullCyrAsia-TerminusBold22x11.psf.gz',
            #'cp_dum6' => 'FullGreek-TerminusBold22x11.psf.gz',
            'cp_term' => 'FullCyrAsia-Terminus22x11.psf.gz',
           ],
  '12x24'=> ['cp_dum1' => 'Uni2-TerminusBold24x12.psf.gz',
            'cp_dum2' => 'Uni3-TerminusBold24x12.psf.gz',
            'cp_dum5' => 'FullCyrSlav-TerminusBold24x12.psf.gz',
            'cp_dum3' => 'FullCyrAsia-TerminusBold24x12.psf.gz',
            #'cp_dum6' => 'FullGreek-TerminusBold24x12.psf.gz',
            'cp_term' => 'Uni3-Terminus24x12.psf.gz',
           ],
  '14x28'=> ['cp_dum1' => 'Uni2-TerminusBold28x14.psf.gz',
            'cp_dum2' => 'Uni3-TerminusBold28x14.psf.gz',
            'cp_dum5' => 'FullCyrSlav-TerminusBold28x14.psf.gz',
            'cp_dum3' => 'FullCyrAsia-TerminusBold28x14.psf.gz',
            #'cp_dum6' => 'FullGreek-TerminusBold28x14.psf.gz',
            'cp_term' => 'CyrKoi-Terminus28x14.psf.gz',
           ],

  '16x16'=> ['chars_unicsur16' => 'unifont-csur.bdf',
             'chars_16x15cn' => 'gb16st.bdf' # Doesn't contribute
             #'chars_monak16'   => 'monak16.bdf' # Doesn't contribute
            ],

  '16x28'=> ['cp_dum1' => 'Uni1-VGA28x16.psf.gz',
            'cp_dum2' => 'Uni2-VGA28x16.psf.gz',
            'cp_dum5' => 'FullCyrSlav-VGA28x16.psf.gz',
            'cp_dum6' => 'FullGreek-VGA28x16.psf.gz',
            #'cp_dum7' => 'Arabic-VGA28x16.psf.gz',
            #'cp_dum8' => 'Hebrew-VGA28x16.psf.gz',
           ],
  '16x32'=> ['cp_dum1' => 'Uni1-VGA32x16.psf.gz',
            'cp_dum2' => 'Uni2-VGA32x16.psf.gz',
            'cp_dum5' => 'FullCyrSlav-VGA32x16.psf.gz',
            #'cp_dum6' => 'FullGreek-VGA32x16.psf.gz',
            'cp_dum7' => 'Arabic-VGA32x16.psf.gz',
            'cp_dum8' => 'Hebrew-VGA32x16.psf.gz',
            'cp_term' => 'FullGreek-Terminus32x16.psf.gz',
           ],

  '18x18' => ['chars_18x18ja'=>'18x18ja.bdf',
              'chars_18x18ko'=>'18x18ko.bdf'],

  '24x24' => ['chars_24x24cn'=>'cmex24m.bdf',
              'chars_24x20cn'=>'gb24st.bdf'],
);

/* If a .psf.gz specifies its own character set, incorporate it */
foreach($specs as $size => $selections)
{
  $newsel = Array();
  $changes = false;
  foreach($selections as $cset => $filename)
  {
    if(preg_match('/\.psf\.gz$/', $filename))
    {
      $translation = Read_PSFgzEncoding("data/$filename");
      if($translation !== false)
      {
        $csetname = 'chars_'.strtr($filename, '.-/', '___');
        global $$csetname;
        $tmp = array_keys($translation);
        $$csetname = array_combine($tmp, $tmp);
        $newsel[$csetname] = $filename;
        $changes = true;
        continue;
      }
    }
    $newsel[$cset] = $filename;
  }
  if($changes)
    $specs[$size] = $newsel;
}

/* Merge identical font-specific sets */
$sets = Array();
foreach($GLOBALS as $name => $value)
  if(preg_match('/^chars_/', $name))
    $sets[serialize($value)][] = $name;

foreach($sets as $k=>$names)
  if(count($names) > 1)
  {
    reset($names);
    list($n,$name) = each($names);
    foreach($specs as $size => $selections)
    {
      $newsel = Array();
      foreach($selections as $cset => $filename)
        if(array_search($cset, $names) !== false)
          $newsel[$name] = $filename;
        else
          $newsel[$cset] = $filename;
      $specs[$size] = $newsel;
    }
  }
#print_r($specs);
#exit;

$t = Array();
foreach($specs as $size => $selections)
{
  preg_match('/(\d+)x(\d+)/', $size, $mat);
  #/*
  if($argc < 3)
  {
    $p = proc_open("php {$argv[0]} {$authentic_only} {$size} | dd bs=16M iflag=fullblock 2>/dev/null",
      [0=>['file','php://stdin','r'],
       1=>['pipe','w'],
       2=>['file','php://stderr','w']], $pipes);
    $t[] = Array($p,$pipes);
    continue;
  }
  if($size != $argv[2]) continue;
  #*/

  $fontwidth  = $mat[1];
  $fontheight = $mat[2];
  $map      = CreateTranslation($size, array_keys($selections), $authentic_only);
  
  #if($fontwidth != 8) continue;
  #if($fontheight != 15 && $fontheight != 19) continue;

  $used = Array();
  foreach($map as $specs) $used[$specs[0]] = true;
  foreach($selections as $setname=>$dummy)
    if(!isset($used[$setname]))
      file_put_contents('php://stderr', "// $size: $setname is unused\n");

  $fontdata = Array();
  foreach($selections as $encoding => $filename)
  {
    if(!file_exists("data/$filename"))
      $filename = str_replace('.asm', '.inc', $filename);
    $fontdata[$encoding] = Read_Font("data/$filename", $fontwidth, $fontheight);
  }
  
  $bitmap  = Array();
  $revmap  = Array();
  $done    = Array();
  $n = 0;
  foreach($map as $unicodevalue => $specs)
  {
    $setname = $specs[0];
    $index   = $specs[1];
    if(!isset($fontdata[$setname][$index]))
    {
      file_put_contents('php://stderr', "$index not found in $setname (".json_encode($selections).")\n");
    }
    $cell = $fontdata[$setname][$index];
    $key = md5(json_encode($cell));
    if(!isset($done[$key]))
    {
      $done[$key] = $n;
      $bytes = $fontheight * (($fontwidth + 7) >> 3);
      for($b=0; $b<$bytes; ++$b) $bitmap[] = (int)$cell[$b];
      ++$n;
    }
    $revmap[$unicodevalue] = $done[$key];
    #printf("U+%04X: Glyph index %d, source glyph index %d in %s\n",
    #  $unicodevalue, $done[$key], $index, $setname);
  }
  
  $globalname = 'f'.$size;
  print "namespace ns_$globalname {\n";

  $font = new Font;
  $font->GenerateOutput($bitmap, $fontwidth, $fontheight, $revmap);

  print "}\n";
  /*
  print "struct $globalname: public UIfontBase\n";
  print "{\n";
  print "    virtual const unsigned char* GetBitmap() const { return ns_$globalname::bitmap; }\n";
  print "    virtual unsigned GetIndex(char32_t c) const { return ns_$globalname::unicode_to_bitmap_index(c); }\n";
  print "};\n";
  print "static UIfontBase* Get{$globalname}()\n";
  print "{\n";
  print "    static $globalname f;\n";
  print "    return &f;\n";
  print "}\n";
  */
}

/*
$remaining = count($t);
while($remaining)
{
  $r = Array(); $w = null; $e = null;

  foreach($t as $tt) $r[] = $tt[1][1];

  stream_select($r, $w,$e, null,null);

  foreach($r as $s)
  {
    print stream_get_contents($s);
    --$remaining;
  }
}
*/
foreach($t as $process)
{
  print stream_get_contents($process[1][1]);
  proc_close($process[0]);
}
