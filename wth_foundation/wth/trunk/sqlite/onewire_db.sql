--
-- Datastructures of 1-wire sensors
-- 
-- sample description of table contents
--
-- Sensor#	Sensorname		Description		
-- -------	----------		-----------	
-- 1		HTSsensor		Humidity/Temp/Solar
-- 2		Barometersensor		Barometer
-- 3		Humiditysensor		Indoor humidity
-- 4		Humiditysensor		Outdoor humidity
-- 5		Radiationsensor		Global radiation
-- 
-- Device#	Devicetyp	Familycode	Serialnum
-- -------	---------	----------	---------
-- 1		DS2438		26		D0000000A9192626
-- 2		DS1820		10		370008014D32A110
--
-- Parameter#	Parametername		Unit		Gain	Offset
-- ----------	-------------		----		----	------
-- 1		Pressure		hPa		1.000	0.000
-- 2		Temperature		% rel.hum	1.000	0.000
-- 2		Humidity		degC		1.000	0.000
-- 3		VAD			V		1.000	0.000
-- 4		VSENS+			mV		1.000	0.000
--
-- 1-Wire database schema
-- 
-- Table sensorname
-- -----------------
-- contains name and serial number of sensor
-- 
CREATE TABLE sensorname
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_name VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL
    );
INSERT INTO sensorname VALUES ( 1, 'HTSsensor', 'Humidity, temperature and solar sensor based on HIH-4021 humidity sensor solar detector detects light in the visible to infrared portion of the spectrum, with peak detection at 850nm and an acceptance angle of 150 degrees');
INSERT INTO sensorname VALUES ( 2, 'Barometersensor', 'Pressure Indoorsensor based on Motorola MPX4115A using the DS2438 10V VAD reading');
--
-- Table devicetyp
-- ---------------
-- contains devicetyp, description, familycode and serialnumber
--
CREATE TABLE devicetyp
    (
      device_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      devicetyp VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL,
      familycode VARCHAR(255) NOT NULL,
      serialnum VARCHAR(255) NOT NULL
    );
INSERT INTO devicetyp VALUES ( 1, 'DS2438', 'smart battery monitor', '26', 'D0000000A9192626');
INSERT INTO devicetyp VALUES ( 2, 'DS1920/DS1820', 'high precision digital thermometer', '10', '370008014D32A110');
INSERT INTO devicetyp VALUES ( 3, 'DS2509', '1k add-only memory', '9', '42000004292E4409');
INSERT INTO devicetyp VALUES ( 4, 'DS2438', 'smart battery monitor', '26', 'D4000000AEE92926');

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
INSERT INTO parametername VALUES (1, 'Temperature', 'degC',  1.000, 0.000);
INSERT INTO parametername VALUES (2, 'VAD', 'V', 1.000, 0.000);
INSERT INTO parametername VALUES (3, 'VDD', 'V', 1.000, 0.000);
INSERT INTO parametername VALUES (4, 'VSENS+', 'mV', 1.000, 0.000);
INSERT INTO parametername VALUES (5, 'VSENS-', 'mV', 1.000, 0.000);
INSERT INTO parametername VALUES (6, 'Humidity', '% rel hum', 1.000, 0.000);
INSERT INTO parametername VALUES (7, 'Pressure', 'hPa', 1.000, 0.000);
INSERT INTO parametername VALUES (8, 'Radiation power', 'W m-2', 1.000, 0.000);
-- 
-- Table sensordeviceparameters
-- -----------------------
-- relation of table sensorname to table devicetyp and table parametername
-- 
CREATE TABLE sensordevparameters
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      device_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (device_no) REFERENCES devicetyp (device_no),
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no)
    );
INSERT INTO sensordevparameters VALUES ( 1, 1, 1, 1); -- HTSsensor and DS2438 and Temperature
INSERT INTO sensordevparameters VALUES ( 2, 1, 1, 4); -- HTSsensor and DS2438 and VSENS
INSERT INTO sensordevparameters VALUES ( 3, 1, 1, 2); -- HTSsensor and DS2438 and VAD
INSERT INTO sensordevparameters VALUES ( 4, 1, 1, 3); -- HTSsensor and DS2438 and VDD
INSERT INTO sensordevparameters VALUES ( 5, 1, 1, 6); -- HTSsensor and DS2438 and Humidity
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
