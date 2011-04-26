--SELECT 
--  sp.sensor_meas_no 
--FROM 
--  sensornames sn, parameternames pn, sensorparameters sp
--WHERE
--  sn.sensor_name = 'thbnew_sensor'
--AND
--  pn.parameter_name = 'pressure'
--AND
--  sp.sensor_no = sn.sensor_no
--AND
--  sp.parameter_no = pn.parameter_no

SELECT 
  count(*)
--  sn.sensor_name, pn.parameter_name, sd.dataset_date, sd.meas_value, pn.unit
FROM 
  sensornames sn, sensorparameters sp, parameternames pn, sensordata sd 
WHERE 
  sn.sensor_no = sp.sensor_no 
AND
  pn.parameter_no = sp.parameter_no
AND 
  pn.parameter_name = 'pressure' 
AND 
  sn.sensor_name = 'thbnew_sensor'
AND
  sd.sensor_meas_no = sp.sensor_meas_no
AND
  sd.dataset_date 
    BETWEEN 
      1301056500 
    AND 
      1303658500;

--SELECT DISTINCT 
--  sensornames.sensor_name,parameternames.parameter_name,
--  sensorupdate.last_update, sensordata.meas_value,
--  parameternames.unit 
--FROM  
--  sensorupdate, sensordata,sensornames,sensorparameters,parameternames 
--WHERE sensorupdate.last_update = sensordata.dataset_date 
--AND 
--  sensorupdate.sensor_meas_no = sensordata.sensor_meas_no 
--AND 
--  sensornames.sensor_no = sensorparameters.sensor_no 
--AND 
--  sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no 
--AND 
--  parameternames.parameter_no = sensorparameters.parameter_no
--AND
--  parameternames.parameter_name = 'pressure'
--AND
--  sensornames.sensor_name = 'thbnew_sensor'

;

