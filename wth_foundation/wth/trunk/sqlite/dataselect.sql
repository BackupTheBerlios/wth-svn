SELECT sensor_meas_no, MAX(dataset_date), meas_value FROM sensordata GROUP BY sensor_meas_no;
SELECT sensor_meas_no, dataset_date, meas_value FROM sensordata WHERE dataset_date >= 1184353365 GROUP BY sensor_meas_no;
