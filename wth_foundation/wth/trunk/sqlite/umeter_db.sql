--
-- umeter_db.sql
--
-- create SQLITE database for ULTIMETER weatherstation
--
--
--
-- SQLITE Database description for 
--   Peetbros ULTIMETER Weatherstation
--
--
-- Copyright (C) 2010 Volker Jahns, volker@thalreit.de
--

CREATE TABLE sensordata
    (
      dataset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      dataset_date DATE NOT NULL,
      sensor_meas_no INTEGER NOT NULL,
      meas_value FLOAT NOT NULL,
      FOREIGN KEY (sensor_meas_no) REFERENCES sensorparameters (sensor_meas_no)
    );

CREATE INDEX sdidx2 ON sensordata(dataset_date);

CREATE TABLE sensorstatus
    (
      statusset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      statusset_date DATE NOT NULL,
      sensor_no INTEGER NOT NULL,
      sensor_status INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no)
    );

CREATE INDEX ssidx ON sensorstatus(sensor_no, statusset_date,sensor_status);

CREATE TABLE sensorupdate
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      last_update DATE NOT NULL
    );

CREATE TABLE sensornames
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensorname VARCHAR(255) NOT NULL
    );

INSERT INTO sensornames VALUES ('1','Wind Sensor');                          -- Wind Sensor
INSERT INTO sensornames VALUES ('2','Temperature Sensor');                   -- Temperature Sensor
INSERT INTO sensornames VALUES ('3','Rain Gauge');                           -- Rain Gauge
INSERT INTO sensornames VALUES ('4','Indoor Humidity Sensor');               -- Indoor Humidity Sensor ( optional)
INSERT INTO sensornames VALUES ('5','Outdoor Humidity/Temperature Sensor');  -- T/H Sensor (optional)
INSERT INTO sensornames VALUES ('6','Indoor Temperator/Barometer Sensor');   -- Indoor T/Pressure (inside display unit)

CREATE TABLE parameternames
  (
    parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    parameter_name VARCHAR(64) NOT NULL,
    parameter_unit VARCHAR(64) NOT NULL
  );
-- datalogger mode parameters
INSERT INTO parameternames VALUES ('1','Wind Speed','m s-1');
INSERT INTO parameternames VALUES ('2','Wind Direction','deg');
INSERT INTO parameternames VALUES ('3','Outdoor Temp','deg C');
INSERT INTO parameternames VALUES ('4','Rain Longterm Total','mm');
INSERT INTO parameternames VALUES ('5','Barometer','hPa');
INSERT INTO parameternames VALUES ('6','Indoor temp','deg C');
INSERT INTO parameternames VALUES ('7','Outdoor Humidity','% rel.hum.');
INSERT INTO parameternames VALUES ('8','Indoor Humidity','% rel.hum');
INSERT INTO parameternames VALUES ('9','Today Rain Total','mm');
INSERT INTO parameternames VALUES ('10','1 Minute Windspeed Average','m s-1');
-- packetmode parameters
INSERT INTO parameternames VALUES ('11','Windspeed Peak 5 Minute','m s-1');
INSERT INTO parameternames VALUES ('12','Direction of Windspeed Peak','deg');
INSERT INTO parameternames VALUES ('13','Current Outdoor Temp','deg C');

INSERT INTO parameternames VALUES ('14','Current Barometer','hPa');
INSERT INTO parameternames VALUES ('15','Barometer Delta','hPa');
INSERT INTO parameternames VALUES ('16','Barometer Correction Factor','');
INSERT INTO parameternames VALUES ('17','Current Outdoor Humidity','% rel.hum.');
INSERT INTO parameternames VALUES ('18','5 Minute Windspeed Average','m s-1');



CREATE TABLE sensorparameters
  (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no),
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no)
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

INSERT INTO sensorupdate VALUES ('1','0');
INSERT INTO sensorupdate VALUES ('2','0');


