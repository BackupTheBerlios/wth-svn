SELECT DISTINCT sensorparameters.sensor_no, sensorupdate.last_update
FROM sensorupdate,sensorparameters
WHERE sensorparameters.sensor_meas_no = sensorupdate.sensor_meas_no
AND
last_update > 0
;
