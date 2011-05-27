--select *  from sensordata where sensor_meas_no = 18 and dataset_date >  1306000000;

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

--select 
--  *  
--from
--  sensornewflag
--where 
--  sensor_no = 10
--and 
--  statusset_date > 1300000000;
