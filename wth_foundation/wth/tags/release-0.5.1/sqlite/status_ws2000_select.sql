select sensor_no, max(statusset_date), sensor_status from sensorstatus group by sensor_no;
