SELECT DISTINCT 
  sensornames.sensor_name,parameternames.parameter_name,
  sensorupdate.last_update, sensordata.meas_value,
  parameternames.unit 
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
  parameternames.parameter_no = sensorparameters.parameter_no
;
