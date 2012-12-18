<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" 
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <title>Thalreit Weather</title>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
  <meta http-equiv='Content-Style-Type' content='text/css' />
  <link rel='stylesheet' href='layout.css' type='text/css' />
</head>

<body>
<div id='header'>
  <div id='headerlink'>
    <a href="/index.html">Home</a>
    <a href="/weather/index.html">Weather</a>
    <a href="/project/index.php">Project</a>
    <a href="/about.html">About</a>
  </div>

  <div id='headerlogo'>
    <a href='/index.html'><img src='/images/thalreit-logo.gif'
    alt='Thalreit Linux Project' border='0' /></a>
  </div>

  <div id='headercontent'>Thalreit Linux</div>
  <div id='headergeopos'>Lat 47&deg;46'41"N, Lon 12&deg;06'42", JN67BS</div>
</div>

<div id='midsection'>
  <div id='leftcol'>
  </div>
  <div id='content'>
    <h1>1-Wire weatherstation</h1>
Current conditions in the Thalreit region.

<?php
  $status_sql = "SELECT DISTINCT 
  sensornames.sensor_name,parameternames.param_name,
  sensorupdate.last_update, sensordata.meas_value,
  parameternames.param_unit 
FROM  
  sensorupdate, sensordata,sensornames,sensorparameters,parameternames 
WHERE sensorupdate.last_update = sensordata.dataset_date 
AND 
  sensorupdate.sensor_meas_no = sensordata.sensor_meas_no 
AND 
  sensornames.sensor_no = sensorparameters.sensor_no 
AND 
  sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no 
AND 
  parameternames.param_no = sensorparameters.param_no";



  try {
    $dbh = new PDO("pgsql:dbname=onewire user=wthrd host=ikarus");
    $stmt =$dbh->prepare( $status_sql);
    $stmt->execute();
    $rows = $stmt->fetchAll();
    $now = time(); 
    echo "<table>\n";
    echo "<tr><th>Sensor</th><th>Parameter</th><th>Date</th><th>Measured Value</th><th>Unit</th></tr>\n";
    foreach ($rows as $row) {
      $measdatetime = date_create($row[2]);
      $date_string = date_format($measdatetime, 'Y-m-d H:i:s');
      $meas_epoch  = strtotime($date_string);
      $measdiff = $now - $meas_epoch;

      if ( $measdiff < 84600) {
        printf("<tr><td>%s</td><td>%s</td><td>%s</td><td align=right>%01.2f</td><td>%s</td></tr >\n", $row[0], $row[1], $date_string, $row[3], $row[4]);
      }
    }
    echo "</table>\n";
    $dbh = null;

    } catch(PDOException $e) {
      echo $e->getMessage();
    }
?>
<p>
Data measured with 1-Wire weatherstation. Values are always most recent ones.
  </div>
</div>
<div id='footer'>
  <img SRC="/images/power-button.png"  border=0 alt="[Linux powered]">
  &copy 2001-2011 <a href="mailto:volker@thalreit.de">Volker Jahns</a>
</div>
</body>
</html>
