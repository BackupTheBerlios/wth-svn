--
-- Datastructures of 1-wire sensors
-- 
-- sample description of table contents
--
-- Sensor#	Sensorname		Description		Sensortyp	Familycode	Serialnumber
-- -------	----------		-----------		---------	----------	------------
-- 1		Barometersensor		Indoor pressure		DS2438		26		01:08:37:d9:07:00:00:a1
-- 2		Temperaturesensor	Indoor temperature
-- 3		Humiditysensor		Indoor humidity
-- 4		Humiditysensor		Outdoor humidity
-- 5		Radiationsensor		Global radiation
-- 
-- Parameter#	Parametername		Unit	Gain	Offset
-- ----------	-------------		----	----	------
-- 1		Pressure		hPa	1.000	0.000
-- 2		Temperature		degC	1.000	0.000
-- 3		VAD			V	1.000	0.000
-- 4		VSENS+			mV	1.000	0.000
--
-- 1-Wire database schema
-- ----------------------
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
INSERT INTO parametername VALUES (1, 'Temperature', degC,  1.000, 0.000);
INSERT INTO parametername VALUES (2, 'VAD', 'V', 1.000, 0.000);
INSERT INTO parametername VALUES (3, 'VDD', 'V', 1.000, 0.000);
INSERT INTO parametername VALUES (4, 'VSENS+', 'mV', 1.000, 0.000);
INSERT INTO parametername VALUES (5, 'VSENS-', 'mV', 1.000, 0.000);
INSERT INTO parametername VALUES (6, 'Pressure', 'hPa', 1.000, 0.000);
INSERT INTO parametername VALUES (7, 'Radiation power', 'W m-2', 1.000, 0.000);
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
-- Table sensorname
-- -----------------
-- contains name and serial number of sensor
-- 
CREATE TABLE sensorname
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_name VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL,
      hardware VARCHAR(255) NOT NULL,
      familycode VARCHAR(8) NOT NULL,
      serialnum VARCHAR(255) NOT NULL
    );
INSERT INTO sensorname VALUES ( 1, 'Barometersensor', 'Pressure Indoorsensor based on Motorola MPX4115A using the DS2438 10V VAD reading','DS2438', '26', '01:08:37:d9:07:00:00:a1');
-- 
-- Table sensorparameters
-- -----------------------
-- relation of table sensorname to table parametername
-- 
CREATE TABLE sensorparameters
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no),
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
    );
INSERT INTO sensorparameters VALUES ( 1, 1, 2); -- Barometersensor and VAD
INSERT INTO sensorparameters VALUES ( 2, 1, 3); -- Barometersensor and VDD
INSERT INTO sensorparameters VALUES ( 3, 1, 4); -- Barometersensor and VSENS+
INSERT INTO sensorparameters VALUES ( 4, 1, 5); -- Barometersensor and VSENS-
INSERT INTO sensorparameters VALUES ( 5, 1, 6); -- Barometersensor and Pressure
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
