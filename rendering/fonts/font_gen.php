<?php

class Font
{
  public function GenerateOutput(
    // 8 bits per scanline, Height bytes per character
    // I.e. bytes for character N are N*height to (N+1)*height
    $bitmap,
    // Width and height
    $width, $height,
    $revmap)
  {
    $n = count($bitmap);
    print "static const unsigned char bitmap[$n] = {\n";
    $n=0;
    $chno=0;
    #print_r($unicode_map);
    $bytes_x = ($width+7) >> 3;
    foreach($bitmap as $value)
    {
      printf("0x%02X,", $value);
      if(++$n == $height * $bytes_x)
      {
        $n=0;
        $s = Array();
        foreach($revmap as $u=>$v) if($v==$chno) $s[] = sprintf('U+%04X', $u);
        printf(" /* %02X (%s) */\n", $chno, join(', ', $s));
        #print "\n";
        ++$chno;
      }
    }
    print "};\n";
    
    GenerateOptimalTablePresentation($revmap, 'unicode_to_bitmap_index', $chno);
  }
};

function MakeCType($table, $size_only = false)
{
  $index = ceil( log(log(max($table)) / log(256)) / log(2) );
  if($index <= 0) $index = 0;
  if($size_only) return $index;

  $types = Array('std::uint_least8_t', 'std::uint_least16_t', 'std::uint_least32_t', 'std::uint_least64_t');
  return $types[$index];
}

function GenerateOptimalTablePresentation($table, $name, $significant)
{
  #$offset_map = Array();
  #foreach($table as $k=>$v)
  #  @$offset_map[$v-$k] += 1;
  #asort($offset_map);
  ksort($table);
  
  $tables = Array();

  $end = count($table);
  
  $default = 'index';

  if($significant <= 256)
  {
    // FIXME
    $default = "Help(index,InRecursion)";
  }
  
  #print_r($table);

  $mask = 0xFFFFFFF;
  if(max($table) < 65536) $mask = 0xFFFF;
  if(max($table) < 256)   $mask = 0xFF;

  // $table:   Unicode into INDEX
  
  $in_type  = MakeCType(array_keys($table));
  $out_type = MakeCType(array_values($table));

  $ifs = Array();

  /* First step: Find any sections that are long enough that consist
   *             of simply input_value + constant.
   */

  $prev_offset  = null;
  $region_begin = 0;
  $region_end   = 0;
  $good = Array();
  ksort($table);
  $flush = function()use(&$region_end,&$region_begin,&$good,&$prev_offset,$end)
  {
    $region_length = $region_end - $region_begin;
    if($region_length >= 8)
    {
      if($prev_offset != 0 
      || $region_begin == 0
      || $region_end   == $end)
      {
        $good[$prev_offset][] = Array($region_begin,$region_end);
      }
    }
  };
  foreach($table as $k=>$v)
  {
    $offset = $v - ($k & $mask);
    
    if($offset !== $prev_offset)
    {
      if($region_end > 0) $flush();
      $region_begin = $k;
      $prev_offset  = $offset;
    }
    $region_end = $k+1;
  }
  if($region_end > 0) $flush();

  foreach($good as $offset => $regions)
  {
    $rstr = Array();
    $coverage = 0;
    $moffset = -$offset;
    foreach($regions as $r)
      $ifs[ $r[0] ]
        = Array( $r[0], $r[1], "index - $moffset"
               );
  }

  foreach($good as $datas)
    foreach($datas as $data)
      for($k=$data[0]; $k!=$data[1]; ++$k)
        unset($table[$k]);

  /* After this, remove any part where key==value */
  foreach($table as $k=>$v)
    if($v == ($k & $mask))
      unset($table[$k]);

  /* Second step: Find any sections that are long enough that
   *              consist of _any_ values for consecutive keys.
   */
  
  $region_begin = 0;
  $region_end   = -1;
  $good = Array();
  asort($table);
  $flush = function()use(&$region_end,&$region_begin,&$good,$end)
  {
    $region_length = $region_end - $region_begin;
    if($region_length >= 8)
    {
      $good[$region_begin] = $region_end;
    }
  };
  #print_r($table);
  foreach($table as $k=>$v)
  {
    if($k != $region_end)
    {
      if($region_end > 0) $flush();
      $region_begin = $k;
    }
    $region_end = $k+1;
  }
  if($region_end > 0) $flush();
  #print_r($good);

  foreach($good as $begin => $end)
  {
    $tab = Array();
    for($k=$begin; $k!=$end; ++$k)
      $tab[] = $table[$k];
    
    $tabname = "{$name}_" . MakeCType($tab,true);
    $tabbegin = count(@$tables[$tabname]['data']);
    foreach($tab as $k)
      $tables[$tabname]['data'][] = $k;
    $tables[$tabname]['type'] = MakeCType($tab);

    $offs = $tabbegin - $begin;
    $ifs[$begin]
      = Array( $begin, $end, "{$tabname}[index + $offs]"
             );
  }

  foreach($good as $begin=>$end)
    for($k=$begin; $k!=$end; ++$k)
      unset( $table[$k] );


if(0)
{
  /* Third step: Find any sections that are long enough that
   *             consist of _any_ values for consecutive VALUES.
   */
  
  $region      = Array();
  $prev_value  = null;
  $first_value = null;

  $good = Array();
  asort($table);
  $flush = function()use(&$region,&$first_value,&$prev_value,&$good,$end)
  {
    if(count($region) >= 8)
    {
      $good[$first_value] = $region;
    }
  };
  #print_r($table);
  foreach($table as $k=>$v)
  {
    if(($v-1) !== $prev_value)
    {
      $flush();
      $region      = Array();
      $first_value = $v;
    }
    $region[]   = $k;
    $prev_value = $v;
  }
  $flush();
  #print_r($good);

  foreach($good as $first_value => $tab)
  {
    $begin = min($tab);
    $end   = max($tab)+1;
    if(count($tab) == 1)
    {
      $ifs[$begin]
        = Array( $begin, $end, "$first_value"
               );
    }
    else
    {
      $tabname = "{$name}_" . MakeCType($tab,true);
      $tabbegin = count(@$tables[$tabname]['data']);
      foreach($tab as $k)
        $tables[$tabname]['data'][] = $k;
      $tabend   = count(@$tables[$tabname]['data']);
      $tables[$tabname]['type'] = MakeCType($tab);

      $ifs[$begin]
        = Array( $begin, $end, "binarysearch(index, $tabname+$tabbegin, $tabname+$tabend, [](unsigned pos)->$out_type { return $first_value + pos; },".
                                                     " $default)"
               );
    }
  }

  foreach($good as $first_value => $tab)
    foreach($tab as $k)
      unset( $table[$k] );
}//third part endif

  asort($table);

  $n = count($table);
  if($n)
  {
    $tab = array_keys($table);
    $intabname = "{$name}_" . MakeCType($tab,true);
    $intabbegin = count(@$tables[$intabname]['data']);
    foreach($tab as $k)
      $tables[$intabname]['data'][] = $k;
    $intabend   = count(@$tables[$intabname]['data']);
    $tables[$intabname]['type'] = MakeCType($tab);

    $tab = array_values($table);
    $outtabname = "{$name}_" . MakeCType($tab,true);
    $outtabbegin = count(@$tables[$outtabname]['data']);
    foreach($tab as $k)
      $tables[$outtabname]['data'][] = $k;
    $outtabend   = count(@$tables[$outtabname]['data']);
    $tables[$outtabname]['type'] = MakeCType($tab);
  }

  foreach($tables as $tabname => $tabdata)
  {
    $tabsize = count($tabdata['data']);
    print "static const {$tabdata['type']} {$tabname}[$tabsize] =\n";
    print "{ " . join(',', $tabdata['data']) . " };\n";
  }
  
  print "static const struct {$name}_generator\n";
  print "{\n";
  if($n)
  {
    print "    template<typename T, typename T2>\n";
    print "    static $out_type binarysearch\n";
    print "       ($in_type index, const T* begin, const T* end,\n";
    print "        T2&& act, $out_type other)\n";
    print "    {\n";
    print "        auto i = std::lower_bound(begin, end, index);\n";
    print "        return (i != end && *i == index) ?  act(i-begin) : other;\n";
    print "    }\n";
    print "    static $out_type DefFind($in_type index)\n";
    print "    {\n";

    print "        return binarysearch(index, $intabname+$intabbegin, $intabname+$intabend,\n";
    print "             [](unsigned pos)->$out_type { return {$outtabname}[$outtabbegin+pos]; },\n";
    print "             $default);\n";
    print "    }\n";
  }
  
  $other = $n ? "DefFind(index)" : "$default";

  if($significant <= 256)
  {
    print "    static $out_type Help($in_type index, bool InRecursion)\n";
    print "    {\n";
    print "        return InRecursion ? (index & 0xFF) : Find(UnicodeToASCIIapproximation(index)&0xFF, true)\n";
    print "    }\n";
  }
  print "    static $out_type Find($in_type index, bool InRecursion)\n";
  print "    {\n";
  print "        return\n";

  ksort($ifs);
  #print_r($ifs);
  MakeRecursiveIftree($ifs, $other, '        ', 0,0x1FFFFF);
 
  print "        ;\n";
  print "    }\n";

  print "    $out_type operator[] ($in_type index) const { return Find(index, false); }\n";
  print "} $name;\n";
}

function MakeRecursiveIfTree($ifs, $other, $indent, $ge,$lt, $first='  ')
{
  /* $ge,$lt describes the confidence of what we know currently.
  */

  // First, figure out which conditions _can_ apply here.
  $applicable = Array();
  foreach($ifs as $begin=>$case)
    if($begin < $lt && $case[1] > $ge)
      $applicable[$begin] = $case;

  switch(count($applicable))
  {
    case 0:
      print "{$indent}{$first}$other\n";
      return;
    case 1:
      foreach($applicable as $begin=>$case) {}
      $expression = $case[2];
      $conditions = Array();
      if($case[0] > $ge) $conditions[] = "index >= {$case[0]}";
      if($case[1] < $lt) $conditions[] = "index <  {$case[1]}";
      if(!empty($conditions))
        $expression = "((".join(" && ",$conditions).") ? $expression : $other)";
      print "{$indent}{$first}$expression\n";
      return;
    case 2:
      reset($applicable);
      list($dummy,$a) = each($applicable);
      list($dummy,$b) = each($applicable);
      reset($applicable);
      #print "-- $ge, $lt --\n";
      #print serialize($a)."\n";
      #print serialize($b)."\n";
      if($b[0] == $a[1] && $a[0] == $ge && $b[1] == $lt)
      {
        $expression = "((index < {$b[0]}) ? {$a[2]} : {$b[2]})";
        print "{$indent}{$first}$expression\n";
        return;
      }
  }
  // Find midpoint from the selection.
  $n = count($applicable);
  $m = 0;
  $midpoint = 0;
  foreach($applicable as $begin=>$case)
  {
    if($m >= $n) { $midpoint = $begin; break; }
    $m += 2;
  }

  print "{$indent}{$first}(index < $midpoint)\n";
  MakeRecursiveIfTree($ifs, $other, "$indent  ", $ge, $midpoint, '? ');
  MakeRecursiveIfTree($ifs, $other, "$indent  ", $midpoint, $lt, ': ');
}
