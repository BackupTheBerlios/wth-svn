<?php
/* This code will dynamically generate a gif chart image
   of temperature and humidity weather data vs. time.

   tmp_hum.gif.php

   Jim Yuzwalk
   Sep 02, 2005

*/


/* Make a connection to the database */
require_once('DB.php');
//$db = DB::connect("pgsql://jjy@yuzwalk/weather");
$db = DB::connect("pgsql://jjy@nomad2/weather");
if(DB::iserror($db)){
  echo "Error connecting to db<br>\n";
  die($db->getMessage());
}


/* Create a query for inside temperature and humidity
   data over the last 24 hours. But first 
   get the minimum and maximum values. 
*/
//$sql = "select min(t),max(t),min(tempi),max(tempi),min(humi),max(humi) from weather";
$sql = "select min(t),max(t),min(tempi),max(tempi),min(humi),max(humi) from weather where t >= (now() -interval '24 hours')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting most current weather data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row);

/* Save the minimum and maximum values */
$t_min=$row[0];
$t_max=$row[1];
$tmp_min=$row[2];
$tmp_max=$row[3];
$hum_min=$row[4];
$hum_max=$row[5];

/* Now get the time, temperature and humidity data rows */
//$sql = "select t,tempi,humi from weather";
$sql = "select t,tempi,humi from weather where t >= (now() -interval '24 hours') order by t";

/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting most current weather data from db<br>\n";
  die($q->getMessage());
}

$row_count=0;
while($q->fetchInto($row)){

  /* stuff the data arrays with the data */
  $time[$row_count]=$row[0];
  $tempi[$row_count]=$row[1];
  $humi[$row_count]=$row[2];

  /* count the number of returned rows */
  $row_count+=1;

}//while()

/* report the results (debugging) */
//echo "Processed $row_count rows<br>";
//echo "t_min=$t_min<br>";
//echo "t_max=$t_max<br>";
//echo "tmp_min=$tmp_min<br>";
//echo "tmp_max=$tmp_max<br>";
//echo "hum_min=$hum_min<br>";
//echo "hum_max=$hum_max<br>";

/* set the graph's x and y dimensions */
$gx=800;
$gy=240;


/* Time to create the $gx x $gy image */

/* first creat a blank image */
$im=imageCreate($gx,$gy);

/* use a white background */
$white = imageColorAllocate($im,255,255,255);

/* here's a small pallete to use for the graphs (RGB values) */
$black = imageColorAllocate ($im,0,0,0);
$brown = imageColorAllocate ($im,255,255,0);
$red = imageColorAllocate ($im,255,0,0);
$yellow = ImageColorAllocate($im,224,252,24);
$green = imageColorAllocate ($im,0,255,0);
$drkGreen = imageColorAllocate ($im,0,190,0);
$blue = imageColorAllocate ($im,0,0,255);
$grey = imageColorAllocate ($im,128,128,128);
$ltGrey = imageColorAllocate ($im,192,192,192);
$ltrGrey = imageColorAllocate ($im,224,224,224);


/* experimenting, i.e., debugging */
/* For a 100 x 100 image:
     (0,0) is the upper left hand corner,
     (100,100) is the lower right hand corner.
*/
//imageLine($im,0,0,200,0,$black);
//imageLine($im,200,1,1,100,$red);
/* imageString(image resource, font size, x, y, color) */
//imageString($im,10,20,50,"Hello",$black);

/* keep the data within a bounding box */
$gxb=$gx*.80;
$dx=($gx-$gxb)/2;
$gyb=$gy*.80;
$dy=($gy-$gyb)/2;
//imageRectangle($im,$dx,$dy,$dx+$gxb,$dy+$gyb,$ltGrey);


/* draw the x axis */
imageLine($im,$dx,$gyb+$dy,$dx+$gxb,$gyb+$dy,$black);

/* draw the y axis */
imageLine($im,$dx,$gyb+$dy,$dx,$dy,$black);

/* generate the graph's title */
imageString($im,8,$gx/6,0,"Indoor",$black);
imageString($im,8,$gx/6,0,"       Temp (F)",$red);
imageString($im,8,$gx/6,0,"                and",$black);
imageString($im,8,$gx/6,0,"                    Humidity",$drkGreen);
imageString($im,8,$gx/6,0,"                             vs. Time",$black);

/* place tics along the x axis, put in vertical graph lines,
   and label x axis tics. 
*/
$tics=10;
$ticHt=5;
/* Height of text label -- use this to drop the H:M of every other
   time label by $labHt pixels to improve readibility.
*/
$labHt=5;
$dtic=($row_count-1)/$tics;
$labelW=22;
$flag=false; /* use this to only display every other tic label */
for($i=$dx,$ii=0;$i<=$gxb+$dx;$i+=$gxb/$tics){
  /* x axis tics */
  imageLine($im,$i,$gyb+$dy+$ticHt,$i,$gyb+$dy-$ticHt,$black);
  /* x axis vertical graph lines */
  if($i!=$dx && $i!=$gxb+$dx){
    imageLine($im,$i,$dy,$i,$gyb+$dy-$ticHt,$ltGrey);
  }
  /* x axis labels */
  //if($flag=!$flag){
  if(0){
    imageString($im,8,$i-$labelW,$gyb+$dy+$ticHt,substr($time[$ii],11,5),$black);
  }else{
    imageString($im,8,$i-$labelW,$gyb+$dy+$ticHt+$labHt,substr($time[$ii],11,5),$black);
  }
  $ii+=$dtic;
}

/* place tics along the y axis, put in horizontal graph lines,
   and label y axis tics.
*/
$tics=10;
$ticW=5;
$dtic=($row_count-1)/$tics;
$labelH=10;
$tmp=$tmp_max; /* start with max temp */
$dtmp=($tmp_max-$tmp_min)/$tics;
$flag=false; /* use this to only display every other tic label */
for($i=$dy,$ii=0;$i<=$gyb+$dy;$i+=$gyb/$tics){
  /* y axis tics */
  imageLine($im,$dx-$ticW,$i,$dx+$ticW,$i,$black);
  /* y axis horizontal graph lines */
  if($i+1<$gyb+$dy){
    imageLine($im,$dx+$ticW,$i,$dx+$gxb-$ticW,$i,$ltGrey);
  }  
  /* y axis labels */
  //  if($flag=!$flag){
  imageString($im,5,25,$i-$labelH,sprintf("%+3.1f",$tmp),$red);
  //  }
  $tmp-=$dtmp;
}

/* draw the 2nd y axis */
imageLine($im,$dx+$gxb,$gyb+$dy,$dx+$gxb,$dy,$black);

/* place tics along the 2nd y axis and label humidity */
$tics=10;
$ticW=5;
$dtic=($row_count-1)/$tics;
$labelH=10;
$hum=$hum_max; /* start with max humidity */
$dhum=($hum_max-$hum_min)/$tics;
$flag=false; /* use this to only display every other tic label */
for($i=$dy,$ii=0;$i<=$gyb+$dy;$i+=$gyb/$tics){
  /* y axis2 tics */
  imageLine($im,$dx+$gxb-$ticW,$i,$dx+$gxb+$ticW,$i,$black);
  /* y axis2 labels */
  //  if($flag=!$flag){
  imageString($im,5,$dx+$gxb+2*$ticW,$i-$labelH,sprintf("%3.1f",$hum),$drkGreen);
  //  }
  $hum-=$dhum;
}
 

/* draw the temperature graph */
$scale=$gyb/($tmp_max-$tmp_min);
for($i=0;$i+1<$row_count;$i++){
  $x1=$i*$gxb/$row_count+$dx;
  $y1=($gyb+$dy)-($tempi[$i]-$tmp_min)*$scale;
  $x2=($i+1)*$gxb/$row_count+$dx;
  $y2=($gyb+$dy)-($tempi[$i+1]-$tmp_min)*$scale;
  imageLine($im,$x1,$y1,$x2,$y2,$red);
}

/* draw the humidity graph */
$scale=$gyb/($hum_max-$hum_min);
for($i=0;$i+1<$row_count;$i++){
  $x1=$i*$gxb/$row_count+$dx;
  $y1=($gyb+$dy)-($humi[$i]-$hum_min)*$scale;
  $x2=($i+1)*$gxb/$row_count+$dx;
  $y2=($gyb+$dy)-($humi[$i+1]-$hum_min)*$scale;
  imageLine($im,$x1,$y1,$x2,$y2,$drkGreen);
}
 
/* use gif as the image format */
header("Content-type: image/gif");
imageGIF($im);

/* lastly delete the image from memory */
imageDestroy($im);


?>