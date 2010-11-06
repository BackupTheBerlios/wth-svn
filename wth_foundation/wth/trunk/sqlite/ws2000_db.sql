--
-- SQLITE Database description for 
-- - WS2000 and Lacrosse Weatherstation
--
--
-- WS2000 weatherstation has
--     - status
--         - DCF
--         - HF
--     - sensors which
--         - measure parameter values
--         - have status
--
-- Copyright (C) 2008 Volker Jahns, volker@thalreit.de
--


--
-- table sensordata holds measured values and corresponding time
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
-- index on dataset_date increases table lookup
--
    CREATE INDEX sdidx2 ON sensordata(dataset_date);

--
-- sensor status
--
    CREATE TABLE sensorstatus
    (
      statusset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      statusset_date DATE NOT NULL,
      sensor_no INTEGER NOT NULL,
      sensor_status INTEGER NOT NULL,
      new_flag INTEGER,
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no)
    );

-- create index to speed up request
  CREATE INDEX ssidx ON sensorstatus(sensor_no, statusset_date,sensor_status);

--
-- status of WS2000 weatherstation
--
    CREATE TABLE ws2000status
    (
      wstatus_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      wstatus_date DATE NOT NULL,
      interval_time INTEGER NOT NULL,
      DCF_status INTEGER NOT NULL,
      DCF_sync INTEGER NOT NULL,
      HF_status INTEGER NOT NULL,
      no_sensors INTEGER NOT NULL
    );

--
-- sensornames
--
    create table sensornames
    (
      sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensorname VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL
    );

--
-- last update time of a sensor parameter
--
    create table sensorupdate
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      last_update DATE NOT NULL
    );

--
-- name and unit of sensor parameter
--
    create table parameternames
    (
      parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      parameter_name VARCHAR(64) NOT NULL,
      parameter_unit VARCHAR(64) NOT NULL
    );

--
-- relation between sensor and its parameters
--
    create table sensorparameters
    (
      sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      sensor_no INTEGER NOT NULL,
      parameter_no INTEGER NOT NULL,
      FOREIGN KEY (parameter_no) REFERENCES parameternames (parameter_no),
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no)
    );

--
-- presetting fixed values
--
--
-- table parameternames will not change (WS2000 can't do more)
--
INSERT INTO parameternames VALUES ('1','Temperature','degC');
INSERT INTO parameternames VALUES ('2','Relative Humidity','per cent');
INSERT INTO parameternames VALUES ('3','Precipitation','mm m-2');
INSERT INTO parameternames VALUES ('4','Windspeed','m s-1');
INSERT INTO parameternames VALUES ('5','Winddrct Variance','deg');
INSERT INTO parameternames VALUES ('6','Winddirection','deg');
INSERT INTO parameternames VALUES ('7','Pressure','hPa');
INSERT INTO parameternames VALUES ('8','Precpt. Intensity','mm h-1');

  --
  -- table sensornames will not change (WS2000 can't do more)
  --
INSERT INTO sensornames VALUES ('1','Sensor1','T/H Sensor');
INSERT INTO sensornames VALUES ('2','Sensor2','T/H Sensor');
INSERT INTO sensornames VALUES ('3','Sensor3','T/H Sensor');
INSERT INTO sensornames VALUES ('4','Sensor4','T/H Sensor');
INSERT INTO sensornames VALUES ('5','Sensor5','T/H Sensor');
INSERT INTO sensornames VALUES ('6','Sensor6','T/H Sensor');
INSERT INTO sensornames VALUES ('7','Sensor7','T/H Sensor');
INSERT INTO sensornames VALUES ('8','Sensor8','T/H Sensor');
INSERT INTO sensornames VALUES ('9','Rainsensor','Rainsensor');
INSERT INTO sensornames VALUES ('10','Windsensor','Windsensor');
INSERT INTO sensornames VALUES ('11','Indoorsensor','Indoor T/H/P Sensor');
INSERT INTO sensornames VALUES ('12','Sensor9','Extra T/H Sensor');
INSERT INTO sensornames VALUES ('13','Sensor10','Extra T/H Sensor');
INSERT INTO sensornames VALUES ('14','Sensor11','Extra T/H Sensor');
INSERT INTO sensornames VALUES ('15','Sensor12','Extra T/H Sensor');
INSERT INTO sensornames VALUES ('16','Sensor13','Extra T/H Sensor');
INSERT INTO sensornames VALUES ('17','Sensor14','Extra T/H Sensor');
INSERT INTO sensornames VALUES ('18','Sensor15','Extra T/H Sensor');

  --
  -- table sensorparameters 
  --
  -- sensor1
INSERT INTO sensorparameters VALUES ('1','1','1'); -- temperature
INSERT INTO sensorparameters VALUES ('2','1','2'); -- humidity

  -- sensor2
INSERT INTO sensorparameters VALUES ('3','2','1'); -- temperature
INSERT INTO sensorparameters VALUES ('4','2','2'); -- humidity

  -- sensor3
INSERT INTO sensorparameters VALUES ('5','3','1');
INSERT INTO sensorparameters VALUES ('6','3','2');

  -- sensor4
INSERT INTO sensorparameters VALUES ('7','4','1') ;
INSERT INTO sensorparameters VALUES ('8','4','2');

  -- sensor5
INSERT INTO sensorparameters VALUES ('9','5','1');
INSERT INTO sensorparameters VALUES ('10','5','2');

  -- sensor6
INSERT INTO sensorparameters VALUES ('11','6','1');
INSERT INTO sensorparameters VALUES ('12','6','2');

  -- sensor7
INSERT INTO sensorparameters VALUES ('13','7','1');
INSERT INTO sensorparameters VALUES ('14','7','2');

  -- Sensor8
INSERT INTO sensorparameters VALUES ('15','8','1');
INSERT INTO sensorparameters VALUES ('16','8','2');
  -- Regensensor
INSERT INTO sensorparameters VALUES ('17','9','3');
INSERT INTO sensorparameters VALUES ('38','9','8'); -- precipitation intensity
  -- Windsensor
INSERT INTO sensorparameters VALUES ('18','10','4');
INSERT INTO sensorparameters VALUES ('19','10','5');
INSERT INTO sensorparameters VALUES ('20','10','6');
  -- Innensensor
INSERT INTO sensorparameters VALUES ('21','11','7');
INSERT INTO sensorparameters VALUES ('22','11','1');
INSERT INTO sensorparameters VALUES ('23','11','2');
  -- Sensor9
INSERT INTO sensorparameters VALUES ('24','12','1');
INSERT INTO sensorparameters VALUES ('25','12','2');
  -- Sensor10
INSERT INTO sensorparameters VALUES ('26','13','1');
INSERT INTO sensorparameters VALUES ('27','13','2');
  -- Sensor11
INSERT INTO sensorparameters VALUES ('28','14','1');
INSERT INTO sensorparameters VALUES ('29','14','2');
  -- Sensor12
INSERT INTO sensorparameters VALUES ('30','15','1');
INSERT INTO sensorparameters VALUES ('31','15','2');
  -- Sensor13
INSERT INTO sensorparameters VALUES ('32','16','1');
INSERT INTO sensorparameters VALUES ('33','16','2');
  -- Sensor14
INSERT INTO sensorparameters VALUES ('34','17','1');
INSERT INTO sensorparameters VALUES ('35','17','2');
  -- Sensor15
INSERT INTO sensorparameters VALUES ('36','18','1');
INSERT INTO sensorparameters VALUES ('37','18','2');

  --
  -- table sensorupdate
  --
  -- Sensor1
INSERT INTO sensorupdate VALUES ('1','0');
INSERT INTO sensorupdate VALUES ('2','0');
  -- Sensor2
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
  -- Rainsensor
INSERT INTO sensorupdate VALUES ('17','0'); -- precipitation
INSERT INTO sensorupdate VALUES ('38','0'); -- precipitation intensity
  -- Windsensor
INSERT INTO sensorupdate VALUES ('18','0');
INSERT INTO sensorupdate VALUES ('19','0');
INSERT INTO sensorupdate VALUES ('20','0');
  -- Indoorsensor
INSERT INTO sensorupdate VALUES ('21','0');
INSERT INTO sensorupdate VALUES ('22','0');
INSERT INTO sensorupdate VALUES ('23','0');

-- Sensor 9
INSERT INTO sensorupdate VALUES ('24','0');
INSERT INTO sensorupdate VALUES ('25','0');

-- Sensor 10
INSERT INTO sensorupdate VALUES ('26','0');
INSERT INTO sensorupdate VALUES ('27','0');
  -- Sensor11
INSERT INTO sensorupdate VALUES ('28','0');
INSERT INTO sensorupdate VALUES ('29','0');
  -- Sensor12
INSERT INTO sensorupdate VALUES ('30','0');
INSERT INTO sensorupdate VALUES ('31','0');
  -- Sensor13
INSERT INTO sensorupdate VALUES ('32','0');
INSERT INTO sensorupdate VALUES ('33','0');
  -- Sensor14
INSERT INTO sensorupdate VALUES ('34','0');
INSERT INTO sensorupdate VALUES ('35','0');
  -- Sensor15
INSERT INTO sensorupdate VALUES ('36','0');
INSERT INTO sensorupdate VALUES ('37','0');

