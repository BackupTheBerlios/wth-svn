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
-- 1		wind_direction		degree		1.000	0.000
-- 2		gust_windspeed		m/sec^2		1.000	0.000
-- 3		average_windspeed	m/sec^2		1.000	0.000
-- 4		windchill		degC		1.000	0.000
-- 5		current_rainrate	mm/hr		1.000	0.000
-- 6		total_rainfall		mm		1.000	0.000
-- 7		yesterday_rainfall	mm		1.000	0.000
-- 8		total_startdate		dd/mm/yyyy
-- 9		temperature		degC		1.000	0.000
-- 10		humidity		% rel.hum.	1.000	0.000
-- 11		dew_temperature		degC		1.000	0.000
-- 12		pressure		mbar		1.000	0.000
-- 13		sealevel_offset		mbar
--
-- Status#   	Statusname		
-- -------	----------
-- 1		gust_overrange
-- 2		average_overrange
-- 3		low_battery
-- 4		chill_nodata
-- 5		chill_overrange	
-- 6		rate_overrange
-- 7		total_overrange	
-- 8		yesterday_overrange
-- 9		channel_no
-- 10		dew_underrange
-- 11		over_underrange
-- 12		weather_status
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
INSERT INTO sensorname VALUES ( 7, 'thb_new_sensor', 6, 'indoor temperature, humidity and dew point. Barometric pressure. New version');

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
INSERT INTO parametername VALUES ( 1, 'Wind direction', 'degree', 1.000, 0.000);
INSERT INTO parametername VALUES ( 2, 'Gust windspeed', 'm/sec^2', 1.000, 0.000);
INSERT INTO parametername VALUES ( 3, 'Average windspeed', 'm/sec^2', 1.000, 0.000);
INSERT INTO parametername VALUES ( 4, 'Windchill', 'degC', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 5, 'Current rainrate', 'mm/hr', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 6, 'Total rainfall', 'mm', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 7, 'Yesterday rainfall', 'mm', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 8, 'Total startdate', 'dd/mm/yyyy' );
INSERT INTO parametername VALUES ( 9, 'Temperature', 'degC', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 10, 'Humidity', '% rel.hum.', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 11, 'Dew temperature', 'degC', 1.000, 0.000 );
INSERT INTO parametername VALUES ( 12, 'Barometric pressure', 'mbar', 1.000, 0.000 ); 
INSERT INTO parametername VALUES ( 13, 'Sealevel offset', 'mbar' );

 
-- Table sensor_parameter
-- -----------------------
-- relation of table sensorname to table parametername
-- 
CREATE TABLE sensor_parameter
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no)
    );
INSERT INTO sensor_parameter VALUES ( 1, 1, 1); -- windsensor and wind_direction
INSERT INTO sensor_parameter VALUES ( 2, 1, 2); -- windsensor and gust_windspeed
INSERT INTO sensor_parameter VALUES ( 3, 1, 3); -- windsensor and average_windspeed
INSERT INTO sensor_parameter VALUES ( 4, 1, 4); -- windsensor and wind_chill
INSERT INTO sensor_parameter VALUES ( 5, 2, 5); -- rainsensor and current_rainrate 
INSERT INTO sensor_parameter VALUES ( 6, 2, 6); -- rainsensor and total_rainfall
INSERT INTO sensor_parameter VALUES ( 7, 2, 7); -- rainsensor and yesterday_rainfall 
INSERT INTO sensor_parameter VALUES ( 8, 2, 8); -- rainsensor and total_startdate 
INSERT INTO sensor_parameter VALUES ( 9, 3, 9); -- th_sensor and temperature
INSERT INTO sensor_parameter VALUES ( 10, 3, 10); -- th_sensor and humidity
INSERT INTO sensor_parameter VALUES ( 11, 3, 11); -- th_sensor and dew_temperature
INSERT INTO sensor_parameter VALUES ( 12, 4, 9); -- mushroom_sensor and temperature
INSERT INTO sensor_parameter VALUES ( 13, 4, 10); -- mushroom_sensor and humidity
INSERT INTO sensor_parameter VALUES ( 14, 4, 11); -- mushroom_sensor and dew_temperature
INSERT INTO sensor_parameter VALUES ( 15, 5, 9); -- t_sensor and temperature
INSERT INTO sensor_parameter VALUES ( 17, 6, 9); -- thb_sensor and temperature
INSERT INTO sensor_parameter VALUES ( 18, 6, 10); -- thb_sensor and humidity
INSERT INTO sensor_parameter VALUES ( 19, 6, 11); -- thb_sensor and dew_temperature
INSERT INTO sensor_parameter VALUES ( 20, 6, 12); -- thb_sensor and barometric_pressure
INSERT INTO sensor_parameter VALUES ( 21, 6, 13); -- thb_sensor and sealevel_offset
INSERT INTO sensor_parameter VALUES ( 22, 7, 9); -- thb_new_sensor and temperature
INSERT INTO sensor_parameter VALUES ( 23, 7, 10); -- thb_new_sensor and humidity
INSERT INTO sensor_parameter VALUES ( 24, 7, 11); -- thb_new_sensor and dew_temperature
INSERT INTO sensor_parameter VALUES ( 25, 7, 12); -- thb_new_sensor and barometric_pressure
INSERT INTO sensor_parameter VALUES ( 26, 7, 13); -- thb_new_sensor and sealevel_offset

--
-- Table statusname
-- -------------------
-- contains status of sensor, e.g.
-- - over/underrange 
-- - low battery
-- 
CREATE TABLE statusname
    (
      status_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      status_name VARCHAR(64) NOT NULL,
      status_description VARCHAR(128) NOT NULL,
    );
INSERT INTO statusname VALUES ( 1, 'Windgust overrange', '');
INSERT INTO statusname VALUES ( 2, 'Average windspeed overrange', '');
INSERT INTO statusname VALUES ( 3, 'Low battery', 'Battery status' );
INSERT INTO statusname VALUES ( 4, 'Chill nodata', '');
INSERT INTO statusname VALUES ( 5, 'Chill overrange', '');
INSERT INTO statusname VALUES ( 6, 'Rainrate overrate', '' );
INSERT INTO statusname VALUES ( 7, 'Raintotal overrange', '' );
INSERT INTO statusname VALUES ( 8, 'Yesterday overrange', '' );
INSERT INTO statusname VALUES ( 9, 'Channel number', '' );
INSERT INTO statusname VALUES ( 10, 'Dew underrange', '' );
INSERT INTO statusname VALUES ( 11, 'Over/underrange', '' );
INSERT INTO statusname VALUES ( 12, 'Weather status' );


-- Table sensor_status
-- -----------------------
-- relation of table sensorname to table statusname
-- 
CREATE TABLE sensor_status
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      status_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
      FOREIGN KEY (status_no) REFERENCES statusname (status_no)
    );
INSERT INTO sensor_status VALUES ( 1, 1, 1); -- windsensor and windgust_overrange
INSERT INTO sensor_status VALUES ( 2, 1, 2); -- windsensor and averagewindspeed_overrange
INSERT INTO sensor_status VALUES ( 3, 1, 3); -- windsensor and low_battery
INSERT INTO sensor_status VALUES ( 4, 1, 4); -- windsensor and chill_nodata
INSERT INTO sensor_status VALUES ( 4, 1, 5); -- windsensor and chill_overrange
INSERT INTO sensor_status VALUES ( 5, 2, 6); -- rainsensor and rainrate_overrange 
INSERT INTO sensor_status VALUES ( 6, 2, 7); -- rainsensor and raintotal_overrange
INSERT INTO sensor_status VALUES ( 7, 2, 3); -- rainsensor and low_battery 
INSERT INTO sensor_status VALUES ( 8, 2, 8); -- rainsensor and yesterday_overrange 
INSERT INTO sensor_status VALUES ( 9, 3, 9);  -- th_sensor and channel_no
INSERT INTO sensor_status VALUES ( 10, 3, 10);  -- th_sensor and dew_underrange
INSERT INTO sensor_status VALUES ( 11, 3, 3);  -- th_sensor and low_battery
INSERT INTO sensor_status VALUES ( 12, 3, 11); -- th_sensor and over_underrange
INSERT INTO sensor_status VALUES ( 13, 4, 10); -- mushroom_sensor and dew_underrange
INSERT INTO sensor_status VALUES ( 14, 4, 3);  -- mushroom_sensor and low_battery
INSERT INTO sensor_status VALUES ( 15, 4, 11); -- mushroom_sensor and over_underrange
INSERT INTO sensor_status VALUES ( 16, 5, 9);  -- t_sensor and channel_no
INSERT INTO sensor_status VALUES ( 17, 5, 3);  -- t_sensor and low_battery
INSERT INTO sensor_status VALUES ( 18, 5, 11); -- t_sensor and over_underrange
INSERT INTO sensor_status VALUES ( 19, 6, 10);  -- thb_sensor and dew_underrange
INSERT INTO sensor_status VALUES ( 20, 6, 3);  -- thb_sensor and low_battery
INSERT INTO sensor_status VALUES ( 21, 6, 11); -- thb_sensor and over_underrange
INSERT INTO sensor_status VALUES ( 22, 6, 12); -- thb_sensor and weather_status
INSERT INTO sensor_status VALUES ( 23, 7, 10); -- thb_new_sensor and dew_underrange
INSERT INTO sensor_status VALUES ( 24, 7, 3);  -- thb_new_sensor and low_battery
INSERT INTO sensor_status VALUES ( 25, 7, 11); -- thb_new_sensor and over_underrange
INSERT INTO sensor_status VALUES ( 26, 7, 12); -- thb_new_sensor and weather_status

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
