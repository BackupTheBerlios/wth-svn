--
-- onewire_db_status.sql
--
-- read status of 1-Wire sensors
-- 

select sn.sensor_no, pn.param_no, sdp.sensor_meas_no, sn.sensor_name, su.last_update, dt.devicetyp, dt.familycode, dt.serialnum, pn.param_name, pn.param_unit, pn.param_gain, pn.param_offset, sdp.sensor_no  from sensornames as sn, sensorupdate as su, devicetyp as dt, parameternames as pn, sensorparameters as sdp where sn.sensor_no = sdp.sensor_no and dt.device_no = sdp.device_no and pn.param_no = sdp.param_no and su.sensor_meas_no = sdp.sensor_meas_no;
