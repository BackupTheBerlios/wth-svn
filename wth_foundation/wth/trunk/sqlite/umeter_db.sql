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
CREATE TABLE sensorname
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_name VARCHAR(255) NOT NULL
    );

INSERT INTO sensorname VALUES ('1','WindSensor');                        -- Wind Sensor
INSERT INTO sensorname VALUES ('2','TemperatureSensor');                 -- Temperature Sensor
INSERT INTO sensorname VALUES ('3','RainGauge');                         -- Rain Gauge
INSERT INTO sensorname VALUES ('4','IndoorHumiditySensor');              -- Indoor Humidity Sensor ( optional)
INSERT INTO sensorname VALUES ('5','OutdoorHumidity_TemperatureSensor'); -- T/H Sensor (optional)
INSERT INTO sensorname VALUES ('6','IndoorTemperator_BarometerSensor');  -- Indoor T/Pressure (inside display unit)


--
-- table parameternames with all physical parameters
--
CREATE TABLE parametername
  (
    parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    parameter_name VARCHAR(64) NOT NULL,
    parameter_unit VARCHAR(64) NOT NULL
  );
-- datalogger mode parameters
INSERT INTO parametername VALUES ('1','Wind Speed','m s-1');
INSERT INTO parametername VALUES ('2','Wind Direction','deg');
INSERT INTO parametername VALUES ('3','Outdoor Temp','deg C');
INSERT INTO parametername VALUES ('4','Rain Longterm Total','mm');
INSERT INTO parametername VALUES ('5','Barometer','hPa');
INSERT INTO parametername VALUES ('6','Indoor temp','deg C');
INSERT INTO parametername VALUES ('7','Outdoor Humidity','% rel.hum.');
INSERT INTO parametername VALUES ('8','Indoor Humidity','% rel.hum');
INSERT INTO parametername VALUES ('9','Today Rain Total','mm');
INSERT INTO parametername VALUES ('10','1 Minute Windspeed Average','m s-1');
-- packetmode parameters
INSERT INTO parametername VALUES ('11','Windspeed Peak 5 Minute','m s-1');
INSERT INTO parametername VALUES ('12','Direction of Windspeed Peak','deg');
INSERT INTO parametername VALUES ('13','Current Outdoor Temp','deg C');

INSERT INTO parametername VALUES ('14','Current Barometer','hPa');
INSERT INTO parametername VALUES ('15','Barometer Delta','hPa');
INSERT INTO parametername VALUES ('16','Barometer Correction Factor','');
INSERT INTO parametername VALUES ('17','Current Outdoor Humidity','% rel.hum.');
INSERT INTO parametername VALUES ('18','5 Minute Windspeed Average','m s-1');


--
-- relation of parameter to sensor
-- i.e which physical parameter is measured in which sensor 
--
CREATE TABLE sensor_parameter
  (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (parameter_no) REFERENCES parametername (parameter_no),
      FOREIGN KEY (sensor_no) REFERENCES sensorname (sensor_no)
  );


INSERT INTO sensor_parameter VALUES ('1','1','1');   -- Windsensor and Wind Speed
INSERT INTO sensor_parameter VALUES ('2','1','2');   -- Windsensor and Wind Direction 
INSERT INTO sensor_parameter VALUES ('3','1','10');  -- Windsensor and 1 Minute Windspeed Average 
INSERT INTO sensor_parameter VALUES ('4','1','11');  -- Windsensor and Windspeed Peak 5 Minute
INSERT INTO sensor_parameter VALUES ('5','1','12');  -- Windsensor and Direction of Windspeed Peak
INSERT INTO sensor_parameter VALUES ('6','1','18');  -- Windsensor and 5 Minute Windspeed Average

INSERT INTO sensor_parameter VALUES ('7','2','3');   -- Temperatursensor and Outdoor Temp
INSERT INTO sensor_parameter VALUES ('8','2','13');  -- Temperatursensor and Current Outdoor Temp

INSERT INTO sensor_parameter VALUES ('9','3','4');   -- Raingauge and Rain Long Term Total
INSERT INTO sensor_parameter VALUES ('10','3','9');  -- Raingauge and Today Rain Total

INSERT INTO sensor_parameter VALUES ('11','4','8');  -- Indoor Humiditysensor and Indoor Humidity

INSERT INTO sensor_parameter VALUES ('12','5','3');  -- Outdoor H/T Sensor and Outdoor Temp
INSERT INTO sensor_parameter VALUES ('13','5','7');  -- Outdoor H/T Sensor and Outdoor Humidity
INSERT INTO sensor_parameter VALUES ('14','5','13'); -- Outdoor H/T Sensor and Current Outdoor Temp
INSERT INTO sensor_parameter VALUES ('15','5','17'); -- Outdoor H/T Sensor and Current Outdoor Humidity

INSERT INTO sensor_parameter VALUES ('16','6','5');  -- Indoor T/Pressure sensor and Barometer
INSERT INTO sensor_parameter VALUES ('17','6','6');  -- Indoor T/Pressure sensor and Indoor Temperature
INSERT INTO sensor_parameter VALUES ('18','6','14'); -- Indoor T/Pressure sensor and Current Barometer
INSERT INTO sensor_parameter VALUES ('19','6','15'); -- Indoor T/Pressure sensor and Barometer Delta
INSERT INTO sensor_parameter VALUES ('20','6','16'); -- Indoor T/Pressure sensor and Barometer Correction Factor

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

