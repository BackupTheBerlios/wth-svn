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
-- 1		gust_over		sign		
-- 2		average_over		sign
-- 3		low_battery		sign
-- 4		wind_direction		degree		1.000	0.000
-- 5		gust_windspeed		m/sec^2		1.000	0.000
-- 6		average_windspeed	m/sec^2		1.000	0.000
-- 7		chill_nodata		sign
-- 8		chill_over		sign
-- 9		windchill		degC		1.000	0.000
-- 10		rate_over		sign
-- 11		total_over		sign
-- 12		yesterday_over		sign
-- 13		current_rainrate	mm/hr		1.000	0.000
-- 14		total_rainfall		mm		1.000	0.000
-- 15		yesterday_rainfall	mm		1.000	0.000
-- 16		total_start		dd/mm/yyyy
-- 17		channel_no
-- 18		dew_under		sign
-- 19		temperature		degC		1.000	0.000
-- 20		over_under		sign
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
INSERT INTO parametername VALUES ( 16, 'Total start', 'dd/mm/yyyy' );
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
INSERT INTO sensordevparameters VALUES ( 6, 1, 2, 1); -- HTSsensor and DS1820 and Temperature

INSERT INTO sensordevparameters VALUES ( 7, 2, 4, 1); -- Barometersensor and DS2438 and Temperature
INSERT INTO sensordevparameters VALUES ( 8, 2, 4, 4); -- Barometersensor and DS2438 and VSENS
INSERT INTO sensordevparameters VALUES ( 9, 2, 4, 2); -- Barometersensor and DS2438 and VAD
INSERT INTO sensordevparameters VALUES (10, 2, 4, 3); -- Barometersensor and DS2438 and VDD
INSERT INTO sensordevparameters VALUES (11, 2, 4, 7); -- Barometersensor and DS2438 and Pressure
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
