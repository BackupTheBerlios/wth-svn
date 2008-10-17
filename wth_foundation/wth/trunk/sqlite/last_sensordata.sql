select distinct sensornames.sensorname, parameternames.parameter_name, sensorupdate.last_update, sensordata.meas_value, parameternames.parameter_unit from sensorupdate, sensordata,sensornames,sensorparameters,parameternames where sensorupdate.last_update = sensordata.dataset_date and sensorupdate.sensor_meas_no = sensordata.sensor_meas_no and sensornames.sensor_no = sensorparameters.sensor_no and sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no and parameternames.parameter_no = sensorparameters.parameter_no;


