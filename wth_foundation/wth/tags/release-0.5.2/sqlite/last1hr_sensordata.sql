SELECT DISTINCT sensornames.sensorname, parameternames.parameter_name, 
 sensordata.dataset_date, sensordata.meas_value, 
 parameternames.parameter_unit 
FROM 
 sensorupdate, sensordata,sensornames,sensorparameters,parameternames 
WHERE 
 sensordata.dataset_date >= sensorupdate.last_update - 3600
AND
 sensorupdate.sensor_meas_no = sensordata.sensor_meas_no 
AND 
 sensornames.sensor_no = sensorparameters.sensor_no 
AND 
 sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no 
AND 
 parameternames.parameter_no = sensorparameters.parameter_no
AND
 sensornames.sensor_no = 9
AND
 sensordata.sensor_meas_no = 17;
