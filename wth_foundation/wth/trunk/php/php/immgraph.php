<?php

function convfunc($valtoconv,$convtowhat)
{

     if($valtoconv == 0) return 0;
     if(strlen($valtoconv) == 1) return $valtoconv;

     switch($convtowhat) {
         case 1:
             $a = $valtoconv / 20.0;
             $b = pow(10,$a);
             $c = $b*sqrt(8);
             $x = $c;
            error_log("a = $a, b = $b, c = $c");
            return $x;
         case 2:
             $a = $valtoconv / sqrt(8);
            $b = log10($a);
            $c = 20.0 * $b;
            $x = $c;
            error_log("a = $a, b = $b, c = $c");
             return $x;
         default:
             return $valtoconv;
     }

}

     Header("Content-type: image/png");

     $MaxLines = 7;

$prof1 = time();

     function isalnum($arg1) {
         if($arg1 >= 'a' && $arg1 <= 'z') return 1;
         if($arg1 >= 'A' && $arg1 <= 'Z') return 1;
         if($arg1 >= '0' && $arg1 <= '9') return 1;
         if($arg1 == '#' ) return 1;
         return 0;
     }

     if($argc == 0) {
         $png = ImageCreate(600,500);
         $bg = ImageColorAllocate($png,255,255,255);
         $tx = ImageColorAllocate($png,125,125,255);
         ImageString($png,5,200,240,"Datafile not specified",$tx);
         ImagePng($png);
     }

     if($tstgraph == "TEST") {
         $goget = $tstgraphdata;
     }
     else {
         $goget = $argv[0];
     }
// echo("tstgraph = [$tstgraph]\n");

     $trp = split("~",$goget);
$prof2 = time();

     $getfile = "cat /xydata/xydata/".$trp[0];
//     $getfile = "/usr/bin/ssh -l deptdata xydata cat ".$trp[0];
//     $getfile2 = "/usr/bin/ssh -l deptdata xydata cat ".$trp[0];

#     $rawdata = `$getfile`;

error_log($getfile,0);
     exec($getfile,$rawdata);

$prof3 = time();
     $sep = "";

     $line1 = split($sep,$rawdata[0]);
     $line2 = split($sep,$rawdata[1]);

     $datattype = $line1[0];

//error_log("($line1[0]) ($line2[0]) ($line1[1])\n");

     $x = 2;
     $y = -2;

     include("viewtesttypes.inc");
     include("datatypes.inc");
$prof4 = time();

#     $asdf = "asdf";

     include("gettestinfo.inc");
$prof5 = time();

     if(ereg("[[:alpha:]]",$character)) $limitcrv = "";

     $Y1 = $Y2 = $R = $X = $L = -1;

     for($ii=0;$ii<$MaxLines;$ii++) {
         $xx = $line2[$ii];
         if($xx[0] == 'X') {
             $X = $ii;
             $xlabel = $line1[$ii+1];
             $ar = explode("(",$xx);
             $br = explode(")",$ar[1]);
             $xlabel .= " ".$br[0];
         }
         else if($xx[0] == 'L') {
             $L = $ii;
             $llabel = $line1[$ii+1];
             $ar = explode("(",$xx);
             $br = explode(")",$ar[1]);
             $llabel .= " ".$br[0];
         }
         else if($Y1 != -1 && $xx[0] == 'Y') {
             $Y2 = $ii;
             $show[$ii] = 1;
             $ylabel[$ii] = $line1[$ii+1];
         }
         else if($xx[0] == 'Y') {
             if($Y1 == -1) {
                 $Y1 = $ii;
                 $ylabel1 = $line1[$ii+1];
                 $ar = explode("(",$xx);
                 $br = explode(")",$ar[1]);
                 $ylabel1 .= " ".$br[0];
             }
             $show[$ii] = 1;
             $ylabel[$ii] = $line1[$ii+1];
         }
         else if($xx[0] == 'R') {
             $show[$ii] = 0;
             $R = $ii;
         }
         else $show[$ii] = 0;
     }

     if($Y2 >= 0) {
         if($ylabel[$Y1] == "LIMIT" ||
           $ylabel[$Y1] == "Limit") {
             $tmpp = $Y1;
             $Y1 = $Y2;
             $Y2 = $tmpp;
         }
     }

     if($y == -2) {
         $png = ImageCreate(600,500);
         $bg = ImageColorAllocate($png,255,255,255);
         $tx = ImageColorAllocate($png,125,125,255);
         ImageString($png,5,200,240,"No data available",$tx);
         ImagePng($png);
     }

// convert y axis stuff here.

     $cnv = 0;

     if($testnick == "AFT") {
         if(strtoupper($levunits) == "DBVRMS")
             $limitcrv = convfunc($limitcrv,2);
//         $ylabel1 = "Vpp";
     }

     while($rawdata[$x] != "") {
         $dataline = split($sep,$rawdata[$x]);
         $dv[0][$y] = $dataline[0]; // assuming X is first
         $dv[1][$y] = convfunc($dataline[1],$cnv);
         $dv[2][$y] = convfunc($dataline[2],$cnv);
         $dv[3][$y] = convfunc($dataline[3],$cnv);
         $dv[4][$y] = $dataline[4];
         $dv[5][$y] = $dataline[5];
         $dv[6][$y] = $dataline[6];
         $yippee = "$y: $dataline[0] $dataline[1] $dataline[2] $dataline[3] $dataline[4] $dataline[5] $dataline[6]";
         error_log($yippee,0);
         $x++;
         $y++;
     }

$prof6 = time();
     $numpoints = $y;

     $WIDTH = 600;
     $HEIGHT = 500;
     $VBORDER = 80;
     $HBORDER = 160;

     $png = ImageCreate($WIDTH,$HEIGHT);
     $bg = ImageColorAllocate($png,255,255,255);
     $tx = ImageColorAllocate($png,125,125,255);
//     ImageString($png,1,0,0,"$goget $argv[0] $y points",$tx);
//     ImagePng($png);
//     die("");

// ImageString($png,2,150,380,"hello!!".$getfile2,$tx);
   $white = ImageColorAllocate($png, 255, 255, 255);
   $black = ImageColorAllocate($png, 0, 0, 0);
   $red = ImageColorAllocate($png, 255, 0, 0);
   $green = ImageColorAllocate($png, 0, 190, 0); //255 is too bright for green
   $blue = ImageColorAllocate($png, 0, 0, 255);
   $yellow = ImageColorAllocate($png, 0xe0, 0xfc, 0x18);

   $color[0] = $green;
   $color[1] = $blue;
   $color[2] = $red;
   $color[3] = $yellow;
   $color[4] = $black;

     for($ii=0;$ii<$MaxLines;$ii++) {
         error_log("$ii: $ylabel[$ii]");
         if($ylabel[$ii] == "Limit" ||
           $ylabel[$ii] == "LIMIT") {
             $col[$ii] = $yellow;
         }
         else {
             $col[$ii] = $green;
         }
     }

     $fheight1 = imagefontheight(1);
     $fheight2 = imagefontheight(2);
     $fheight3 = imagefontheight(3);
     $fheight4 = imagefontheight(4);
     $fheight5 = imagefontheight(5);

     $fwidth1 = imagefontwidth(1);
     $fwidth2 = imagefontwidth(2);
     $fwidth3 = imagefontwidth(3);
     $fwidth4 = imagefontwidth(4);
     $fwidth5 = imagefontwidth(5);


     $gtop = ($VBORDER / 2);
     $gbot = $HEIGHT - $VBORDER - 100;
     $gleft = ($HBORDER / 2);
     $gright = $WIDTH - ($HBORDER / 2);

     if($X >= 0) $xlo = $xhi = $dv[$X][0];
     if($Y1 >= 0) $ylo = $yhi = $dv[$Y1][0];
     if($Y2 >= 0) $y2lo = $y2hi = $dv[$Y2][0];

     $hivswr = 0;

     for($x = 0; $x < $numpoints;$x++) {
         if($X > -1 && $xlo > $dv[$X][$x])
             $xlo = $dv[$X][$x];
         if($X > -1 && $xhi < $dv[$X][$x])
             $xhi = $dv[$X][$x];
         if($Y1 > -1 && $ylo > $dv[$Y1][$x])
             $ylo = $dv[$Y1][$x];
         if($Y1 > -1 && $yhi < $dv[$Y1][$x])
             $yhi = $dv[$Y1][$x];
         if($Y2 > -1 && $y2lo > $dv[$Y2][$x])
             $y2lo = $dv[$Y2][$x];
         if($Y2 > -1 && $y2hi < $dv[$Y2][$x])
             $y2hi = $dv[$Y2][$x];
         if($vswr) {
             if($hivswr < $dv[$vswr][$x])
                 $hivswr = $dv[$vswr][$x];
         }
         $yp1 = $dv[$X][$x];
         $yp2 = $dv[$Y1][$x];
         $yp3 = $dv[$Y2][$x];
         $yippee = "yp: $yp1 $yp2 $yp3 ";
         error_log($yippee,0);

     }

     if($limitcrv > 0
      && $yhi < $limitcrv)
         $yhi = $limitcrv;

$prof7 = time();

error_log("prof7: $prof7",0);
//     if(($xhi-$xlo) == 0) die("");
//     if(($yhi-$ylo) == 0) die("");
//     if(($y2hi-$y2lo) == 0) die("");


     $ghi = (int)(($yhi > $y2hi) ? $yhi : $y2hi);
     $glo = (int)(($ylo < $y2lo) ? $ylo : $y2lo);

     if($glo % 10 && $testnick != "LFC") {
        $glo -= ($glo % 10);
     }

     if($ghi < 10) {
         $ghi += 1;
         $yticks = 5;
     }
     else {
         if($ghi % 10) {
             $ghi -= ($ghi % 10);
         }
         $ghi += 10;
         $yticks = 10;
     }

     if($glo > 0) $glo = 0;

     if(($xhi-$xlo) == 0) {
         $xmult = ($gright-$gleft)/$xhi;
         $aaa = "Start and stop X-axis datapoints are equal";
         $x = 300-(($fwidth2/2)*strlen($aaa));
         $y = ($gtop - ($fheight2 + 3)) + 50;
         ImageString($png,2,$x,$y,$aaa,$red);


     }
     else {
         $xmult = ($gright-$gleft)/($xhi-$xlo);
     }

     $ymult = ($gbot-$gtop)/($ghi-$glo);

     if($vswr) {
         $dd = 2;
         while((int)$hivswr > $dd)
             $dd++;
         $vwmult = ($gbot-$gtop)/($dd);
     }


     // Graph Border
   ImageLine($png,$gleft,$gtop,$gleft,$gbot,$black);
   ImageLine($png,$gleft,$gbot,$gright,$gbot,$black);
   ImageLine($png,$gright,$gbot,$gright,$gtop,$black);
   ImageLine($png,$gright,$gtop,$gleft,$gtop,$black);

     $y1 = 0;

     // Vertical Gridlines
   for($ii = $gleft;$ii <= $gright;$ii += (($gright-$gleft)/8)) {
      ImageLine($png,$ii,$gtop,$ii,$gbot,$black);
      if($xhi < 10 || (($xhi-$xlo) < 50.0))
         $buf = sprintf("%.3f",((($xhi-$xlo)/8.0)*(float)$y1)+$xlo);
      else $buf = sprintf("%.0f",((($xhi-$xlo)/8.0)*(float)$y1)+$xlo);
      $x = $ii - (($fwidth2 * strlen($buf)) / 2);
      ImageString($png,2,$x,$gbot+5,$buf,$black);
      $y1++;
   }

   $y1 = ($fwidth2/2) * strlen($xlabel);
   ImageString($png,2,($WIDTH/2)-$y1,$gbot+30,$xlabel,$black);

   ImageStringUp($png,2,($gleft - 50),
     (($HEIGHT-$gtop-100)/2)+(($fwidth2/2)
                         * strlen($ylabel1)),$ylabel1,$black);

     if($vswr) {
         ImageStringUp($png,2,($gright + 35),
             (($HEIGHT-$gtop-100)/2)+(($fwidth2/2)
                                      * strlen("VSWR")),"VSWR",$blue);
     }

     // Horizontal Gridlines
     $y2 = $yticks;
     for($ii = $gtop;$ii <= $gbot;$ii += (($gbot-$gtop)/$yticks)) {
         ImageLine($png,$gleft,$ii,$gright,$ii,$black);
         $yf1 = (float)(((float)($ghi - $glo) / (float)$yticks) * (float)$y2);
         $yf1 += (float)$glo;
         if($ghi < 10.0)
             $buf = sprintf("%.2f",$yf1);
         else
             $buf = sprintf("%.0f",$yf1);
         ImageString($png, 1,($gleft-(strlen($buf) * $fwidth1))-2,
                      $ii-($fheight1/2),$buf,$black);
         if($vswr) {
             $yf1 = (float)(((float)($dd) / (float)$yticks) * (float)$y2);
             $buf = sprintf("%.2f",$yf1);
             ImageString($png, 1, $gright+2,
                  $ii-($fheight1/2),$buf,$blue);
         }
         $y2--;
     }


//error_log("Limitcrv: $limitcrv");
//error_log("Levunits: $levunits");

     // Limit Line
     if($limitcrv > 0) {
         $a = (int)(($limitcrv - (float)$glo) * $ymult);
        if($a < 0) $a = -$a;
        if($b < 0) $b = -$b;
         $y1 = ($gbot - $a);
         ImageLine($png,$gleft,$y1,$gright,$y1,$yellow);
         ImageLine($png,$gleft,$y1+1,$gright,$y1+1,$yellow);
         ImageLine($png,$gleft,$y1-1,$gright,$y1-1,$yellow);
         ImageString($png,4,$gright+5,$y1-($fheight4/2),"Limit",$yellow);
     }

//error_log("X = $X, Y1 = $Y1, Y2 = $Y2, R = $R, L = $L, C = $C");

     $aaa = "Printed ".strftime("%c");
     $x = 300-(($fwidth2/2)*strlen($aaa));
     $y = $gtop - ($fheight2 + 3);
     ImageString($png,2,$x,$y,$aaa,$red);

     $x = 300-(($fwidth4/2)*strlen($signoff));
     $y = $gtop - (($fheight4 + 3) * 2);
     ImageString($png,4,$x,$y,$signoff,$red);

     for($x = 1; $x < $numpoints;$x++) {
         $x1 = (int)((($dv[$X][$x-1] - $xlo) * $xmult) + $gleft);
         $x2 = (int)((($dv[$X][$x] - $xlo) * $xmult) + $gleft);
         if($L > -1) {
             $a1 = (int)(($dv[$L][$x-1] - (float)$glo) * $ymult);
             $b1 = (int)(($dv[$L][$x] - (float)$glo) * $ymult);
             if($a1 < 0) $a1 = -$a1;
             if($b1 < 0) $b1 = -$b1;
             $y11 = ($gbot - $a1);
             $y12 = ($gbot - $b1);
             ImageLine($png,$x1,$y11,$x2,$y12,$yellow);
         }
         if($Y2 > -1) {
             $a1 = (int)(($dv[$Y2][$x-1] - (float)$glo) * $ymult);
             $b1 = (int)(($dv[$Y2][$x] - (float)$glo) * $ymult);
             if($a1 < 0) $a1 = -$a1;
             if($b1 < 0) $b1 = -$b1;
             $y11 = ($gbot - $a1);
             $y12 = ($gbot - $b1);
             if($dv[$C][$x] == '!') ImageLine($png,$x1,$y11,$x2,$y12,$col[$Y2]);
             else if($dv[$C][$x] == '"') ImageLine($png,$x1,$y11,$x2,$y12,$red);
             else if(isalnum($dv[$C][$x]) && $R >= 0) {
                 $y3 = (int)($gbot - ($dv[$R][$x] * $ymult));
                 // the next two lines center the code and
                 // stop the bird dropping just short of it.
                 $lodrop = (($y12 + 8) > $gbot) ? $gbot : ($y12 + 8);
                 if($ylabel[$Y2] == "Limit" ||
                   $ylabel[$Y2] == "LIMIT") {
                     // don't do a damn thing here!
                 }
                 else {
                     ImageDashedLine($png,$x2,$lodrop,$x2,$y3,$red);
                     if($dv[$C][$x] != '#')
                         ImageString($png, 2,$x2-2,$y12,$dv[$C][$x],$red);
                 }
             }
         }
         if($vswr) {
             $vw1 = $gbot - ((int)(($dv[$vswr][$x-1] - (float)$glo) * $vwmult));
             $vw2 = $gbot - ((int)(($dv[$vswr][$x] - (float)$glo) * $vwmult));
             ImageLine($png,$x1,$vw1,$x2,$vw2,$blue);
         }
        $a = (int)(($dv[$Y1][$x-1] - (float)$glo) * $ymult);
        $b = (int)(($dv[$Y1][$x] - (float)$glo) * $ymult);
        if($a < 0) $a = -$a;
        if($b < 0) $b = -$b;
         $y1 = ($gbot - $a);
         $y2 = ($gbot - $b);
         $CC = $dv[$C][$x];
//error_log("y1 = $y1, y2 = $y2, x1 = $x1, x2 = $x2, ($CC)");
         if($CC == '!') ImageLine($png,$x1,$y1,$x2,$y2,$col[$Y1]);
         else if($dv[$C][$x] == '"') ImageLine($png,$x1,$y1,$x2,$y2,$red);
         else if(isalnum($dv[$C][$x]) && $R >= 0) {
            $y3 = (int)($gbot - ($dv[$R][$x] * $ymult));
             // the next two lines center the code and
             // stop the bird dropping just short of it.
             $lodrop = (($y2 + 8) > $gbot) ? $gbot : ($y2 + 8);
            ImageDashedLine($png,$x2,$lodrop,$x2,$y3,$red);
             if($dv[$C][$x] != '#')
                 ImageString($png, 2,$x2-2,$y2,$dv[$C][$x],$blue);
         }
         if($cnv == 1) error_log("cnv ($Y1:$x = $dv[$Y1][$x])");

     }

     if($Y2 >= 0) {
         $labbot = $gbot + 45;
         $labrow = 0;

         $lab = 170;

         ImageLine($png,$lab,$labbot,$lab+25,$labbot,$col[$Y1]);
         ImageString($png,3,$lab+35,$labbot-6,$ylabel[$Y1],$col[$Y1]);

         $lab = 370;

         ImageLine($png,$lab,$labbot,$lab+25,$labbot,$col[$Y2]);
         ImageString($png,3,$lab+35,$labbot-6,$ylabel[$Y2],$col[$Y2]);
     }

     if($L >= 0) {
         $labbot = $gbot + 45;
         $labrow = 0;

         $lab = 170;

         ImageLine($png,$lab,$labbot,$lab+25,$labbot,$col[$Y1]);
         ImageString($png,3,$lab+35,$labbot-6,$ylabel[$Y1],$col[$Y1]);

         $lab = 370;

         ImageLine($png,$lab,$labbot,$lab+25,$labbot,$yellow);
         ImageString($png,3,$lab+35,$labbot-6,$llabel,$yellow);
     }

     $ii = 0;
     $yy = 0;

     while($effid[$ii]) {
         if($ii == 0) {
             $y1 = ($fwidth3/2) * strlen("Effects");

//             ImageString($png,3,($WIDTH/2)-$y1,365,"Effects",$black);
             ImageString($png,3,$gleft,380,"ID",$black);
             $xyz = ((455-205)/2) + $gleft+45;
             $y1 = $xyz - ($fwidth3/2) * strlen("Description");
             ImageString($png,3,$y1,380,"Description",$black);
             ImageLine($png,$gleft,395,400,395,$black);
         }
         if($lasteff != $effid[$ii]) {
             $lasteff = $effid[$ii];
             ImageString($png,2,$gleft,400+($yy*15),$effid[$ii],$black);
             ImageString($png,2,$gleft+45,400+($yy*15),$effdef[$ii],$black);
             $yy++;
         }
         $ii++;
     }

error_log("ending, but the next one is the image return status",0);

/*     ImageString($png,2,0,400+(1*15),$prof1,$black);
     ImageString($png,2,0,400+(2*15),$prof2,$black);
     ImageString($png,2,0,400+(3*15),$prof3,$black);
     ImageString($png,2,0,400+(4*15),$prof4,$black);
     ImageString($png,2,150,400+(1*15),$prof5,$black);
     ImageString($png,2,150,400+(2*15),$prof6,$black);
     ImageString($png,2,150,400+(3*15),$prof7,$black);
     ImageString($png,2,150,400+(1*15),$asdf,$black);
     ImageString($png,2,150,400+(2*15),"hi",$black); */

     $aa = ImagePng($png);

     $aaa = "returns: ".$aa;

error_log($aaa,0);

?>


