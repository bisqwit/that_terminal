<?php

require 'read_font.php';
require 'font_gen2.php';

// grep ^ENCODING data/6x9.bdf|sed 's/ENCODING //'|tr '\012' ,
$chars_6x9 = Array(0,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,351,352,353,354,355,356,357,358,359,360,361,362,363,364,365,366,367,368,369,370,371,372,373,374,375,376,377,378,379,380,381,382,383,399,402,416,417,431,432,437,438,465,466,486,487,506,507,508,509,510,511,536,537,538,539,600,601,699,700,701,710,711,713,728,729,730,731,732,733,768,769,770,771,772,773,774,775,776,777,778,779,780,781,782,783,784,785,786,787,788,803,804,884,885,890,894,900,901,902,903,904,905,906,908,910,911,912,913,914,915,916,917,918,919,920,921,922,923,924,925,926,927,928,929,931,932,933,934,935,936,937,938,939,940,941,942,943,944,945,946,947,948,949,950,951,952,953,954,955,956,957,958,959,960,961,962,963,964,965,966,967,968,969,970,971,972,973,974,976,977,978,979,980,981,982,983,986,987,988,989,990,991,992,993,994,995,996,997,998,999,1000,1001,1002,1003,1004,1005,1006,1007,1008,1009,1010,1011,1012,1013,1024,1025,1026,1027,1028,1029,1030,1031,1032,1033,1034,1035,1036,1037,1038,1039,1040,1041,1042,1043,1044,1045,1046,1047,1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1059,1060,1061,1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,1072,1073,1074,1075,1076,1077,1078,1079,1080,1081,1082,1083,1084,1085,1086,1087,1088,1089,1090,1091,1092,1093,1094,1095,1096,1097,1098,1099,1100,1101,1102,1103,1104,1105,1106,1107,1108,1109,1110,1111,1112,1113,1114,1115,1116,1117,1118,1119,1122,1123,1136,1137,1138,1139,1140,1141,1168,1169,1170,1171,1174,1175,1178,1179,1198,1199,1200,1201,1202,1203,1210,1211,1240,1241,1250,1251,1256,1257,1262,1263,1488,1489,1490,1491,1492,1493,1494,1495,1496,1497,1498,1499,1500,1501,1502,1503,1504,1505,1506,1507,1508,1509,1510,1511,1512,1513,1514,1520,1521,1522,1523,1524,7682,7683,7690,7691,7710,7711,7744,7745,7766,7767,7776,7777,7786,7787,7808,7809,7810,7811,7812,7813,7922,7923,8208,8209,8210,8211,8212,8213,8214,8215,8216,8217,8218,8219,8220,8221,8222,8223,8224,8225,8226,8227,8228,8229,8230,8231,8240,8242,8243,8244,8245,8246,8247,8249,8250,8252,8254,8260,8304,8305,8308,8309,8310,8311,8312,8313,8314,8315,8316,8317,8318,8319,8320,8321,8322,8323,8324,8325,8326,8327,8328,8329,8330,8331,8332,8333,8334,8355,8356,8359,8363,8364,8367,8400,8401,8402,8403,8404,8405,8406,8407,8450,8453,8467,8469,8470,8474,8477,8482,8484,8486,8494,8539,8540,8541,8542,8592,8593,8594,8595,8596,8597,8612,8613,8614,8615,8616,8656,8657,8658,8659,8660,8661,8704,8705,8706,8707,8708,8709,8710,8711,8712,8713,8715,8716,8719,8720,8721,8722,8723,8725,8728,8729,8730,8733,8734,8735,8737,8740,8741,8742,8743,8744,8745,8746,8747,8750,8756,8757,8758,8759,8760,8761,8762,8763,8764,8765,8771,8773,8776,8777,8793,8799,8800,8801,8802,8803,8804,8805,8810,8811,8834,8835,8836,8837,8838,8839,8840,8841,8842,8843,8853,8854,8855,8856,8857,8866,8867,8868,8869,8870,8871,8872,8896,8897,8898,8899,8901,8942,8943,8944,8945,8960,8962,8968,8969,8970,8971,8976,8992,8993,9146,9147,9148,9149,9225,9226,9227,9228,9229,9252,9472,9473,9474,9475,9484,9488,9492,9496,9500,9508,9516,9524,9532,9548,9549,9550,9551,9552,9553,9554,9555,9556,9557,9558,9559,9560,9561,9562,9563,9564,9565,9566,9567,9568,9569,9570,9571,9572,9573,9574,9575,9576,9577,9578,9579,9580,9581,9582,9583,9584,9585,9586,9587,9600,9601,9602,9603,9604,9605,9606,9607,9608,9609,9610,9611,9612,9613,9614,9615,9616,9617,9618,9619,9620,9621,9622,9623,9624,9625,9626,9627,9628,9629,9630,9631,9632,9633,9634,9635,9642,9643,9644,9645,9646,9647,9648,9649,9650,9651,9652,9653,9654,9655,9656,9657,9658,9659,9660,9661,9662,9663,9664,9665,9666,9667,9668,9669,9670,9674,9675,9679,9688,9689,9702,9728,9785,9786,9787,9788,9791,9792,9793,9794,9824,9825,9826,9827,9828,9829,9830,9833,9834,9835,10216,10217,10240,10241,10242,10243,10244,10245,10246,10247,10248,10249,10250,10251,10252,10253,10254,10255,10256,10257,10258,10259,10260,10261,10262,10263,10264,10265,10266,10267,10268,10269,10270,10271,10272,10273,10274,10275,10276,10277,10278,10279,10280,10281,10282,10283,10284,10285,10286,10287,10288,10289,10290,10291,10292,10293,10294,10295,10296,10297,10298,10299,10300,10301,10302,10303,10304,10305,10306,10307,10308,10309,10310,10311,10312,10313,10314,10315,10316,10317,10318,10319,10320,10321,10322,10323,10324,10325,10326,10327,10328,10329,10330,10331,10332,10333,10334,10335,10336,10337,10338,10339,10340,10341,10342,10343,10344,10345,10346,10347,10348,10349,10350,10351,10352,10353,10354,10355,10356,10357,10358,10359,10360,10361,10362,10363,10364,10365,10366,10367,10368,10369,10370,10371,10372,10373,10374,10375,10376,10377,10378,10379,10380,10381,10382,10383,10384,10385,10386,10387,10388,10389,10390,10391,10392,10393,10394,10395,10396,10397,10398,10399,10400,10401,10402,10403,10404,10405,10406,10407,10408,10409,10410,10411,10412,10413,10414,10415,10416,10417,10418,10419,10420,10421,10422,10423,10424,10425,10426,10427,10428,10429,10430,10431,10432,10433,10434,10435,10436,10437,10438,10439,10440,10441,10442,10443,10444,10445,10446,10447,10448,10449,10450,10451,10452,10453,10454,10455,10456,10457,10458,10459,10460,10461,10462,10463,10464,10465,10466,10467,10468,10469,10470,10471,10472,10473,10474,10475,10476,10477,10478,10479,10480,10481,10482,10483,10484,10485,10486,10487,10488,10489,10490,10491,10492,10493,10494,10495,64257,64258,65533);
$chars_6x9 = array_combine($chars_6x9, $chars_6x9);

function CreateTranslation($sets)
{
  global $chars_6x9;
  $characters = Array();
  $dfl_setname = 'cp437';
  foreach($sets as $setname) { $dfl_setname = $setname; break; }
  // Check which characters we have
  foreach($sets as $setname)
    switch($setname)
    {
      case 'chars_6x9':
        foreach($chars_6x9 as $n)
          if(!isset($characters[$n]))
            $characters[$n] = Array($setname, $n);
        break;
      default:
        for($n=0; $n<256; ++$n)
        {
          $s = @iconv($setname, 'utf-32', chr($n));
          if($s !== false)
          {
            $cno = unpack('V/V', $s);
            $utfchar = $cno[1];
            // This $utfchar is found in $setname at index $n
            if(!isset($characters[$utfchar]))
              $characters[$utfchar] = Array($setname, $n);
          }
        }
    }
  // Check which characters are missing
  for($n=0; $n<=0x10FFFF; ++$n)
    if(!isset($characters[$n]))
    {
      $trans = Array();
      // Check if some $setname has a good approximation for it
      foreach($sets as $setname)
        switch($setname)
        {
          case 'chars_6x9':
            foreach(Array('iso-8859-1','iso-8859-2','iso-8859-15','iso-8859-4','cp857','cp850') as $test_set)
            {
              $s = @iconv('utf-32', $test_set.'//TRANSLIT', pack('V', $n));
              if($s !== false)
              {
                $score = 2;
                if($s == '?')       $score = 0;
                if(strlen($s) != 1) $score = 0;
                else
                {
                  if(ord($s) >= 128) $score = 3;
                  if(!isset($characters[ord($s)])) $score = 0;
                }
                if(empty($trans) || $score > $trans[0])
                {
                  $trans = [$score, $s, $setname];
                  if($score == 3) break;
                }
              }
            }
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
        }
      if(!empty($trans) && $trans[0] > 0)
        $characters[$n] = Array($trans[2], ord($trans[1]));
      #else
      #  $characters[$n] = Array($dfl_setname, ord('?'));
    }
  ksort($characters);
  
  #print_r($characters);
  return $characters;
}

$specs = Array
(
  '6x9' => ['chars_6x9'=>'6x9.bdf'],
  '8x8' => ['cp437'=>'8x8.inc',
            'cp850'=>'850-8x8.asm',
            'cp852'=>'852-8x8.asm',
            //'cp860'=>'860-8x8.asm',
            //'cp863'=>'863-8x8.asm',
            //'cp865'=>'865-8x8.asm',
            'cp857'=>'cp857-8x8.psf.gz',
            //'iso-8859-1'=>'iso01.f08.psf.gz',
            //'iso-8859-2'=>'iso02.f08.psf.gz',
            'iso-8859-3'=>'iso03.f08.psf.gz',
            'iso-8859-4'=>'iso04.f08.psf.gz',
            'iso-8859-5'=>'iso05.f08.psf.gz',
            'iso-8859-6'=>'iso06.f08.psf.gz',
            'iso-8859-7'=>'iso07.f08.psf.gz',
            'iso-8859-8'=>'iso08.f08.psf.gz',
            //'iso-8859-9'=>'iso09.f08.psf.gz',
            //'iso-8859-10'=>'iso10.f08.psf.gz',
            //'latin1' => 'lat1-08.psf.gz',
            //'latin2' => 'lat2-08.psf.gz',
            //'latin4' => 'lat4-08.psf.gz',
            'latin9' => 'lat9-08.psf.gz',
            //'koi8r'=>'koi8-8x8.psf.gz'
            ],
  '8x10' => ['cp437'=>'8x10.inc',
            'latin1' => 'lat1-10.psf.gz',
            'latin2' => 'lat2-10.psf.gz',
            'latin4' => 'lat4-10.psf.gz',
            'latin9' => 'lat9-10.psf.gz'],
  '8x12'=> ['cp437'=>'8x12.inc',
            'latin1' => 'lat1-12.psf.gz',
            'latin2' => 'lat2-12.psf.gz',
            'latin4' => 'lat4-12.psf.gz',
            'latin9' => 'lat9-12.psf.gz'],
  '8x14'=> ['cp437'=>'8x14.inc',
            'cp850'=>'850-8x14.asm',
            'cp852'=>'852-8x14.asm',
            //'cp860'=>'860-8x14.asm',
            //'cp863'=>'863-8x14.asm',
            //'cp865'=>'865-8x14.asm',
            'cp857'=>'cp857-8x14.psf.gz',
            //'iso-8859-1'=>'iso01.f14.psf.gz',
            //'iso-8859-2'=>'iso02.f14.psf.gz',
            'iso-8859-3'=>'iso03.f14.psf.gz',
            'iso-8859-4'=>'iso04.f14.psf.gz',
            'iso-8859-5'=>'iso05.f14.psf.gz',
            'iso-8859-6'=>'iso06.f14.psf.gz',
            'iso-8859-7'=>'iso07.f14.psf.gz',
            'iso-8859-8'=>'iso08.f14.psf.gz',
            //'iso-8859-9'=>'iso09.f14.psf.gz',
            //'iso-8859-10'=>'iso10.f14.psf.gz',
            //'latin1' => 'lat1-14.psf.gz',
            //'latin2' => 'lat2-14.psf.gz',
            //'latin4' => 'lat4-14.psf.gz',
            'latin9' => 'lat9-14.psf.gz',
            //'koi8r'=>'koi8-8x14.psf.gz'
           ],
  '8x15'=> ['cp437'=>'8x15.inc'],
  '8x16'=> ['cp437'=>'8x16.inc',
            'cp850'=>'850-8x16.asm',
            'cp852'=>'852-8x16.asm',
            //'cp860'=>'860-8x16.asm',
            //'cp863'=>'863-8x16.asm',
            //'cp865'=>'865-8x16.asm',
            'cp857'=>'cp857-8x16.psf.gz',
            //'iso-8859-1'=>'iso01.f16.psf.gz',
            //'iso-8859-2'=>'iso02.f16.psf.gz',
            'iso-8859-3'=>'iso03.f16.psf.gz',
            'iso-8859-4'=>'iso04.f16.psf.gz',
            'iso-8859-5'=>'iso05.f16.psf.gz',
            'iso-8859-6'=>'iso06.f16.psf.gz',
            'iso-8859-7'=>'iso07.f16.psf.gz',
            'iso-8859-8'=>'iso08.f16.psf.gz',
            //'iso-8859-9'=>'iso09.f16.psf.gz',
            //'iso-8859-10'=>'iso10.f16.psf.gz',
            //'latin1' => 'lat1-16.psf.gz',
            //'latin2' => 'lat2-16.psf.gz',
            //'latin4' => 'lat4-16.psf.gz',
            'latin9' => 'lat9-16.psf.gz',
            //'koi8r'=>'koi8-8x16.psf.gz'
           ],
  '8x19'=> ['cp437'=>'vga8x19.bdf']
);

$t = Array();
foreach($specs as $size => $selections)
{
  preg_match('/(\d+)x(\d+)/', $size, $mat);
  if($argc == 1)
  {
    $p = proc_open("php {$argv[0]} {$size}", [0=>['file','php://stdin','r'],
                                              1=>['pipe','w'],
                                              2=>['file','php://stderr','w']], $pipes);
    $t[] = Array($p,$pipes);
    continue;
  }
  if($size != $argv[1]) continue;

  $fontwidth  = $mat[1];
  $fontheight = $mat[2];
  $map      = CreateTranslation(array_keys($selections));

  $used = Array();
  foreach($map as $specs) $used[$specs[0]] = true;
  foreach($selections as $setname=>$dummy)
    if(!isset($used[$setname]))
      print "// $size: $setname is unused\n";

  $fontdata = Array();
  foreach($selections as $encoding => $filename)
  {
    if(!file_exists("data/$filename"))
      $filename = str_replace('.asm', '.inc', $filename);
    $fontdata[$encoding] = Read_Font("data/$filename", $fontheight);
  }
  #print_r($fontdata);
  
  $bitmap  = Array();
  $revmap  = Array();
  $done    = Array();
  $n = 0;
  foreach($map as $unicodevalue => $specs)
  {
    $setname = $specs[0];
    $index   = $specs[1];
    $cell = $fontdata[$setname][$index];
    $key = md5(json_encode($cell));
    if(!isset($done[$key]))
    {
      $done[$key] = $n;
      for($y=0; $y<$fontheight; ++$y) $bitmap[] = (int)$cell[$y];
      ++$n;
    }
    $revmap[$unicodevalue] = $done[$key];
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
