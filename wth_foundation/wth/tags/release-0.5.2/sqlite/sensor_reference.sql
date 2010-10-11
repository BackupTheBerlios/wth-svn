select sp.sensor_meas_no, sp.sensor_no, sn.sensoraddr, sn.sensorvers, sn.sensorname from sensornames as sn, sensorparameters as sp where sp.sensor_no = sn.sensor_no;

