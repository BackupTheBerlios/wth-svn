--
-- onewire_db_status.sql
--
-- read status of 1-Wire sensors
-- 

select sn.sensor_no, sn.sensor_name, su.last_update, dt.devicetyp, dt.familycode, dt.serialnum, pn.parameter_name, pn.unit, pn.gain, pn.offset, sdp.sensor_no  from sensorname as sn, sensorupdate as su, devicetyp as dt, parametername as pn, sensordevparameters as sdp where sn.sensor_no = sdp.sensor_no and dt.device_no = sdp.device_no and pn.parameter_no = sdp.parameter_no and su.sensor_meas_no = sdp.sensor_meas_no;
