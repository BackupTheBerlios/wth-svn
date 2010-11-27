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
--
-- Copyright (C) 2010 Volker Jahns, volker@thalreit.de
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
INSERT INTO sensornames VALUES ('5','OutdoorHumidity_TemperatureSensor','Outdoor Humitity/Temperature Sensor');

-- Indoor T/Pressure (inside display unit)
INSERT INTO sensornames VALUES ('6','IndoorTemperator_BarometerSensor','Indoor Temperature/Barometer Sensor in central display unit');


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
INSERT INTO parameternames VALUES ('3','Outdoor Temp','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('4','Rain Longterm Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('5','Barometer','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('6','Indoor temp','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('7','Outdoor Humidity','% rel.hum.', 1.000, 0.000);
INSERT INTO parameternames VALUES ('8','Indoor Humidity','% rel.hum', 1.000, 0.000);
INSERT INTO parameternames VALUES ('9','Today Rain Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('10','1 Minute Windspeed Average','m s-1', 1.000, 0.000);
-- packetmode parameters
INSERT INTO parameternames VALUES ('11','Windspeed Peak 5 Minute','m s-1', 1.000, 0.000);
INSERT INTO parameternames VALUES ('12','Direction of Windspeed Peak','deg', 1.000, 0.000);
INSERT INTO parameternames VALUES ('13','Current Outdoor Temp','deg C', 1.000, 0.000);

INSERT INTO parameternames VALUES ('14','Current Barometer','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('15','Barometer Delta','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('16','Barometer Correction Factor','', 1.000, 0.000);
INSERT INTO parameternames VALUES ('17','Current Outdoor Humidity','% rel.hum.', 1.000, 0.000);
INSERT INTO parameternames VALUES ('18','5 Minute Windspeed Average','m s-1', 1.000, 0.000);


--
-- relation of parameter to sensor
-- i.e which physical parameter is measured in which sensor 
--
CREATE TABLE sensorparameters
  (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no)
      FOREIGN KEY (parameter_no) REFERENCES parameternames (parameter_no),
  );


INSERT INTO sensorparameters VALUES ('1','1','1');   -- Windsensor and Wind Speed
INSERT INTO sensorparameters VALUES ('2','1','2');   -- Windsensor and Wind Direction 
INSERT INTO sensorparameters VALUES ('3','1','10');  -- Windsensor and 1 Minute Windspeed Average 
INSERT INTO sensorparameters VALUES ('4','1','11');  -- Windsensor and Windspeed Peak 5 Minute
INSERT INTO sensorparameters VALUES ('5','1','12');  -- Windsensor and Direction of Windspeed Peak
INSERT INTO sensorparameters VALUES ('6','1','18');  -- Windsensor and 5 Minute Windspeed Average

INSERT INTO sensorparameters VALUES ('7','2','3');   -- Temperatursensor and Outdoor Temp
INSERT INTO sensorparameters VALUES ('8','2','13');  -- Temperatursensor and Current Outdoor Temp

INSERT INTO sensorparameters VALUES ('9','3','4');   -- Raingauge and Rain Long Term Total
INSERT INTO sensorparameters VALUES ('10','3','9');  -- Raingauge and Today Rain Total

INSERT INTO sensorparameters VALUES ('11','4','8');  -- Indoor Humiditysensor and Indoor Humidity

INSERT INTO sensorparameters VALUES ('12','5','3');  -- Outdoor H/T Sensor and Outdoor Temp
INSERT INTO sensorparameters VALUES ('13','5','7');  -- Outdoor H/T Sensor and Outdoor Humidity
INSERT INTO sensorparameters VALUES ('14','5','13'); -- Outdoor H/T Sensor and Current Outdoor Temp
INSERT INTO sensorparameters VALUES ('15','5','17'); -- Outdoor H/T Sensor and Current Outdoor Humidity

INSERT INTO sensorparameters VALUES ('16','6','5');  -- Indoor T/Pressure sensor and Barometer
INSERT INTO sensorparameters VALUES ('17','6','6');  -- Indoor T/Pressure sensor and Indoor Temperature
INSERT INTO sensorparameters VALUES ('18','6','14'); -- Indoor T/Pressure sensor and Current Barometer
INSERT INTO sensorparameters VALUES ('19','6','15'); -- Indoor T/Pressure sensor and Barometer Delta
INSERT INTO sensorparameters VALUES ('20','6','16'); -- Indoor T/Pressure sensor and Barometer Correction Factor

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

