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
    
    ksort($revmap);
    $min = min(array_keys($revmap));
    $max = max(array_keys($revmap));
    $values = Array();
    $maxval = max(array_values($revmap));
    for($n=$min; $n<=$max; ++$n)
    {
      if(isset($revmap[$n]))
        $values[$n-$min] = $revmap[$n];
      else
        $values[$n-$min] = $revmap[ord('?')];
    }
    $type = 'unsigned';
    if($maxval < 65536) $type = 'std::uint_least16_t';
    if($maxval < 256)   $type = 'std::uint_least8_t';

    printf("static const %s trans[%u] = { %s };\n",
      $type, $max-$min+1, join(', ', $values));

    $condition = ($min > 0) ? "c >= $min && c <= $max" : "c <= $max";

    printf("unsigned unicode_to_bitmap_index(char32_t c)\n".
           "{\n".
           "    return ($condition) ? trans[c-$min] : 0;\n".
           "}\n");
    /*
    $p = proc_open('./constablecom',
                   [0=>['pipe','r'], 1=>['pipe','w'], 2=>['file', 'php://stderr', 'w']],
                   $pipes);
    fwrite($pipes[0], $table);
    fclose($pipes[0]);
    print stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    proc_close($p);
    */
  }
};
