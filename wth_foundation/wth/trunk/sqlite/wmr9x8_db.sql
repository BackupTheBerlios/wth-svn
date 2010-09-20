--
-- Datastructures of WMR9x8 weatherstation
-- 
-- sample description of table contents
--
-- Sensor#	Sensorname	Devicetype	Description		
-- -------	----------	----------	-----------	
-- 1		windsensor 	0		wind speed, wind direction
-- 2		rainsensor	1		rain
-- 3		th_sensor	2		temperature, humidity
-- 4		mushroomsensor	3		temperature, humidity
-- 5		t_sensor	4		temperature
-- 6		thb_sensor	5		temperature, humidity, pressure
-- 7		thb_new_sensor 	6		new temperature, humidity, pressure
-- 
--
-- Parameter#	Parametername		Unit		Gain	Offset
-- ----------	-------------		----		----	------
-- 1		gust_overrange		sign		
-- 2		average_overrange		sign
-- 3		low_battery		sign
-- 4		wind_direction		degree		1.000	0.000
-- 5		gust_windspeed		m/sec^2		1.000	0.000
-- 6		average_windspeed	m/sec^2		1.000	0.000
-- 7		chill_nodata		sign
-- 8		chill_overrange		sign
-- 9		windchill		degC		1.000	0.000
-- 10		rate_overrange		sign
-- 11		total_overrange		sign
-- 12		yesterday_overrange	sign
-- 13		current_rainrate	mm/hr		1.000	0.000
-- 14		total_rainfall		mm		1.000	0.000
-- 15		yesterday_rainfall	mm		1.000	0.000
-- 16		total_startdate		dd/mm/yyyy
-- 17		channel_no
-- 18		dew_underrange		sign
-- 19		temperature		degC		1.000	0.000
-- 20		over_underrange		sign
-- 21		humidity		% rel.hum.	1.000	0.000
-- 22		dew_temperature		degC		1.000	0.000
-- 23		pressure		mbar		1.000	0.000
-- 24		weather_status
-- 25		sealevel_offset		mbar

--
-- WMR9x8 database schema
-- 
-- Table sensorname
-- -----------------
-- contains number, name, devicetype and description of sensor
-- 
CREATE TABLE sensorname
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_name VARCHAR(255) NOT NULL,
      device_type INTEGER NOT NULL,
      description VARCHAR(255) NOT NULL
    );
INSERT INTO sensorname VALUES ( 1, 'windsensor', 0, 'wind direction, gust speed, average wind speed and wind chill. Windchill data in combination with ourdoor temperature reading.');
INSERT INTO sensorname VALUES ( 2, 'rainsensor', 1, 'rain rate, yesterdays rainfall and total rainfall');
INSERT INTO sensorname VALUES ( 3, 'th_sensor', 2, 'outdor temperature, humidity and dew pointa.');
INSERT INTO sensorname VALUES ( 4, 'mushroomsensor', 3, 'outdoor temperature, humidity and dew point');
INSERT INTO sensorname VALUES ( 5, 't_sensor', 4, 'indoor temperature');
INSERT INTO sensorname VALUES ( 6, 'thb_sensor', 5, 'indoor temperature, humidity and dew point. Barometric pressure');

--
-- Table parametername
-- -------------------
-- contains name und unit of parameter, e.g. 
-- - pressure 
-- - mbar
-- 
CREATE TABLE parametername
    (
      parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      parameter_name VARCHAR(64) NOT NULL,
      unit VARCHAR(64) NOT NULL,
      gain FLOAT NOT NULL,
      offset FLOAT NOT NULL
    );
INSERT INTO parametername VALUES ( 1, 'Windgust overrange', '', 1.000, 0.000);
INSERT INTO parametername VALUES ( 2, 'Average windspeed overrange', '', 1.000, 0.000);
INSERT INTO parametername VALUES ( 3, 'Low battery', 'boolean' );
INSERT INTO parametername VALUES ( 4, 'Wind direction', 'degree', 1.000, 0.000);
INSERT INTO parametername VALUES ( 5, 'Gust windspeed', 'm/sec^2', 1.000, 0.000);
INSERT INTO parametername VALUES ( 6, 'Average windspeed', 'm/sec^2', 1.000, 0.000);
INSERT INTO parametername VALUES ( 7, 'Chill nodata', 'boolean');
INSERT INTO parametername VALUES ( 8, 'Chill overrange', 'boolean');
INSERT INTO parametername VALUES ( 9, 'Windchill', 'degC', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 10, 'Rainrate overrate', 'boolean' );
INSERT INTO parametername VALUES ( 11, 'Raintotal overrange', 'boolean' );
INSERT INTO parametername VALUES ( 12, 'Yesterday overrange', 'boolean' );
INSERT INTO parametername VALUES ( 13, 'Current rainrate', 'mm/hr', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 14, 'Total rainfall', 'mm', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 15, 'Yesterday rainfall', 'mm', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 16, 'Total startdate', 'dd/mm/yyyy' );
INSERT INTO parametername VALUES ( 17, 'Channel number', 'number' );
INSERT INTO parametername VALUES ( 18, 'Dew underrange', 'boolean' );
INSERT INTO parametername VALUES ( 19, 'Temperature', 'degC', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 20, 'Over/underrange', 'boolean' );
INSERT INTO parametername VALUES ( 21, 'Humidity', '% rel.hum.', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 22, 'Dew temperature', 'degC', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 23, 'Barometric pressure', 'mbar', 1.000, 0.000 ); 
INSERT INTO parametername VALUES ( 24, 'Weather status' );
INSERT INTO parametername VALUES ( 25, 'Sealevel offset', 'mbar' );
 
-- Table sensordeviceparameters
-- -----------------------
-- relation of table sensorname to table parametername
-- 
CREATE TABLE sensordevparameters
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no)
    );
INSERT INTO sensordevparameters VALUES ( 1, 1, 1); -- windsensor and gust_over
INSERT INTO sensordevparameters VALUES ( 2, 1, 2); -- windsensor and average_over
INSERT INTO sensordevparameters VALUES ( 3, 1, 3); -- windsensor and low_battery
INSERT INTO sensordevparameters VALUES ( 4, 1, 4); -- windsensor and wind_direction
INSERT INTO sensordevparameters VALUES ( 5, 1, 5); -- windsensor and gust_windspeed
INSERT INTO sensordevparameters VALUES ( 6, 1, 6); -- windsensor and average_windspeed
INSERT INTO sensordevparameters VALUES ( 7, 1, 7); -- windsensor and chill_nodata
INSERT INTO sensordevparameters VALUES ( 8, 1, 8); -- windsensor and chill_overrange
INSERT INTO sensordevparameters VALUES ( 9, 1, 9); -- windsensor and wind_chill
INSERT INTO sensordevparameters VALUES ( 10, 2, 10); -- rainsensor and rate_overrange 
INSERT INTO sensordevparameters VALUES ( 11, 2, 11); -- rainsensor and total_overrange 
INSERT INTO sensordevparameters VALUES ( 12, 2, 3); -- rainsensor and low_battery
INSERT INTO sensordevparameters VALUES ( 13, 2, 12); -- rainsensor and yesterday_overrange
INSERT INTO sensordevparameters VALUES ( 14, 2, 13); -- rainsensor and current_rainrate 
INSERT INTO sensordevparameters VALUES ( 15, 2, 14); -- rainsensor and total_rainfall
INSERT INTO sensordevparameters VALUES ( 16, 2, 15); -- rainsensor and yesterday_rainfall 
INSERT INTO sensordevparameters VALUES ( 17, 2, 16); -- rainsensor and total_startdate 
INSERT INTO sensordevparameters VALUES ( 18, 3, 17); -- th_sensor and channel_no
INSERT INTO sensordevparameters VALUES ( 19, 3, 18); -- th_sensor and dew_underrange
INSERT INTO sensordevparameters VALUES ( 20, 3, 3); -- th_sensor and low_battery
INSERT INTO sensordevparameters VALUES ( 21, 3, 19); -- th_sensor and temperature
INSERT INTO sensordevparameters VALUES ( 22, 3, 20); -- th_sensor and over_underrange
INSERT INTO sensordevparameters VALUES ( 23, 3, 21); -- th_sensor and humidity
INSERT INTO sensordevparameters VALUES ( 24, 3, 22); -- th_sensor and dew_temperature
INSERT INTO sensordevparameters VALUES ( 25, 4, 18); -- mushroom_sensor and dew_underrange
INSERT INTO sensordevparameters VALUES ( 26, 4, 3); -- mushroom_sensor and low_battery
INSERT INTO sensordevparameters VALUES ( 27, 4, 19); -- mushroom_sensor and temperature
INSERT INTO sensordevparameters VALUES ( 28, 4, 20); -- mushroom_sensor and over_underrange
INSERT INTO sensordevparameters VALUES ( 29, 4, 21); -- mushroom_sensor and humidity
INSERT INTO sensordevparameters VALUES ( 30, 4, 22); -- mushroom_sensor and dew_temperature
INSERT INTO sensordevparameters VALUES ( 31, 5, 17); -- t_sensor and channel_no
INSERT INTO sensordevparameters VALUES ( 32, 5, 3); -- t_sensor and low_battery
INSERT INTO sensordevparameters VALUES ( 33, 5, 19); -- t_sensor and temperature
INSERT INTO sensordevparameters VALUES ( 34, 5, 20); -- t_sensor and over_underrange
INSERT INTO sensordevparameters VALUES ( 35, 6, 18); -- thb_sensor and dew_underrange
INSERT INTO sensordevparameters VALUES ( 36, 6, 3); -- thb_sensor and low_battery
INSERT INTO sensordevparameters VALUES ( 37, 6, 19); -- thb_sensor and temperature
INSERT INTO sensordevparameters VALUES ( 38, 6, 20); -- thb_sensor and over_underrange
INSERT INTO sensordevparameters VALUES ( 39, 6, 21); -- thb_sensor and humidity
INSERT INTO sensordevparameters VALUES ( 40, 6, 22); -- thb_sensor and dew_temperature
INSERT INTO sensordevparameters VALUES ( 41, 6, 23); -- thb_sensor and barometric_pressure
INSERT INTO sensordevparameters VALUES ( 42, 6, 24); -- thb_sensor and weather_status
INSERT INTO sensordevparameters VALUES ( 43, 6, 25); -- thb_sensor and sealevel_offset
INSERT INTO sensordevparameters VALUES ( 44, 7, 18); -- thb_new_sensor and dew_underrange
INSERT INTO sensordevparameters VALUES ( 45, 7, 3); -- thb_new_sensor and low_battery
INSERT INTO sensordevparameters VALUES ( 46, 7, 19); -- thb_new_sensor and temperature
INSERT INTO sensordevparameters VALUES ( 47, 7, 20); -- thb_new_sensor and over_underrange
INSERT INTO sensordevparameters VALUES ( 48, 7, 21); -- thb_new_sensor and humidity
INSERT INTO sensordevparameters VALUES ( 49, 7, 22); -- thb_new_sensor and dew_temperature
INSERT INTO sensordevparameters VALUES ( 50, 7, 23); -- thb_new_sensor and barometric_pressure
INSERT INTO sensordevparameters VALUES ( 51, 7, 24); -- thb_new_sensor and weather_status
INSERT INTO sensordevparameters VALUES ( 52, 7, 25); -- thb_new_sensor and sealevel_offset
-- 
-- Table sensordata
-- ----------------
-- contains 
-- - time
-- - instrument value ( e.g. value of voltage)
-- - measured value ( the value of physical quantity)
-- 
CREATE TABLE sensordata
    (
      dataset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      dataset_date DATE NOT NULL,
      sensor_meas_no INTEGER NOT NULL,
      meas_value FLOAT NOT NULL,
      FOREIGN KEY (sensor_meas_no) REFERENCES sensorparameters (sensor_meas_no)
    );
-- 
--
-- 
CREATE TABLE sensorstatus
    (
      statusset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      statusset_date DATE NOT NULL,
      sensor_no INTEGER NOT NULL,
      low_battery INTEGER NOT NULL,
      channel_no INTEGER NOT NULL,
      gust_over INTEGER NOT NULL,
      average_over INTEGER NOT NULL,
      chill_nodata INTEGER NOT NULL,
      chill_over INTEGER NOT NULL,
      rate_over INTEGER NOT NULL,
      total_over INTEGER NOT NULL,
      yesterday_over INTEGER NOT NULL,
      dew_under INTEGER NOT NULL,
      over_under INTEGER NOT NULL,
      weather_status INTEGER NOT NULL,
      sealevel_offset FLOAT NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
    );
-- 
-- Table sensorupdate
-- ------------------
-- contains information about last measured date
-- 
CREATE TABLE sensorupdate
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      last_update DATE NOT NULL
    );
INSERT INTO sensorupdate VALUES (1, 0);
INSERT INTO sensorupdate VALUES (2, 0);
INSERT INTO sensorupdate VALUES (3, 0);
INSERT INTO sensorupdate VALUES (4, 0);
INSERT INTO sensorupdate VALUES (5, 0);
INSERT INTO sensorupdate VALUES (6, 0);
INSERT INTO sensorupdate VALUES (7, 0);
INSERT INTO sensorupdate VALUES (8, 0);
INSERT INTO sensorupdate VALUES (9, 0);
INSERT INTO sensorupdate VALUES (10, 0);
INSERT INTO sensorupdate VALUES (11, 0);
INSERT INTO sensorupdate VALUES (12, 0);
INSERT INTO sensorupdate VALUES (13, 0);
INSERT INTO sensorupdate VALUES (14, 0);
INSERT INTO sensorupdate VALUES (15, 0);
INSERT INTO sensorupdate VALUES (16, 0);
INSERT INTO sensorupdate VALUES (17, 0);
INSERT INTO sensorupdate VALUES (18, 0);
INSERT INTO sensorupdate VALUES (19, 0);
INSERT INTO sensorupdate VALUES (20, 0);
INSERT INTO sensorupdate VALUES (21, 0);
INSERT INTO sensorupdate VALUES (22, 0);
INSERT INTO sensorupdate VALUES (23, 0);
INSERT INTO sensorupdate VALUES (24, 0);
INSERT INTO sensorupdate VALUES (25, 0);
INSERT INTO sensorupdate VALUES (26, 0);
INSERT INTO sensorupdate VALUES (27, 0);
INSERT INTO sensorupdate VALUES (28, 0);
INSERT INTO sensorupdate VALUES (29, 0);
INSERT INTO sensorupdate VALUES (30, 0);
INSERT INTO sensorupdate VALUES (31, 0);
INSERT INTO sensorupdate VALUES (32, 0);
INSERT INTO sensorupdate VALUES (33, 0);
INSERT INTO sensorupdate VALUES (34, 0);
INSERT INTO sensorupdate VALUES (35, 0);
INSERT INTO sensorupdate VALUES (36, 0);
INSERT INTO sensorupdate VALUES (37, 0);
INSERT INTO sensorupdate VALUES (38, 0);
INSERT INTO sensorupdate VALUES (39, 0);
INSERT INTO sensorupdate VALUES (40, 0);
INSERT INTO sensorupdate VALUES (41, 0);
INSERT INTO sensorupdate VALUES (42, 0);
INSERT INTO sensorupdate VALUES (43, 0);
INSERT INTO sensorupdate VALUES (44, 0);
INSERT INTO sensorupdate VALUES (45, 0);
INSERT INTO sensorupdate VALUES (46, 0);
INSERT INTO sensorupdate VALUES (47, 0);
INSERT INTO sensorupdate VALUES (48, 0);
INSERT INTO sensorupdate VALUES (49, 0);
INSERT INTO sensorupdate VALUES (50, 0);
INSERT INTO sensorupdate VALUES (51, 0);
INSERT INTO sensorupdate VALUES (52, 0);
