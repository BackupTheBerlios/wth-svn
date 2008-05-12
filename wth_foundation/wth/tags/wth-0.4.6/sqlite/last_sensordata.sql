SELECT DISTINCT sd.dataset_date, sd.sensor_meas_no, sd.meas_value FROM sensordata AS sd JOIN sensorupdate AS sup WHERE sd.dataset_date = sup.last_update;

