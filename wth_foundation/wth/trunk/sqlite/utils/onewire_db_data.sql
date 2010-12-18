SELECT DISTINCT sensorname.sensor_name, sensorname.sensor_no, 
parametername.parameter_name, 
sensorupdate.last_update, sensordata.meas_value, 
parametername.unit 
FROM 
  sensorupdate, sensordata,sensorname,sensordevparameters,parametername 
WHERE sensorupdate.last_update = sensordata.dataset_date 
AND
  sensorupdate.sensor_meas_no = sensordata.sensor_meas_no
AND
  sensorname.sensor_no = sensordevparameters.sensor_no
AND
  sensorupdate.sensor_meas_no = sensordevparameters.sensor_meas_no
AND
  parametername.parameter_no = sensordevparameters.parameter_no
;
