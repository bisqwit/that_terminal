<?php
require '../read_font.php';

foreach(Array(437,850,852,860,863,865) as $enc)
foreach(Array(8,14,16) as $size)
{
  $asm = "{$enc}-8x{$size}.asm";
  $inc = "{$enc}-8x{$size}.inc";
  $data = Read_Font($asm);
  $s   = "// Converted from $asm (or from EGA.CPI)\n";
  foreach($data as $line)
  {
    foreach($line as $w) $s .= sprintf("0x%02X,", $w);
    $s .= "\n";
  }
  file_put_contents($inc, $s);
  touch($inc, strtotime('1987-28-05 10:00'));
}
