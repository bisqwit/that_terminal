<?php
$names = Array();
$codes = Array();

$fp = fopen('UnicodeData.txt', 'r');
while($s = fgets($fp,4096))
  if(preg_match('/^([0-9A-F]+);([^;]+)/', $s, $mat))
  {
    $codes[$mat[2]] = hexdec($mat[1]);
    $names[hexdec($mat[1])] = $mat[2];
  }

function ToUTF8($code)
{
  return iconv('ucs4', 'utf8', pack('N',$code));
}
function SubstFor($code,$name,   $base,$attrs=Array())
{
  global $names,$codes;
  if(count($attrs)) $base .= ' WITH '.join(' AND ',$attrs);
  if(isset($codes[$base]))
  {
    printf("Array(0x%X /*%s*/, 0x%X/*%s*/), // Make %s from %s\n",
      $codes[$base], ToUTF8($codes[$base]),
      $code,         ToUTF8($code),
      $name, $base);
  }
  #else
  #  printf("// Unable to make %s from %s\n", $name, $base);
}

foreach($codes as $name=>$code)
  if(preg_match('/(.*) WITH (.*)/', $name, $mat))
  {
    $base = $mat[1];
    $attrs = explode(' AND ', $mat[2]);
    $len = count($attrs);
    for($n=$len-1; $n>0; --$n)
    {
      $pick = Array();
      $do = function($index,$start)use(&$attrs,&$pick,$n,$len,$code,$name,$base,&$do)
      {
        for($a=$start; $a<$len; ++$a)
        {
          $pick[$index] = $attrs[$a];
          if($index+1 == $n) SubstFor($code,$name, $base,$pick);
          else               $do($index+1, $a+1);
        }
      };
      $do(0, 0);
    }
  }

foreach($codes as $name=>$code)
  if(preg_match('/(.*) WITH (.*)/', $name, $mat))
  {
    $base = $mat[1];
    SubstFor($code,$name, $base);
  }

$words = Array('PARENTHESIZED','CIRCLED','BLACK','WHITE','HEAVY','LIGHT','FULLWIDTH','HALFWIDTH','SMALL');
foreach($codes as $name=>$code)
  if(preg_match('/^('.join('|',$words).') (.*)/', $name, $mat))
    foreach($words as $w)
      if($w != $mat[1])
        SubstFor($code,$name, "$w {$mat[2]}");
foreach($codes as $name=>$code)
  if(preg_match('/^('.join('|',$words).') (.*)/', $name, $mat))
    SubstFor($code,$name, $mat[2]);
