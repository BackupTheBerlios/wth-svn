<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>Nomad2 Weather Info:</title>

<script language="JavaScript1.1">


<!-- Hide

// -->
</script>

</head>
<body bgcolor="white" link="blue" vlink="red">

<center>
<h1>Nomad2 Weather Data:</h1>

<?php
#
# Jim Yuzwalk, September 9, 2005
# weather.php
# version 1.3
#
# This php script will create a page showing various weather related
# data gathered from a LaCrosse data logger and stored in a postgres
# database.
#
# History
#
# v1.0 August 27, 2005
# v1.1 August 31, 2005   - Added gd support to generate live graphs of
#                          weather data.
# v1.2 September 2, 2005 - Added wind direction compass headings to data
#                          table display
# v1.3 September 3, 2005 - Added multiple needle or pointer capability
#                          to wind speed and direction.  Gave thermometer
#                          and hydrometers two pointers - one for 
#                          indoor reading, the other for outdoor readings.
#                          Barometer now sports a bargraph trend indicator
#                          and a prior history pointer.
# v1.4 September 9, 2005 - Improved most recent data query by increasing
#                          time span of data request and ordering the 
#                          result by t in descending order.
# v1.5 September 26, 2005- Added graphical display of rain data
# 

/* Make a connection to the database */
require_once('DB.php');
//$db = DB::connect("pgsql://jjy@yuzwalk/weather");
$db = DB::connect("pgsql://jjy@nomad2/weather");
if(DB::iserror($db)){
  echo "Error connecting to db<br>\n";
  die($db->getMessage());
}

/* Create a query for the most current weather data */
//$sql = "select tempi,humi,bar,tempo,humo,winds,windd,rain,t from weather";
$sql = "select tempi,humi,bar,tempo,humo,winds,windd,rain,t from weather where t >= (now() -interval '6 minutes') order by t desc";

/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting most current weather data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row);

/* Check that we have received data from the query */
/* If not, do another query */
if($row[0]==""){
  $sql = "select tempi,humi,bar,tempo,humo,winds,windd,rain,t from weather where t >= (now() -interval '15 minutes') order by t desc";
  $q = $db->query($sql);
  if(DB::iserror($q)) {  
    echo "Error getting most current weather data from db<br>\n";
    die($q->getMessage());
  }
  $q->fetchInto($row);
}

/* Save the query results */
$tempi=$row[0];
$humi=$row[1];
$bar=$row[2]*.03882; /* Convert barometric pressure to inHg */
$bar=sprintf("%2.2f",$bar); /* Only show two significant digits for pressure */
$tempo=$row[3];
$humo=$row[4];
$winds=$row[5];
$windd=$row[6];
$rain=$row[7];
$time=$row[8];

/* Determine the wind direction compass heading */
if($windd <= 23 || $windd >= 337){
  $wndComp="N";
}else if($windd <= 67){
  $wndComp="NE";
}else if($windd <= 113){
  $wndComp="E";
}else if($windd <= 157){
  $wndComp="SE";
}else if($windd <= 203){
  $wndComp="S";
}else if($windd <= 247){
  $wndComp="SW";
}else if($windd <= 293){
  $wndComp="W";
}else{
  $wndComp="NW";
}




/* Do query for 1hr rain data */
$sql = "select rain,t from weather where t >= (now() -interval '1 hours 2 minutes') and t < (now() -interval '58 minutes')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 1hr rain data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row1);
if($row1[0]){
  $rain_1hr=($rain-$row1[0])*.49/38;
  $rain_1hr=sprintf("%2.3f",$rain_1hr);
}else{
  $rain_1hr="N/A";
}

if($row1[1]){
  $time_1hr=$row1[1];
}else{
  $time_1hr="N/A";
}


/* Do query for 24hr rain data */
$sql = "select rain,t from weather where t >= (now() -interval '24 hours 2 minutes') and t < (now() -interval '23 hours 58 minutes')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 24hr rain data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row2);
if($row2[0]){
  $rain_24hr=($rain-$row2[0])*.49/38;
  $rain_24hr=sprintf("%2.3f",$rain_24hr);
}else{
  $rain_24hr="N/A";
}

if($row2[1]){
  $time_24hr=$row2[1];
}else{
  $time_24hr="N/A";
}



/* Do query for 24hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '24 hours') and t < (now() -interval '23 hours')";
/* First initialize all data to 0 */
$row3[0]=0;
$bar24=$bar20=$bar16=$bar12=$bar08=$bar04=$bar01=$bar00=0;

/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 24hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar24=$row3[0];

/* Do query for 20hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '20 hours') and t < (now() -interval '19 hours')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 20hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar20=$row3[0];

/* Do query for 16hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '16 hours') and t < (now() -interval '15 hours')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 16hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar16=$row3[0];

/* Do query for 12hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '12 hours') and t < (now() -interval '11 hours')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 12hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar12=$row3[0];

/* Do query for 8hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '8 hours') and t < (now() -interval '7 hours')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 8hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar08=$row3[0];

/* Do query for 4hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '4 hours') and t < (now() -interval '3 hours')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 4hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar04=$row3[0];

/* Do query for 1hr barometer data */
$sql = "select bar from weather where t >= (now() -interval '1 hours') and t < (now() -interval '30 minutes')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 1hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar01=$row3[0];

/* Do query for 4min barometer data */
$sql = "select bar from weather where t >= (now() -interval '4 minutes')";
/* Do the query thus created */
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$bar00=$row3[0];



echo "<table bgcolor=\"skyblue\" border=\"2\">";
echo "<caption><b>Current Outdoor Readings:</b></caption>";
echo "<tr><th>Time</th><th>Temp</th><th>Humidity</th><th>Wind Speed</th><th>Wind Direction</th><th>Rain 1hr</th><th>Rain 24hr</th></tr>\n";
echo "<tr><td align=\"center\">$time</td><td align=\"center\">$tempo &deg;F</td><td align=\"center\">$humo%</td><td align=\"center\">$winds mph</td><td align=\"center\">$windd&deg; $wndComp</td><td align=\"center\">$rain_1hr</td><td align=\"center\">$rain_24hr</td></tr>\n";
echo "</table>\n";

echo "<br>";

echo "<table bgcolor=\"skyblue\" border=\"2\">";
echo "<caption><b>Current Indoor Readings:</b></caption>";
echo "<tr><th>Time</th><th>Temp</th><th>Humidity</th><th>Barometric Pressure</th></tr>\n";
echo "<tr><td align=\"center\">$time</td><td align=\"center\">$tempi &deg;F</td><td align=\"center\">$humi%</td><td align=\"center\">$bar inHg</td></tr>\n";
echo "</table>\n";



/* Do query for maximum wind speed data over past 24 hours */
$sql = "select max(winds) from weather where t >= (now() -interval '24 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax24=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax24=$row3[0];

/* Do query for maximum wind speed data over past 20 hours */
$sql = "select max(winds) from weather where t >= (now() -interval '20 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax20=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax20=$row3[0];

/* Do query for maximum wind speed data over past 16 hours */
$sql = "select max(winds) from weather where t >= (now() -interval '16 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax16=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax16=$row3[0];

/* Do query for maximum wind speed data over past 12 hours */
$sql = "select max(winds) from weather where t >= (now() -interval '12 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax12=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax12=$row3[0];

/* Do query for maximum wind speed data over past 8 hours */
$sql = "select max(winds) from weather where t >= (now() -interval '8 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax08=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax08=$row3[0];

/* Do query for maximum wind speed data over past 4 hours */
$sql = "select max(winds) from weather where t >= (now() -interval '4 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax04=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax04=$row3[0];

/* Do query for maximum wind speed data over past hour */
$sql = "select max(winds) from weather where t >= (now() -interval '1 hours')";
/* Do the query thus created */
/* Reset the wind speed variables to zero */
$wndmax01=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wndmax01=$row3[0];




/* Do query for maximum wind direction data over past 20 minutes */
$sql = "select windd from weather where t >= (now() -interval '20 minutes')";
/* Do the query thus created */
/* Reset the wind direction variables to zero */
$wnddir20=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wnddir20=$row3[0];


/* Do query for maximum wind direction data over past 15 minutes */
$sql = "select windd from weather where t >= (now() -interval '15 minutes')";
/* Do the query thus created */
/* Reset the wind direction variables to zero */
$wnddir15=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wnddir15=$row3[0];

/* Do query for maximum wind direction data over past 10 minutes */
$sql = "select windd from weather where t >= (now() -interval '10 minutes')";
/* Do the query thus created */
/* Reset the wind direction variables to zero */
$wnddir10=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wnddir10=$row3[0];


/* Do query for maximum wind direction data over past 5 minutes */
$sql = "select windd from weather where t >= (now() -interval '5 minutes')";
/* Do the query thus created */
/* Reset the wind direction variables to zero */
$wnddir05=$row3[0]=0;
$q = $db->query($sql);
if(DB::iserror($q)) {  
  echo "Error getting 0hr bar data from db<br>\n";
  die($q->getMessage());
}
$q->fetchInto($row3);
$wnddir05=$row3[0];



/* Determine if dial positions for wind direction history
   pointers will be displayed, 1=yes; 0=no;
*/
$wndAng05=$wndAng10=$wndAng15=$wndAng20=0;
if($wnddir05>0 && $wnddir05 != $windd){
  $wndAng05=1;
}
if($wnddir10>0 && $wnddir10 != $windd){
  $wndAng10=1;
}
if($wnddir15>0 && $wnddir15 != $windd){
  $wndAng15=1;
}
if($wnddir20>0 && $wnddir20 != $windd){
  $wndAng20=1;
}


/* Calculate dial position for wind speed indicator */
$wndSpdAng=180+$winds*30/5;
if($wndmax24>0){
  $wndSpdAnga=180+$wndmax24*30/5;/* max wind speed pointer angle */
}

/* Calculate dial positions for thermometers */
$tempoAng=180+$tempo*30/10;
$tempiAng=180+$tempi*30/10;

/* Calculate dial positions for hydrometers */
$humoAng=210+$humo*30/10;
$humiAng=210+$humi*30/10;

/* Calculate dial position for barometer main pointer */
$barAng=30*$bar-870;
/* Calculate dial position for barometer history pointer */
/* don't display the histor pointer if there is no pressure 
   date from 24, 20, or 16 hours ago
*/
$barAnga=0;
if($bar24>0){
  $bartmp=$bar24*.03882; /* Convert barometric pressure to inHg */
  $barAngleH=30*$bartmp-870;
  $barAnga=1;
}elseif($bar20>0){
  $bartmp=$bar20*.03882; /* Convert barometric pressure to inHg */
  $barAngleH=30*$bartmp-870;
  $barAnga=1;
}elseif($bar16>0){
  $bartmp=$bar16*.03882; /* Convert barometric pressure to inHg */
  $barAngleH=30*$bartmp-870;
  $barAnga=1;
}


/* Calcultate bargraph scale factors for barometer */
$barMax=max($bar00,$bar01,$bar04,$bar08,$bar12,$bar16,$bar20,$bar24);
/* watch out for bar values = 0 or less than 700 when calculating min */
if($bar00==0){
  $barMin=$barMax;
}else{
  $barMin=$bar00;
}
if($bar01==0){
}else{
  $barMin=min($bar01,$barMin);
}
if($bar04==0){
}else{
  $barMin=min($bar04,$barMin);
}
if($bar08==0){
}else{
  $barMin=min($bar08,$barMin);
}
if($bar12==0){
}else{
  $barMin=min($bar12,$barMin);
}
if($bar16==0){
}else{
  $barMin=min($bar16,$barMin);
}
if($bar20==0){
}else{
  $barMin=min($bar20,$barMin);
}
if($bar24==0){
}else{
  $barMin=min($bar24,$barMin);
}

/* dont display a bar for bad data, i.e., if $bar## == 0 */
$dbar=$barMax-$barMin;
if($bar00==0){
  $bara=0;
}else{
  $bara=sprintf("%2.3f",($bar00-$barMin)*100/($dbar));
}
if($bar01==0){
  $barb=0;
}else{
  $barb=sprintf("%2.3f",($bar01-$barMin)*100/($dbar));
}
if($bar04==0){
  $barc=0;
}else{
  $barc=sprintf("%2.3f",($bar04-$barMin)*100/($dbar));
}
if($bar08==0){
  $bard=0;
}else{
  $bard=sprintf("%2.3f",($bar08-$barMin)*100/($dbar));
}
if($bar12==0){
  $bare=0;
}else{
  $bare=sprintf("%2.3f",($bar12-$barMin)*100/($dbar));
}
if($bar16==0){
  $barf=0;
}else{
  $barf=sprintf("%2.3f",($bar16-$barMin)*100/($dbar));
}
if($bar20==0){
  $barg=0;
}else{
  $barg=sprintf("%2.3f",($bar20-$barMin)*100/($dbar));
}if($bar24==0){
  $barh=0;
}else{
  $barh=sprintf("%2.3f",($bar24-$barMin)*100/($dbar));
}

?>

<br>
<hr>

<table><tr>
<td>
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=thermometer WIDTH=180 HEIGHT=180>
  <PARAM NAME=movie VALUE="thermometer2.swf?ango=1&angleo=<?php echo "$tempoAng";?>&ang=1&angle=<?php echo "$tempiAng";?>">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="thermometer2.swf?ango=1&angleo=<?php echo "$tempoAng";?>&ang=1&angle=<?php echo "$tempiAng";?>"
    bgcolor="white" WIDTH=180 HEIGHT=180 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>
</td>

<td>&nbsp;&nbsp;</td>

<td>
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=hydrometer WIDTH=180 HEIGHT=180>
  <PARAM NAME=movie VALUE="hydrometer2.swf?ango=1&angleo=<?php echo "$humoAng";?>&ang=1&angle=<?php echo "$humiAng";?>">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="hydrometer2.swf?ango=1&angleo=<?php echo "$humoAng";?>&ang=1&angle=<?php echo "$humiAng";?>" 
    bgcolor="white" WIDTH=180 HEIGHT=180 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>
</td>

<td>&nbsp;&nbsp;</td>

<td>
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=compass2 WIDTH=180 HEIGHT=180>
  <PARAM NAME=movie VALUE="compass3.swf?angle=<?php echo "$windd";?>&anga=<?php echo "$wndAng05";?>&anglea=<?php echo "$wnddir05";?>&angb=<?php echo "$wndAng10";?>&angleb=<?php echo "$wnddir10";?>&angc=<?php echo "$wndAng15";?>&anglec=<?php echo "$wnddir15";?>&angd=<?php echo "$wndAng20";?>&angled=<?php echo "$wnddir20";?>">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="compass3.swf?angle=<?php echo "$windd";?>&anga=<?php echo "$wndAng05";?>&anglea=<?php echo "$wnddir05";?>&angb=<?php echo "$wndAng10";?>&angleb=<?php echo "$wnddir10";?>&angc=<?php echo "$wndAng15";?>&anglec=<?php echo "$wnddir15";?>&angd=<?php echo "$wndAng20";?>&angled=<?php echo "$wnddir20";?>"
    bgcolor="white" WIDTH=180 HEIGHT=180 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>
</td>

<td>&nbsp;&nbsp;</td>

<td>
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=speed WIDTH=180 HEIGHT=180>
  <PARAM NAME=movie VALUE="speed.swf?angle=<?php echo "$wndSpdAng";?>&anga=<?php echo "$wndmax24";?>&anglea=<?php echo "$wndSpdAnga";?>">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="speed.swf?angle=<?php echo "$wndSpdAng";?>&anga=<?php echo "$wndmax24";?>&anglea=<?php echo "$wndSpdAnga";?>" 
    bgcolor="white" WIDTH=180 HEIGHT=180 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>
</td>
</tr>

<tr>
<td align="center"><b><font color = "red">Outdoor Temp = <?php echo "$tempo&deg;F";?></font></b></td>
<td>&nbsp;&nbsp;</td>
<td align="center"><b><font color = "red">Outdoor Hum = <?php echo "$humo%";?></font></b></td>
<td>&nbsp;&nbsp;</td>
<td align="center"><b>Wind Direction = <?php echo "$windd&deg;";?></b></td>
<td>&nbsp;&nbsp;</td>
<td align="center"><b>Wind Speed = <?php echo "$winds mph";?></b></td>
</tr>

<tr>
<td align="center"><b><font color = "blue">Indoor Temp = <?php echo "$tempi&deg;F";?></font></b></td>
<td>&nbsp;&nbsp;</td>
<td align="center"><b><font color = "blue">Indoor Hum = <?php echo "$humi%";?></font></b></td>
<td>&nbsp;&nbsp;</td>
<td align="center"></td>
<td>&nbsp;&nbsp;</td>
<td align="center"><b><font color = "blue">Max Wind Speed = <?php echo "$wndmax24 mph";?></font></b></td>
</tr>
</table>

<hr>

<table><tr><td>
<table bgcolor="skyblue" border=2>
<tr><th align="center">Hour</th><th align="center">Pressure<br>(inHg)</th></tr>
<tr><td>24</td><td><?php printf("%2.2f",$bar24*.03882);?></td></tr>
<tr><td>20</td><td><?php printf("%2.2f",$bar20*.03882);?></td></tr>
<tr><td>16</td><td><?php printf("%2.2f",$bar16*.03882);?></td></tr>
<tr><td>12</td><td><?php printf("%2.2f",$bar12*.03882);?></td></tr>
<tr><td>08</td><td><?php printf("%2.2f",$bar08*.03882);?></td></tr>
<tr><td>04</td><td><?php printf("%2.2f",$bar04*.03882);?></td></tr>
<tr><td>01</td><td><?php printf("%2.2f",$bar01*.03882);?></td></tr>
<tr><td>00</td><td><?php printf("%2.2f",$bar00*.03882);?></td></tr>
</table>
</td>
<td>&nbsp;&nbsp;&nbsp;</td>
<td>
<table><tr><td>
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=barometer WIDTH=180 HEIGHT=180>
  <PARAM NAME=movie VALUE="barometer2.swf?angle=<?php echo "$barAng";?>&anga=<?php echo "$barAnga";?>&anglea=<?php echo "$barAngleH";?>&bara=<?php echo "$bara";?>&barb=<?php echo "$barb";?>&barc=<?php echo "$barc";?>&bard=<?php echo "$bard";?>&bare=<?php echo "$bare";?>&barf=<?php echo "$barf";?>&barg=<?php echo "$barg";?>&barh=<?php echo "$barh";?>">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="barometer2.swf?angle=<?php echo "$barAng";?>&anga=<?php echo "$barAnga";?>&anglea=<?php echo "$barAngleH";?>&bara=<?php echo "$bara";?>&barb=<?php echo "$barb";?>&barc=<?php echo "$barc";?>&bard=<?php echo "$bard";?>&bare=<?php echo "$bare";?>&barf=<?php echo "$barf";?>&barg=<?php echo "$barg";?>&barh=<?php echo "$barh";?>" 
    bgcolor="white" WIDTH=180 HEIGHT=180 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>
</td></tr>
<tr><td align="center"><b>Barometer</b></td></tr>
</table>
</td>
<td>&nbsp;&nbsp;&nbsp;</td>
<td>
<table bgcolor="skyblue" border="2">
<tr><th align="center">Hour</th><th align="center">Max Wind<br>Speed (mph)</th></tr>
<tr><td>24</td><td><?php printf("%2.2f",$wndmax24);?></td></tr>
<tr><td>20</td><td><?php printf("%2.2f",$wndmax20);?></td></tr>
<tr><td>16</td><td><?php printf("%2.2f",$wndmax16);?></td></tr>
<tr><td>12</td><td><?php printf("%2.2f",$wndmax12);?></td></tr>
<tr><td>08</td><td><?php printf("%2.2f",$wndmax08);?></td></tr>
<tr><td>04</td><td><?php printf("%2.2f",$wndmax04);?></td></tr>
<tr><td>01</td><td><?php printf("%2.2f",$wndmax01);?></td></tr>
<tr><td>00</td><td><?php printf("%2.2f",$winds);?></td></tr>
</table>
</tr>
</table>

<hr>

<br>
<br>
<!--
<img src="tmp_hum_bar.gif.php">
// -->
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=tmp_hum_bar WIDTH=800 HEIGHT=240>
  <PARAM NAME=movie VALUE="tmp_hum_bar.swf.php">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="tmp_hum_bar.swf.php" 
    bgcolor="white" WIDTH=800 HEIGHT=240 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>

<br>
<br>
<!--
<img src="tmp_hum.gif.php">
// -->
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=tmp_hum WIDTH=800 HEIGHT=240>
  <PARAM NAME=movie VALUE="tmp_hum.swf.php">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="tmp_hum.swf.php" 
    bgcolor="white" WIDTH=800 HEIGHT=240 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>


<br>
<br>
<!--
<img src="wind.gif.php">
// -->
<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=wind WIDTH=800 HEIGHT=240>
  <PARAM NAME=movie VALUE="wind.swf.php">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="wind.swf.php" 
    bgcolor="white" WIDTH=800 HEIGHT=240 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>

<br>
<br>

<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
  codebase="http://macromedia.com/cabs/swflash.cab#version=5,0,0,0"   
  ID=wind WIDTH=800 HEIGHT=240>
  <PARAM NAME=movie VALUE="rain.swf.php">
  <PARAM NAME=quality VALUE="medium">
  <PARAM NAME=bgcolor VALUE="white">
  <EMBED src="rain.swf.php" 
    bgcolor="white" WIDTH=800 HEIGHT=240 
    TYPE="application/x-shockwave-flash">
  </EMBED>
</OBJECT>


</center>
<br>
<br>
</body>
</html>