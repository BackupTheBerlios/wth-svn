SELECT DISTINCT 
  sensornames.sensor_name,sensordata.sensor_meas_no,
  parameternames.param_name,
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
  parameternames.param_no = sensorparameters.param_no
;
