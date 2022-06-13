<?php
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

for($n=0; $n<=0x10FFFF; ++$n)
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
        if(empty($trans))
        {
          // Convert this symbol back to unicode.
          // Verify that this symbol actually exists in the font.
          $uninum = unpack('N', @iconv($setname, 'ucs4', $s));
          if(empty($uninum)) print "BAD $setname $n -> $s\n";
          if(!empty($uninum)) // && isset($characters[$uninum[1]]))
          {
            $n2 = $uninum[1];
            if($n2 != $n)
              $trans[$n2] = $score;
          }
        }
      }
    }
    if(count($trans) > 0)
    {
      arsort($trans);
      printf("%X", $n);
      foreach($trans as $k=>$v)
        printf("  %X %u", $k,$v);
      print "\n";
    }
  }
