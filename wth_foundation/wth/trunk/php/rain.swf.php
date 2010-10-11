<?php
/* This code will dynamically generate a swf chart image
   of rain data vs. time.

   rain.swf.php

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


/* Create a query for rainfall
   data over the last 24 hours. But first 
   get the minimum and maximum values. 
*/
//$sql = "select min(t),max(t),min(rain),max(rain) from weather";
$sql = "select min(t),max(t),min(rain),max(rain) from weather where t >= (now() -interval '24 hours')";
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
$rain_min=$row[2];
$rain_max=$row[3];


/* Now get the time and rain data rows */
//$sql = "select t,rain from weather";
$sql = "select t,rain from weather where t >= (now() -interval '24 hours') order by t";

/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting most current weather data from db<br>\n";
  die($q->getMessage());
}

//Reset the row counter
$row_count=0;
//Reset temporary data storage locations
$row[0]=0;
$row[1]=0;
while($q->fetchInto($row)){

  /* stuff the data arrays with the data */
  $time[$row_count]=$row[0];
  $rain[$row_count]=$row[1];

  /* count the number of returned rows */
  $row_count+=1;

}//while()


/* Determine the number of rain counts that occurred within the last 24 hours */
$rain_counts=$rain_max-$rain_min;
/* Calculate the rainfall in inches */
$rain_inch=$rain_counts*.49/38;
/* Round up the rainfall result and save it for the graph */
$rain_ceil=ceil($rain_inch);

/* Convert the rain count data to rain in inches relative */
/* to the minimum rain count */
for($i=0;$i<$row_count;$i++){
  $rain[$i]=($rain[$i]-$rain_min)*.49/38;
}

/* Convert the rain minimum and maximum counts to inches */
/* relative to the minimum count */
$rain_min=0;
if($rain_ceil>0){
  $rain_max=$rain_ceil;
}else{
  $rain_max=1;
}




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
$txt->setColor(0,0,255);
$txt->moveTo($dist_x,$dist_y);
$txt->addString("Rainfall (inches)");
$txt->setColor(0,0,0);
$txt->moveTo($dist_x+135,$dist_y);
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
$tmp=$rain_max; /* start with max rain */
$drain=($rain_max-$rain_min)/$tics;
$flag=false; /* use this to only display every other tic label */
/* use blue text */
$txt->setColor(0,0,255);
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
  $txt->moveTo(50,$i-$labelH);
  $txt->addString(sprintf("%2.1f",$tmp));
  //}
  $tmp-=$drain;
}


/* draw the 2nd y axis */
/* use a 1 pixel width black pen */
$im->setLine(1,0,0,0);
$im->movePenTo($dx+$gxb,$gyb+$dy);
$im->drawLineTo($dx+$gxb,$gyb+$dy);
$im->drawLineTo($dx+$gxb,$dy);



/* place tics along the 2nd y axis and label rain */
$tics=10;
$ticW=5;
$dtic=($row_count-1)/$tics;
$labelH=-2;
$tmp=$rain_max; /* start with max rain */
$drain=($rain_max-$rain_min)/$tics;
$flag=false; /* use this to only display every other tic label */
/* use a 1 pixel width black pen */
$im->setLine(1,0,0,0);
/* use blue text */
$txt->setColor(0,0,255);
for($i=$dy,$ii=0;$i<=$gyb+$dy;$i+=$gyb/$tics){
  /* y axis2 tics */
  $im->movePenTo($dx+$gxb-$ticW,$i);
  $im->drawLineTo($dx+$gxb-$ticW,$i);
  $im->drawLineTo($dx+$gxb+$ticW,$i);
  /* y axis2 labels */
  //if($flag=!$flag){
  $txt->moveTo($dx+$gxb+2*$ticW,$i-$labelH);
  $txt->addString(sprintf("%2.1f",$tmp));
  //}
  $tmp-=$drain;
}
 


/* draw the rain graph */
$scale=$gyb/($rain_max-$rain_min);
/* use a 1 pixel width blue pen */
$im->setLine(1,0,0,255);
for($i=0;$i+1<$row_count;$i++){
  $x1=$i*$gxb/$row_count+$dx;
  $y1=($gyb+$dy)-($rain[$i]-$rain_min)*$scale;
  $x2=($i+1)*$gxb/$row_count+$dx;
  $y2=($gyb+$dy)-($rain[$i+1]-$rain_min)*$scale;
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