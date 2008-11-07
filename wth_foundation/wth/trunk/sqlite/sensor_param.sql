select sp.sensor_meas_no, sp.sensor_no, pn.parameter_name from sensorparameters as sp, parameternames as pn where sp.parameter_no = pn.parameter_no; 
