select sp.sensor_meas_no, sp.sensor_no, pn.param_name from sensorparameters as sp, parameternames as pn where sp.param_no = pn.param_no; 
