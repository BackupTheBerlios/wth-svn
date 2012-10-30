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
-- Table sensornames
-- -----------------
-- contains name and serial number of sensor
-- 



CREATE TABLE sensornames
    (
      sensor_no SERIAL PRIMARY KEY,
      sensor_name VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL
    );
INSERT INTO sensornames VALUES ( 1, 'HTSsensor', 'Humidity, temperature and solar sensor based on HIH-4021 humidity sensor solar detector detects light in the visible to infrared portion of the spectrum, with peak detection at 850nm and an acceptance angle of 150 degrees');
INSERT INTO sensornames VALUES ( 2, 'Barometersensor', 'Pressure Indoorsensor based on Motorola MPX4115A using the DS2438 10V VAD reading');
--
-- Table devicetyp
-- ---------------
-- contains devicetyp, description, familycode and serialnumber
--
CREATE TABLE devicetyp
    (
      device_no SERIAL PRIMARY KEY,
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
-- Table parameternames
-- --------------------
-- contains name und unit of parameter, e.g. 
-- - pressure 
-- - mbar
-- 
CREATE TABLE parameternames
    (
      param_no SERIAL PRIMARY KEY,
      param_name VARCHAR(64) NOT NULL,
      param_description VARCHAR(1024) NOT NULL,
      param_unit VARCHAR(64) NOT NULL,
      param_gain FLOAT NOT NULL,
      param_offset FLOAT NOT NULL
    );
INSERT INTO parameternames VALUES (1, 'Temperature', 'Temperatur', '°C',  1.000, 0.000);
INSERT INTO parameternames VALUES (2, 'VAD', 'V AD', 'V', 1.000, 0.000);
INSERT INTO parameternames VALUES (3, 'VDD', 'V DD', 'V', 1.000, 0.000);
INSERT INTO parameternames VALUES (4, 'VSENS+', 'VSENS+', 'mV', 1.000, 0.000);
INSERT INTO parameternames VALUES (5, 'VSENS-', 'VSENS-', 'mV', 1.000, 0.000);
INSERT INTO parameternames VALUES (6, 'Humidity', 'Humidity', '%', 1.000, 0.000);
INSERT INTO parameternames VALUES (7, 'Pressure', 'Pressure', 'hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES (8, 'Radiation power', 'Radiation power', 'W/m²', 1.000, 0.000);
-- 
-- Table sensordeviceparameters
-- -----------------------
-- relation of table sensorname to table devicetyp and table parametername
-- 
CREATE TABLE sensorparameters
    (
      sensor_meas_no SERIAL PRIMARY KEY,
      sensor_no INTEGER NOT NULL REFERENCES sensornames(sensor_no),
      device_no INTEGER NOT NULL REFERENCES devicetyp(device_no),
      param_no INTEGER NOT NULL REFERENCES parameternames(param_no)
--      FOREIGN KEY (device_no) REFERENCES devicetyp (device_no),
--      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
--      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no)
    );
INSERT INTO sensorparameters VALUES ( 1, 1, 1, 1); -- HTSsensor and DS2438 and Temperature
INSERT INTO sensorparameters VALUES ( 2, 1, 1, 4); -- HTSsensor and DS2438 and VSENS
INSERT INTO sensorparameters VALUES ( 3, 1, 1, 2); -- HTSsensor and DS2438 and VAD
INSERT INTO sensorparameters VALUES ( 4, 1, 1, 3); -- HTSsensor and DS2438 and VDD
INSERT INTO sensorparameters VALUES ( 5, 1, 1, 6); -- HTSsensor and DS2438 and Humidity
INSERT INTO sensorparameters VALUES ( 6, 1, 2, 1); -- HTSsensor and DS1820 and Temperature

INSERT INTO sensorparameters VALUES ( 7, 2, 4, 1); -- Barometersensor and DS2438 and Temperature
INSERT INTO sensorparameters VALUES ( 8, 2, 4, 4); -- Barometersensor and DS2438 and VSENS
INSERT INTO sensorparameters VALUES ( 9, 2, 4, 2); -- Barometersensor and DS2438 and VAD
INSERT INTO sensorparameters VALUES (10, 2, 4, 3); -- Barometersensor and DS2438 and VDD
INSERT INTO sensorparameters VALUES (11, 2, 4, 7); -- Barometersensor and DS2438 and Pressure
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
      dataset_no SERIAL PRIMARY KEY,
      dataset_date TIMESTAMP NOT NULL,
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
      sensor_meas_no SERIAL PRIMARY KEY,
      last_update TIMESTAMP NOT NULL
    );
INSERT INTO sensorupdate VALUES (1, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (2, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (3, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (4, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (5, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (6, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (7, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (8, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (9, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (10, TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES (11, TO_TIMESTAMP(0));
