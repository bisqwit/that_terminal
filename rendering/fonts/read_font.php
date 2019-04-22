<?php

function Read_PSFgz($filename, $height)
{
  $data = file_get_contents('compress.zlib://'.$filename);
  $header1 = unpack('nmagic/Cmode/Ccsize', $data);
  $header2 = unpack('Nmagic/Vvers/Vhdrsize/Vflags/Vlength/Vcharsize/Vheight/Vwidth', $data);
  if($header1['magic'] == 0x3604)
  {
    // PSF ver1
    $fontlen  = ($header1['mode'] & 1) ? 512 : 256;
    $hastable = ($header1['mode'] & 6);
    $offset   = 4;
    $utf8     = false;
    $charsize = $header1['csize'];
    $width    = 8;
    $height   = $header1['csize'];
  }
  elseif($header2['magic'] == 0x72B54A86)
  {
    // PSF ver2
    $fontlen  = $header2['length'];
    $hastable = $header2['flags'] & 1;
    $utf8     = true;
    $offset   = $header2['hdrsize'];
    $charsize = $header2['charsize'];
    $width    = $header2['width'];
    $height   = $header2['height'];
  }
  //print_r($header1);
  //print_r($header2);
  if(false && $hastable)
  {
    $pos = $offset + $fontlen * $charsize;
    $list  = Array();
    for($n=0; $n<$fontlen; ++$n)
    {
      $inseq = 0;
      $list[$n] = Array();
      $list_last = &$list[$n][0];
      while($pos < strlen($data))
      {
        if($utf8)
        {
          if($data[$pos] == "\xFF") { ++$pos; break; }
          if($data[$pos] == "\xFE") { ++$pos; $inseq = 1; continue; }
          $unichar = ord($data[$pos++]);
          if($unichar >= 128)
          {
            if(($unichar & 0xF8) == 0xF0) $unichar &= 0x07;
            elseif(($unichar & 0xF0) == 0xE0) $unichar &= 0x0F;
            elseif(($unichar & 0xE0) == 0xC0) $unichar &= 0x1F;
            else $unichar &= 0x7F;
            while((ord($data[$pos]) & 0xC0) == 0xC0)
              $unichar = ($unichar << 6) | (ord($data[$pos++]) & 0x3F);
          }
        }
        else
        {
          $unichar = ord($data[$pos++]);
          $unichar |= ord($data[$pos++]) << 8;
          if($unichar == 0xFFFF) break;
          if($unichar == 0xFFFE) { $inseq = 1; continue; }
        }
        print "$n: inseq=$inseq unichar=$unichar\n";
        if($inseq < 2)
        {
          $list[$n][] = Array($unichar);
          $list_last = &$list[count($list[$n])-1];
        }
        else
        {
          $list_last[] = $unichar;
        }
        if($inseq) ++$inseq;
      }
    }
    print_r($list);
  }
  $result = Array();
  for($n=0; $n<$fontlen; ++$n)
  {
    $pos = $offset + $n*$charsize;
    for($m=0; $m<$height; ++$m)
      $result[$n][$m] = ord($data[$pos+$m]);
  }
  return $result;
}
function Read_BDF($filename, $height)
{
  $chno = 0;
  $data = Array();
  $mode = 0;

  $matrix_row_size = (8 + 7) >> 3; // 1 byte
  $ascent = 0;
  $descent = 0;

  $registry = '';
  $encoding = '';
  $encodings = Array();
  $fontwidth  = 8;
  $fontheight = 1;

  $bitmap = Array();

  foreach(explode("\n",file_get_contents($filename)) as $line)
  {
    if(preg_match('/^FONT_ASCENT (.*)/', $line, $mat))
      $ascent = (int)$mat[1];
    elseif(preg_match('/^FONT_DESCENT (.*)/', $line, $mat))
      $descent = (int)$mat[1];
    elseif(preg_match('/^ENCODING (.*)/', $line, $mat))
      $chno = (int)$mat[1];
    elseif(preg_match('/^FONT -.*-([^-]*)-([^-]*)$/', $line, $mat))
    {
      $registry = $mat[1];
      $encoding = $mat[2];
    }
    elseif(preg_match('/^CHARSET_REGISTRY "?([^"]*)/', $line, $mat))
      $registry = $mat[1];
    elseif(preg_match('/^CHARSET_ENCODING "?([^"]*)/', $line, $mat))
      $encoding = $mat[1];
    elseif(preg_match('/^FONTBOUNDINGBOX ([0-9]*) ([0-9]*)/', $line, $mat))
    {
      $fontwidth  = (int)$mat[1];
      $fontheight = (int)$mat[2];
      $matrix_row_size = ($fontwidth + 7) >> 3; // 1 byte
    }
    elseif(preg_match('/^BBX (-?[0-9]+) (-?[0-9]+) (-?[0-9]+) (-?[0-9]+)/', $line, $mat))
    {
      $x = (int) $mat[1];
      $y = (int) $mat[2];
      $xo = (int) $mat[3];
      $yo = (int) $mat[4];
      
      $shiftbits = ($matrix_row_size - (($x + 7) >> 3) ) * 8 - $xo;
      $beforebox = ($ascent - $yo - $y) * $matrix_row_size;
      $afterbox = ($descent + $yo) * $matrix_row_size;
    }
    elseif($line == 'BITMAP')
      $mode = 1;
    elseif($line == 'ENDCHAR')
    {
      $mode = 0;
      $map = Array();

      while($beforebox < 0)
        { array_shift($data); ++$beforebox; }
      while($afterbox < 0)
        { array_pop($data); ++$afterbox; }

      while($beforebox > 0)
        { $map[] = 0; --$beforebox; }
      foreach($data as $v)
        $map[] = $v;
      while($afterbox > 0)
        { $map[] = 0; --$afterbox; }

      #if($fontwidth <= 8 || $chno < 0x80)
        for($y=0; $y<$fontheight; ++$y)
        {
          $m = (int)@$map[$y];
          if($fontwidth > 8)
          {
            $bitmap[$chno][$y + 0          ] = $m & 0xFF;
            $bitmap[$chno][$y + $fontheight] = $m >> 8;
          }
          else
          {
            $bitmap[$chno][$y] = $m >> (8-$fontwidth);
          }
        }

      $data = Array();
      ++$chno;
    }
    elseif($mode)
    {
      $v = intval($line, 16);
      if($shiftbits > 0)
        $v <<= $shiftbits;
      else
        $v >>= -$shiftbits;
      $data[] = $v;
    }
  }
  ksort($bitmap);
  return $bitmap;
}
function Read_Inc($filename, $height)
{
  $data = Array();
  $n=0;
  foreach(file($filename) as $line)
  {
    preg_match_all('/0x[0-9A-F]+/i', $line, $mat);
    foreach($mat[0] as $hex)
    {
      $data[(int)($n/$height)][] = hexdec($hex);
      ++$n;
    }
  }
  #print_r($data);
  return $data;
}
function Read_ASM($filename, $height)
{
  $data = Array();
  $n=0;
  foreach(file($filename) as $line)
    if(preg_match('/db\s+([^;]+)/i', $line, $mat))
    {
      preg_match_all('/[0-9A-F]+/i', $mat[1], $mat);
      foreach($mat[0] as $hex)
      {
        $data[(int)($n/$height)][] = hexdec($hex);
        ++$n;
      }
    }
  #print_r($data);
  return $data;
}

function Read_Font($filename, $height)
{
  #print "$filename\n";
  preg_match('/(?:.*x|\.f|-)0*([0-9]+).(.*)/', $filename, $mat);
  #$height = (int)($mat[1]);
  $ext    = $mat[2];
  switch($ext)
  {
    case 'psf.gz': return Read_PSFgz($filename, $height);
    case 'inc': return Read_Inc($filename, $height);
    case 'asm': return Read_ASM($filename, $height);
    case 'bdf': return Read_BDF($filename, $height);
    default: print "Unknown filename: $filename\n";
  }
}

#foreach(glob('data/*') as $fn)
#  Read_Font($fn);

#Read_Font('data/UbuntuMono-R-8x16.psf.gz', 16);
