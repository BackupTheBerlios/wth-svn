.header ON
select sp.sensor_meas_no, sn.sensor_name, pn.param_name, pn.param_offset, pn.param_gain
from sensornames as sn , sensorparameters as sp, parameternames as pn 
where sn.sensor_no = sp.sensor_no and sp.param_no = pn.param_no ;
