<?php
/* This code will dynamically generate a swf chart image
   of temperature and humidity weather data vs. time.

   tmp_hum.swf.php

   Jim Yuzwalk
   Sep 26, 2005

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

/* first create a blank image */
$im=new SWFShape();

/* use a 1 pixel width black pen */
$im->setLine(1,0,0,0);

/* use a white background */
$mve=new SWFMovie();
$mve->setDimension($gx,$gy);
$mve->setBackground(255,255,255);

/* here's a small pallete to use for the graphs (RGB values) */
//black = (0,0,0);
//brown = (255,255,0);
//$red = (255,0,0);
//$yellow = (224,252,24);
//$green = (0,255,0);
//$drkGreen = (0,190,0);
//$blue = (0,0,255);
//$grey = (128,128,128);
//$ltGrey = (192,192,192);
//$ltrGrey = (224,224,224);


/* experimenting, i.e., debugging */
/* For a 100 x 100 image:
     (0,0) is the upper left hand corner,
     (100,100) is the lower right hand corner.
*/

/* keep the data within a bounding box */
$gxb=$gx*.80;
$dx=($gx-$gxb)/2;
$gyb=$gy*.80;
$dy=($gy-$gyb)/2;


/* draw the x axis */
$im->movePenTo($dx,$gyb+$dy);
$im->drawLineTo($dx,$gyb+$dy);
$im->drawLineTo($dx+$gxb,$gyb+$dy);


/* draw the y axis */
$im->movePenTo($dx,$gyb+$dy);
$im->drawLineTo($dx,$gyb+$dy);
$im->drawLineTo($dx,$dy);


/* generate the graph's title */
/* first get the font */
$fontsize=18;
/* set the distance, from the left, where the title will begin */
$dist_x=$dx+30;
/* set the distance, from the top, where the title will begin */
$dist_y=15;
/* create the font object and text object */
//$fnt=new SWFFont("Courier_New.fdb");
$fnt=new SWFFont("Arial.fdb");
$txt=new SWFText();
/* build the title */
$txt->setFont($fnt);
$txt->setHeight($fontsize);
$txt->setColor(0,0,0);
$txt->moveTo($dist_x,$dist_y);
$txt->addString("Indoor");
$txt->setColor(255,0,0);
$txt->moveTo($dist_x+60,$dist_y);
$txt->addString("Temp (F)");
$txt->setColor(0,0,0);
$txt->moveTo($dist_x+145,$dist_y);
$txt->addString("and");
$txt->setColor(0,190,0);
$txt->moveTo($dist_x+185,$dist_y);
$txt->addString("Humidity");
$txt->setColor(0,0,0);
$txt->moveTo($dist_x+265,$dist_y);
$txt->addString("vs. Time");


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
$labelSize=14;
$txt->setHeight($labelSize);
$labelW=17;
$flag=false; /* use this to only display every other tic label */
for($i=$dx,$ii=0;$i<=$gxb+$dx;$i+=$gxb/$tics){
  /* x axis tics */
  /* use a 1 pixel width black pen */
  $im->setLine(1,0,0,0);
  $im->movePenTo($i,$gyb+$dy+$ticHt);
  $im->drawLineTo($i,$gyb+$dy+$ticHt);
  $im->drawLineTo($i,$gyb+$dy-$ticHt);
  /* x axis vertical graph lines */
  if($i!=$dx && $i!=$gxb+$dx){
    /* use a 1 pixel width light grey pen */
    $im->setLine(1,192,192,192);
    $im->movePenTo($i,$dy);
    $im->drawLineTo($i,$dy);
    $im->drawLineTo($i,$gyb+$dy-$ticHt);
  }
  /* x axis labels */
  //if($flag=!$flag){
  if(0){
    $txt->setColor(0,0,0);
    $txt->moveTo($i-$labelW,$gyb+$dy+$ticHt+10);
    $txt->addString(substr($time[$ii],11,5));
    
  }else{
    $txt->setColor(0,0,0);
    $txt->moveTo($i-$labelW,$gyb+$dy+$ticHt+$labHt+10);
    $txt->addString(substr($time[$ii],11,5));
  }
  $ii+=$dtic;
}



/* place tics along the y axis, put in horizontal graph lines,
   and label y axis tics.
*/
$tics=10;
$ticW=5;
$dtic=($row_count-1)/$tics;
$labelH=-2;
$tmp=$tmp_max; /* start with max temp */
$dtmp=($tmp_max-$tmp_min)/$tics;
$flag=false; /* use this to only display every other tic label */
/* use red text */
$txt->setColor(255,0,0);
for($i=$dy,$ii=0;$i<=$gyb+$dy;$i+=$gyb/$tics){
  /* y axis tics */
  /* use a 1 pixel width black pen */
  $im->setLine(1,0,0,0);
  $im->movePenTo($dx-$ticW,$i);
  $im->drawLineTo($dx-$ticW,$i);
  $im->drawLineTo($dx+$ticW,$i);
  /* y axis horizontal graph lines */
  if($i+1<$gyb+$dy){
    /* use a 1 pixel width light grey pen */
    $im->setLine(1,192,192,192);    
    $im->movePenTo($dx+$ticW,$i);
    $im->drawLineTo($dx+$ticW,$i);
    $im->drawLineTo($dx+$gxb-$ticW,$i);
  }  
  /* y axis labels */
  //if($flag=!$flag){
  $txt->moveTo(35,$i-$labelH);
  $txt->addString(sprintf("%+3.1f",$tmp));
  //}
  $tmp-=$dtmp;
}


/* draw the 2nd y axis */
/* use a 1 pixel width black pen */
$im->setLine(1,0,0,0);
$im->movePenTo($dx+$gxb,$gyb+$dy);
$im->drawLineTo($dx+$gxb,$gyb+$dy);
$im->drawLineTo($dx+$gxb,$dy);



/* place tics along the 2nd y axis and label humidity */
$tics=10;
$ticW=5;
$dtic=($row_count-1)/$tics;
$labelH=-2;
$hum=$hum_max; /* start with max humidity */
$dhum=($hum_max-$hum_min)/$tics;
$flag=false; /* use this to only display every other tic label */
/* use a 1 pixel width black pen */
$im->setLine(1,0,0,0);
/* use dark green text */
$txt->setColor(0,190,0);
for($i=$dy,$ii=0;$i<=$gyb+$dy;$i+=$gyb/$tics){
  /* y axis2 tics */
  $im->movePenTo($dx+$gxb-$ticW,$i);
  $im->drawLineTo($dx+$gxb-$ticW,$i);
  $im->drawLineTo($dx+$gxb+$ticW,$i);
  /* y axis2 labels */
  //if($flag=!$flag){
  $txt->moveTo($dx+$gxb+2*$ticW,$i-$labelH);
  $txt->addString(sprintf("%3.1f",$hum));
  //}
  $hum-=$dhum;
}
 


/* draw the temperature graph */
$scale=$gyb/($tmp_max-$tmp_min);
/* use a 1 pixel width red pen */
$im->setLine(1,255,0,0);
for($i=0;$i+1<$row_count;$i++){
  $x1=$i*$gxb/$row_count+$dx;
  $y1=($gyb+$dy)-($tempi[$i]-$tmp_min)*$scale;
  $x2=($i+1)*$gxb/$row_count+$dx;
  $y2=($gyb+$dy)-($tempi[$i+1]-$tmp_min)*$scale;
  $im->movePenTo($x1,$y1);
  $im->drawLineTo($x1,$y1);
  $im->drawLineTo($x2,$y2);
}


/* draw the humidity graph */
$scale=$gyb/($hum_max-$hum_min);
/* use a 1 pixel width dark green pen */
$im->setLine(1,0,190,0);
for($i=0;$i+1<$row_count;$i++){
  $x1=$i*$gxb/$row_count+$dx;
  $y1=($gyb+$dy)-($humi[$i]-$hum_min)*$scale;
  $x2=($i+1)*$gxb/$row_count+$dx;
  $y2=($gyb+$dy)-($humi[$i+1]-$hum_min)*$scale;
  $im->movePenTo($x1,$y1);
  $im->drawLineTo($x1,$y1);
  $im->drawLineTo($x2,$y2);
}
 

/* add our swf shape to the swf movie */
$i1 = $mve->add($im);
$i2 = $mve->add($txt);

/* output the flash content */
header('Content-type: application/x-shockwave-flash');
$mve->output(0);

?>