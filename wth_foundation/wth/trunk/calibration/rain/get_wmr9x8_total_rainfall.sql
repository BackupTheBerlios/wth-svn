--
-- get_wmr9x8_total_rainfall.sql
--
-- select total_rainfall data of WMR9x8 rainsensor for calibration
--
--
--
. headers off

--
-- fetch number of datasets
--
.separator ' '

select 
  '# ', count(*), ' datasets '
from 
  sensordata sd
where 
  sd.sensor_meas_no = (
SELECT 
  sensordata.sensor_meas_no
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
AND
  sensornames.sensor_name = 'rainsensor'
AND
  parameternames.param_name = 'total_rainfall'
)
and 
  sd.dataset_date >  1357300000
and 
  sd.dataset_date <  1357370000
;

--
-- fetch rainsensor data
--
.separator ', '

select 
  sd.dataset_date, sd.meas_value 
from 
  sensordata sd
where 
  sd.sensor_meas_no = (
SELECT 
  sensordata.sensor_meas_no
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
AND
  sensornames.sensor_name = 'rainsensor'
AND
  parameternames.param_name = 'total_rainfall'
)
and 
  sd.dataset_date >  1357300000
and 
  sd.dataset_date <  1357370000
;

