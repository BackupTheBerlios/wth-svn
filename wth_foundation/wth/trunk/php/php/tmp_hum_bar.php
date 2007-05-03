<?php
/* Make a connection to the database */
require_once('DB.php');
$db = DB::connect("pgsql://jjy@nomad2/weather");
if(DB::iserror($db)){
  echo "Error connecting to db<br>\n";
  die($db->getMessage());
}


/* Create a query for outside temperature, humidity */
/* and pressure data over the last 24 hours. But first */
/* get the minimum and maximum values. */
//$sql = "select t,bar,tempo,humo from weather where t >= (now() -interval '24 hours') order by t";
$sql = "select min(t),max(t),min(bar),max(bar),min(tempo),max(tempo),min(humo),max(humo) from weather where t >= (now() -interval '24 hours')";
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
$bar_min=$row[2];
$bar_max=$row[3];
$tmp_min=$row[4];
$tmp_max=$row[5];
$hum_min=$row[6];
$hum_max=$row[7];

/* Now get the time, temperature, humidity, and pressure data rows */
$sql = "select t,bar,tempo,humo from weather where t >= (now() -interval '24 hours') order by t";

/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting most current weather data from db<br>\n";
  die($q->getMessage());
}
while($q->fetchInto($row)){

  
  /* count the number of returned rows */
  $row_count+=1;

  /* stuff the data arrays with the data */
  $time[$ii]=$row[0];
  $bar[$ii]=$row[1];
  $tempo[$ii]=$row[2];
  $humo[$ii]=$row[3];

  /* increment the array index */
  $ii++;
}//while()

/* report the results (debugging) */
//echo "Processed $row_count rows<br>";
//echo "t_min=$t_min<br>";
//echo "t_max=$t_max<br>";
//echo "bar_min=$bar_min<br>";
//echo "bar_max=$bar_max<br>";
//echo "tmp_min=$tmp_min<br>";
//echo "tmp_max=$tmp_max<br>";
//echo "hum_min=$hum_min<br>";
//echo "hum_max=$hum_max<br>";

/* set the graph's x and y dimensions */
$gx=400;
$gy=200;


/* Time to create the $gx x $gy image */

/* first creat a blank image */
$im=imageCreate($gx,$gy);

/* here's a small pallete to use for the graphs */
$white = imageColorAllocate ($im, 0xff, 0xff, 0xff);
$black = imageColorAllocate ($im, 0x00, 0x00, 0x00);
$red = imageColorAllocate ($im, 0xff, 0x00, 0x00);
$green = imageColorAllocate ($im, 0x00, 0xff, 0x00);
$blue = imageColorAllocate ($im, 0x00, 0x00, 0xff);

/* use a white background */
imageColorAllocate($im,255,255,255);

/* experimenting, i.e., debugging */
//imageLine($im,1,1,200,100,$black);
//imageLine($im,200,1,1,100,$red);

/* draw the temperature graph */
$scale=$gy/($tmp_max-$tmp_min);
for($i=0;$i<$ii;$i++){
  $x1=$i*$gx/$row_count;
  $y1=($tempo[$i]-$tmp_min)*$scale;
  $x2=($i+1)*$gx/$row_count;
  $y2=($tempo[$i+1]-$tmp_min)*$scale;
  imageLine($im,$x1,$y1,$x2,$y2,$red);
}

/* draw the humidity graph */
$scale=$gy/($hum_max-$hum_min);
for($i=0;$i<$ii;$i++){
  $x1=$i*$gx/$row_count;
  $y1=($humo[$i]-$hum_min)*$scale;
  $x2=($i+1)*$gx/$row_count;
  $y2=($humo[$i+1]-$hum_min)*$scale;
  imageLine($im,$x1,$y1,$x2,$y2,$green);
}

/* draw the barometer graph */
$scale=$gy/($bar_max-$bar_min);
for($i=0;$i<$ii;$i++){
  $x1=$i*$gx/$row_count;
  $y1=($bar[$i]-$bar_min)*$scale;
  $x2=($i+1)*$gx/$row_count;
  $y2=($bar[$i+1]-$bar_min)*$scale;
  imageLine($im,$x1,$y1,$x2,$y2,$blue);
}


  
/* use gif as the image format */
header("Content-type: image/gif");
imageGIF($im);

/* lastly delete the image from memory */
imageDestroy($im);


?>