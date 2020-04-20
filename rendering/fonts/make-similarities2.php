<?php

$results = Array(); // Pairs of "what to generate", "recipe"
foreach(explode("\n", file_get_contents('similarities.dat')) as $line)
{
  if(strlen($line) == 0) continue;
  $codes = unpack('N*',iconv('utf8','ucs4',$line));
  $codes = array_values($codes); // Renumber values so that 0 is the first one
  $a = 2;
  $b = count($codes);
  switch($codes[0])
  {
    case 0x2192: // →
    {
      // Generate symbol on left from symbol on right
      $n=$a;
      for($m=$n+1; $m<$b; ++$m)
        $results[] = Array($codes[$n], $codes[$m]);
      break;
    }
    case 0x2190: // ←
    {
      // Generate symbol on right from symbol on left
      for($n=$b; $n-- >= $a; )
        for($m=$n; --$m >= $a; )
          $results[] = Array($codes[$n], $codes[$m]);
      break;
    }
    case 0x25C6: // ◆
    {
      // Generate any symbol from any other,
      // with preference for close-by one
      $score = Array();
      for($n=$a; $n<$b; ++$n)
      {
        for($m=$a; $m<$b; ++$m)
          if($m != $n)
            $score[abs($n-$m)][] = Array($n,$m);
      }
      ksort($score);
      foreach($score as $code=>$pairs)
        foreach($pairs as $pair)
          $results[] = Array($codes[$pair[0]], $codes[$pair[1]]);
      break;
    }
    case 0x003D: // =
    {
      // Generate any symbol from any other symbol
      for($n=$a; $n<$b; ++$n)
        for($m=$a; $m<$b; ++$m)
          if($m != $n)
            $results[] = Array($codes[$n], $codes[$m]);
      break;
    }
  }
}
print json_encode($results);
