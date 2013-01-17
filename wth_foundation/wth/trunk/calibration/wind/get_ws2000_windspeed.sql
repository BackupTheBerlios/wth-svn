--
-- select data of windspeed parameter
--
-- WS2000 windspeed - sensor_meas_no = 18
--
select 
  sd.dataset_date, sd.meas_value 
from 
  sensordata sd
where 
  sd.sensor_meas_no = 18 
and 
  sd.dataset_date >  1306000000
;

select 
  count(*)
from 
  sensordata sd
where 
  sd.sensor_meas_no = 18 
and 
  sd.dataset_date >  1306000000
;

