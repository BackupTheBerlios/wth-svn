--
-- umeter_db.sql
--
-- create SQLITE database for ULTIMETER weatherstation
--
--
-- SQLITE Database description for 
--   Peetbros ULTIMETER Weatherstation
--
-- includes 
--   all possible sensors
--   parameters of datalogger and 
--   parameters of packet mode
--   parameters of completerecord mode
--
-- Copyright (C) 2010, 2011 Volker Jahns, volker@thalreit.de
--

--
-- table sensordata holds all measured values
--
CREATE TABLE sensordata
    (
      dataset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      dataset_date DATE NOT NULL,
      sensor_meas_no INTEGER NOT NULL,
      meas_value FLOAT NOT NULL,
      FOREIGN KEY (sensor_meas_no) REFERENCES sensor_parameter (sensor_meas_no)
    );

CREATE INDEX sdidx2 ON sensordata(dataset_date);


--
-- table sensornames with all sensors
--
CREATE TABLE sensornames
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_name VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL
    );

-- Wind Sensor
INSERT INTO sensornames VALUES ('1','WindSensor','Wind Sensor');

-- Temperature Sensor
INSERT INTO sensornames VALUES ('2','TemperatureSensor', 'Temperature Sensor');

-- Rain Gauge
INSERT INTO sensornames VALUES ('3','RainGauge', 'Rain Sensor');

-- Indoor Humidity Sensor ( optional)
INSERT INTO sensornames VALUES ('4','IndoorHumiditySensor', 'Indoor Humidity Sensor');

-- T/H Sensor (optional)
INSERT INTO sensornames VALUES ('5','OutdoorHumidityTemperatureSensor','Outdoor Humitity/Temperature Sensor');

-- Indoor T/Pressure (inside display unit)
INSERT INTO sensornames VALUES ('6','IndoorTemperatorBarometerSensor','Indoor Temperature/Barometer Sensor in central display unit');


--
-- table parameternames with all physical parameters
--
CREATE TABLE parameternames
  (
    parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    parameter_name VARCHAR(64) NOT NULL,
    unit VARCHAR(64) NOT NULL,
    gain FLOAT NOT NULL,
    offset FLOAT NOT NULL
  );
-- datalogger mode parameters
INSERT INTO parameternames VALUES ('1','Wind Speed','m s-1', 1.000, 0.000);
INSERT INTO parameternames VALUES ('2','Wind Direction','deg', 1.000, 0.000);
INSERT INTO parameternames VALUES ('3','Outdoor Temperature','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('4','Rain Longterm Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('5','Barometer','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('6','Indoor Temperature','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('7','Outdoor Humidity','% rel.hum.', 1.000, 0.000);
INSERT INTO parameternames VALUES ('8','Indoor Humidity','% rel.hum', 1.000, 0.000);
INSERT INTO parameternames VALUES ('9','Rain Today Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('10','Wind Speed 1 Minute Average','m s-1', 1.000, 0.000);
-- packet mode parameters
INSERT INTO parameternames VALUES ('11','Wind Speed 5 Minute Peak','m s-1', 1.000, 0.000);
INSERT INTO parameternames VALUES ('12','Wind Direction of Wind Speed Peak','deg', 1.000, 0.000);
INSERT INTO parameternames VALUES ('13','Current Outdoor Temperature','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('14','Current Barometer','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('15','Barometer Delta','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('16','Barometer Correction Factor','', 1.000, 0.000);
INSERT INTO parameternames VALUES ('17','Current Outdoor Humidity','% rel.hum.', 1.000, 0.000);
INSERT INTO parameternames VALUES ('18','Wind Speed 5 Minute Average','m s-1', 1.000, 0.000);
-- completerecord mode parameters
INSERT INTO parameternames VALUES ('19','Wind Direction of 5 Minute Wind Speed Peak','deg', 1.000, 0.000); -- might be the same as packetmode paraneter 12
INSERT INTO parameternames VALUES ('20','Barometer 3-Hour Pressure Change','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('21','Wind Chill','deg C', 1.000, 0.000); -- unclear, which T-sensor is used to calculate wind chill
INSERT INTO parameternames VALUES ('22','Dew Point','deg C', 1.000, 0.000); -- unclear, which HT-sensor is used to calculate dew point
--
-- relation of parameter to sensor
-- i.e which physical parameter is measured in which sensor 
--
CREATE TABLE sensorparameters
  (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no),
      FOREIGN KEY (parameter_no) REFERENCES parameternames (parameter_no)
  );


INSERT INTO sensorparameters VALUES ('1','1','1');   -- Windsensor and Wind Speed
INSERT INTO sensorparameters VALUES ('2','1','2');   -- Windsensor and Wind Direction 
INSERT INTO sensorparameters VALUES ('3','1','10');  -- Windsensor and 1 Minute Windspeed Average 
INSERT INTO sensorparameters VALUES ('4','1','11');  -- Windsensor and Windspeed Peak 5 Minute
INSERT INTO sensorparameters VALUES ('5','1','12');  -- Windsensor and Direction of Windspeed Peak
INSERT INTO sensorparameters VALUES ('6','1','18');  -- Windsensor and 5 Minute Windspeed Average
INSERT INTO sensorparameters VALUES ('7','1','19');  -- Windsensor and Wind Direction of 5 Minute Windspeed Peak
INSERT INTO sensorparameters VALUES ('8','1','21');  -- Windsensor and Wind Chill

INSERT INTO sensorparameters VALUES ('9','2','3');   -- Temperatursensor and Outdoor Temp
INSERT INTO sensorparameters VALUES ('10','2','13'); -- Temperatursensor and Current Outdoor Temp

INSERT INTO sensorparameters VALUES ('11','3','4');  -- Raingauge and Rain Long Term Total
INSERT INTO sensorparameters VALUES ('12','3','9');  -- Raingauge and Rain Today Total

INSERT INTO sensorparameters VALUES ('13','4','8');  -- Indoor Humiditysensor and Indoor Humidity

INSERT INTO sensorparameters VALUES ('14','5','3');  -- Outdoor H/T Sensor and Outdoor Temp
INSERT INTO sensorparameters VALUES ('15','5','7');  -- Outdoor H/T Sensor and Outdoor Humidity
INSERT INTO sensorparameters VALUES ('16','5','13'); -- Outdoor H/T Sensor and Current Outdoor Temp
INSERT INTO sensorparameters VALUES ('17','5','17'); -- Outdoor H/T Sensor and Current Outdoor Humidity
INSERT INTO sensorparameters VALUES ('18','5','22'); -- Outdoor H/T Sensor and Dew Point

INSERT INTO sensorparameters VALUES ('19','6','5');  -- Indoor T/Pressure sensor and Barometer
INSERT INTO sensorparameters VALUES ('20','6','6');  -- Indoor T/Pressure sensor and Indoor Temperature
INSERT INTO sensorparameters VALUES ('21','6','14'); -- Indoor T/Pressure sensor and Current Barometer
INSERT INTO sensorparameters VALUES ('22','6','15'); -- Indoor T/Pressure sensor and Barometer Delta
INSERT INTO sensorparameters VALUES ('23','6','16'); -- Indoor T/Pressure sensor and Barometer Correction Factor
INSERT INTO sensorparameters VALUES ('24','6','20'); -- Indoor T/Pressure sensor and Barometer 3-Hour Pressure Change

--
-- table sensorupdate
--
CREATE TABLE sensorupdate
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      last_update DATE NOT NULL
    );

INSERT INTO sensorupdate VALUES ('1','0');
INSERT INTO sensorupdate VALUES ('2','0');
INSERT INTO sensorupdate VALUES ('3','0');
INSERT INTO sensorupdate VALUES ('4','0');
INSERT INTO sensorupdate VALUES ('5','0');

INSERT INTO sensorupdate VALUES ('6','0');
INSERT INTO sensorupdate VALUES ('7','0');
INSERT INTO sensorupdate VALUES ('8','0');
INSERT INTO sensorupdate VALUES ('9','0');
INSERT INTO sensorupdate VALUES ('10','0');

INSERT INTO sensorupdate VALUES ('11','0');
INSERT INTO sensorupdate VALUES ('12','0');
INSERT INTO sensorupdate VALUES ('13','0');
INSERT INTO sensorupdate VALUES ('14','0');
INSERT INTO sensorupdate VALUES ('15','0');

INSERT INTO sensorupdate VALUES ('16','0');
INSERT INTO sensorupdate VALUES ('17','0');
INSERT INTO sensorupdate VALUES ('18','0');
INSERT INTO sensorupdate VALUES ('19','0');
INSERT INTO sensorupdate VALUES ('20','0');
INSERT INTO sensorupdate VALUES ('21','0');
INSERT INTO sensorupdate VALUES ('22','0');
INSERT INTO sensorupdate VALUES ('23','0');
INSERT INTO sensorupdate VALUES ('24','0');



