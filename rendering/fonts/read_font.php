<?php

function Read_PSFgzEncoding($filename)
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
  #print "utf8? $utf8 table? $hastable\n";
  $table = Array();
  if($hastable)
  {
    $pos = $offset + $fontlen * $charsize;
    $ReadUC = $utf8
      ? function()use(&$pos,&$data)
        {
          $unichar = ord($data[$pos++]);
          #printf("%02X ", $unichar);
          if($unichar == 0xFE) return 'ss';
          if($unichar == 0xFF) return 'term';
          if($unichar >= 0x80)
          {
            /*
             UTF-8 encodings:
                 7 bits:  0xxxxxxx                              (7)
                 11 bits: 110xxxxx 10xxxxxx                     (5 + 6)
                 16 bits: 1110xxxx 10xxxxxx 10xxxxxx            (4 + 6 + 6)
                 21 bits: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx   (3 + 6 + 6 + 6)
            */
            if(($unichar & 0xF8) == 0xF0) $unichar &= 0x07;     // 3 bits
            elseif(($unichar & 0xF0) == 0xE0) $unichar &= 0x0F; // 4 bits
            elseif(($unichar & 0xE0) == 0xC0) $unichar &= 0x1F; // 5 bits
            else $unichar &= 0x7F;                              // 7 bits

            while($pos < strlen($data) && (ord($data[$pos]) & 0xC0) == 0x80)
            {
              #printf("%02X ", ord($data[$pos]));
              $unichar = ($unichar << 6) | (ord($data[$pos++]) & 0x3F);
            }
          }
          #print "\n";
          return $unichar;
        }
      : function()use(&$pos,&$data)
        {
          $unichar = ord($data[$pos++]);
          $unichar |= ord($data[$pos++]) << 8;
          if($unichar == 0xFFFE) return 'ss';
          if($unichar == 0xFFFF) return 'term';
          return $unichar;
        };
    for($n=0; $n<$fontlen; ++$n)
    {
      // Grammar:
      //   UC* (SS UC+)* TERM
      $list = Array();
      // Single-codepoint alternatives for this glyph
      for(;;)
      {
        $code = $ReadUC();
        if($code === 'term') break;
        if($code === 'ss') break;
        $list[] = $code;
      }
      // Multi-codepoint alternatives for this glyph (for e.g. combining diacritics)
      while($code === 'ss')
      {
        $seq = Array();
        for(;;)
        {
          $code = $ReadUC();
          if($code === 'ss' || $code === 'term') break;
          $seq[] = $code;
        }
        // We ignore all of them if they have more than 1 codepoint.
        if(count($seq) == 1) $list[] = $seq[0]; //else { print 'seq'; print_r($seq); }
      }
      foreach($list as $u)
        $table[$u] = $n;
    }
    return $table;
  }
  #print "no table\n";
  return false;
}

function Read_PSFgz($filename, $width, $height)
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
  $table = Read_PSFgzEncoding($filename);
  #print_r($table);

  $result = Array();
  for($n=0; $n<$fontlen; ++$n)
  {
    $index = Array($n);
    if($table !== false)
    {
      $index = Array();
      foreach($table as $u=>$c) if($c==$n) $index[] = $u;
    }
    $chars = Array(); foreach($index as $u) $chars[] = sprintf('U+%04x', $u);
    printf("// Char 0x%0X: Fills the role of unicode characters %s\n", $n, join(',',$chars));
    foreach($index as $ch)
    {
      $pos = $offset + $n*$charsize;
      $bytesperchar = $charsize/$height;
      for($m=0; $m<$charsize; $m+=$bytesperchar)
      {
        $w = 0;
        # Read big-endian
        for($a=0; $a<$bytesperchar; ++$a) $w |= ord($data[$pos + $m+$bytesperchar-$a-1]) << ($a*8);
        if($width%8)
        {
          #$w <<= ($width%8);
          #$w <<= (8-$width%8);
          $w >>= (8-$width%8);
          #$w >>= ($width%8);
        }
        # Write little-endian
        for($a=0; $a<$bytesperchar; ++$a) $result[$ch][$m+$a] = ($w >> ($a*8)) & 0xFF;
      }
    }
  }
  return $result;
}

$jis208 = Array(
);
$cp437 = Array(
);
$cp850 = Array
(
);
$gb2312 = Array
(
);

function BDFtranslateToUnicode($index, $reg,$enc)
{
  if($index == -1) return 65533; // notdef in ibmfonts
  switch(strtoupper("{$reg}-{$enc}"))
  {
    case 'JISX0208.1990-0':
    {
      global $jis208;
      if(isset($jis208[$index]))
        return $jis208[$index];
      return $index;
    }
    case 'JISX0201.1976-0':
      if($index == 0x5C) return 0xA5;
      if($index == 0x7E) return 0x203E;
      if($index >= 0xA1) return $index - 0xA1 + 0xFF61;
      return $index;

    case 'IBM-CP437':
    {
      global $cp437;
      if(isset($cp437[$index]))
        return $cp437[$index];
      return $index;
    }
    case 'IBM-CP850':
    {
      global $cp850;
      if(isset($cp850[$index]))
        return $cp850[$index];
      return $index;
    }
    
    case 'GB2312.1980-0':
    {
      global $gb2312;
      if(isset($gb2312[$index]))
        return $gb2312[$index];
      return $index;
    }

    case 'ISO10646-1':
    default:
      return $index;
  }
}

function Read_BDF($filename, $width, $height)
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
  $fontwidth  = $width;
  $fontheight = 1;

  $bitmap = Array();
  
  // Ignore width for variable-width fonts
  $ignore_width = false;
  if(preg_match('/monak1[246]|mona6x12|mona7x14/', $filename))
    $ignore_width = true;

  foreach(explode("\n",file_get_contents($filename)) as $line)
  {
    $line = trim($line);
    if(preg_match('/^FONT_ASCENT (.*)/', $line, $mat))
      $ascent = (int)$mat[1];
    elseif(preg_match('/^FONT_DESCENT (.*)/', $line, $mat))
      $descent = (int)$mat[1];
    elseif(preg_match('/^ENCODING (.*)/', $line, $mat))
    {
      $chno = (int)$mat[1];
      $chno = BDFtranslateToUnicode($chno, $registry,$encoding);
    }
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
      #$fontwidth  = (int)$mat[1];
      $fontheight = (int)$mat[2];
      #$matrix_row_size = ($fontwidth + 7) >> 3; // 1 byte
    }
    elseif(preg_match('/^DWIDTH ([0-9]+) ([0-9]+)/', $line, $mat))
    {
      $fontwidth  = (int)$mat[1];
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

      if(preg_match('/gb16st/', $filename) && $width==9) 
      {
        // The autodetected value seems bad, manually override
        $beforebox = 1;
        $afterbox = 0;
      }
      if(preg_match('/gb24st/', $filename) && $width==10) 
      {
        // The autodetected value seems bad, manually override
        $beforebox = 4;
        $afterbox  = 0;
      }
    }
    elseif($line == 'BITMAP')
      $mode = 1;
    elseif($line == 'ENDCHAR')
    {
      $mode = 0;
      if($fontwidth == $width || $ignore_width)
      {
        $map = Array();
        
        
        #print "before $beforebox after $afterbox count ".count($data)."\n";
        
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

        $bytes = ($width + 7) >> 3;
        $sbytes = ($fontwidth + 7) >> 3;
        for($y=0; $y<$fontheight; ++$y)
        {
          $m = (int)@$map[$y];
          for($b=0; $b<$bytes; ++$b)
          {
            $bitmap[$chno][$y*$bytes + $b] = (($m >> (($sbytes*8)-$fontwidth)) >> ($b*8)) & 0xFF;
          }
        }
        ++$chno;
      }

      $data = Array();
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

function Read_Font($filename, $width, $height)
{
  #print "$filename\n";
  preg_match('/\.(psf\.gz|inc|asm|bdf|[^.*])$/', $filename, $mat);
  $ext    = $mat[1];
  if(!file_exists($filename))
    file_put_contents('php://stderr', "File not found: $filename\n");
  switch($ext)
  {
    case 'psf.gz': return Read_PSFgz($filename, $width, $height);
    case 'inc': return Read_Inc($filename, $height);
    case 'asm': return Read_ASM($filename, $height);
    case 'bdf': return Read_BDF($filename, $width, $height);
    default: print "Unknown filename: $filename\n";
  }
}

#foreach(glob('data/*') as $fn)
#  Read_Font($fn);

#Read_Font('data/UbuntuMono-R-8x16.psf.gz', 16);
