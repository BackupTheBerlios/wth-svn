--
-- last_n_sensordata.sql
--
-- last n datasets from table sensordata for a given sensor/parameter
--
.header ON

select 
  sd.dataset_no, sn.sensor_name, pn.param_name,
  datetime( sd.dataset_date, 'unixepoch') as UTC,
  datetime( sd.dataset_date, 'unixepoch', 'localtime'),
  sd.meas_value 
from sensordata sd, sensornames sn, parameternames pn, sensorparameters sp
where 
  sd.dataset_no in (
    select dataset_no 
    from sensordata 
    where sensor_meas_no = 25 order by dataset_no desc limit 25
  ) and
 sd.sensor_meas_no = sp.sensor_meas_no
 and
 sp.sensor_no = sn.sensor_no
 and
 sp.param_no = pn.param_no;

