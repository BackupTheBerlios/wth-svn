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
-- table sensornames with all sensors
--
CREATE TABLE sensornames
    (
      sensor_no SERIAL PRIMARY KEY,
      sensor_name VARCHAR(255) NOT NULL,
      description VARCHAR(255) NOT NULL
    );

-- Wind Sensor
INSERT INTO sensornames VALUES ('1','windsensor','Wind Sensor');

-- Temperature Sensor
INSERT INTO sensornames VALUES ('2','t_out_sensor', 'Temperature Sensor');

-- Rain Gauge
INSERT INTO sensornames VALUES ('3','raingauge', 'Rain Gauge');

-- Indoor Humidity Sensor ( optional)
INSERT INTO sensornames VALUES ('4','h_in_sensor', 'Indoor Humidity Sensor');

-- T/H Sensor (optional)
INSERT INTO sensornames VALUES ('5','th_out_sensor','Outdoor Humitity/Temperature Sensor');

-- Indoor T/Pressure (inside display unit)
INSERT INTO sensornames VALUES ('6','tp_in_sensor','Indoor Temperature/Barometer Sensor in central display unit');


--
-- table parameternames with all physical parameters
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
-- datalogger mode parameters
INSERT INTO parameternames VALUES ('1','windspeed','Wind Speed','m/s', 1.000, 0.000);
INSERT INTO parameternames VALUES ('2','winddirection','Wind Direction','°', 1.000, 0.000);
INSERT INTO parameternames VALUES ('3','outdoor_temp','Outdoor Temperature','°C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('4','rain_total', 'Rain Longterm Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('5','pressure', 'Barometer','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('6','indoor_temp','Indoor Temperature','°C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('7','outdoor_hum','Outdoor Humidity','%', 1.000, 0.000);
INSERT INTO parameternames VALUES ('8','indoor_hum','Indoor Humidity','%', 1.000, 0.000);
INSERT INTO parameternames VALUES ('9','rain_today', 'Rain Today Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('10','windspeed_1min_avg','Wind Speed 1 Minute Average','m/s', 1.000, 0.000);
-- packet mode parameters
INSERT INTO parameternames VALUES ('11','windspeed_5min_peak','Wind Speed 5 Minute Peak','m/s', 1.000, 0.000);
INSERT INTO parameternames VALUES ('12','winddirection_peak', 'Wind Direction of Wind Speed Peak','°', 1.000, 0.000);
INSERT INTO parameternames VALUES ('14','pressure_delta','Barometer Delta','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('15','windspeed_5min_avg','Wind Speed 5 Minute Average','m/s', 1.000, 0.000);
-- completerecord mode parameters
INSERT INTO parameternames VALUES ('16','pressure_3hr_chg','Barometer 3-Hour Pressure Change','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('17','windchill','Wind Chill','°C', 1.000, 0.000); -- unclear, which T-sensor is used to calculate wind chill
INSERT INTO parameternames VALUES ('18','dewpoint','Dew Point','°C', 1.000, 0.000); -- unclear, which HT-sensor is used to calculate dew point
--
-- relation of parameter to sensor
-- i.e which physical parameter is measured in which sensor 
--
CREATE TABLE sensorparameters
  (
      sensor_meas_no SERIAL PRIMARY KEY,
      sensor_no INTEGER NOT NULL,
      param_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no),
      FOREIGN KEY (param_no) REFERENCES parameternames (param_no)
  );


INSERT INTO sensorparameters VALUES ('1','1','1');   -- Windsensor and Wind Speed
INSERT INTO sensorparameters VALUES ('2','1','2');   -- Windsensor and Wind Direction 
INSERT INTO sensorparameters VALUES ('3','1','10');  -- Windsensor and 1 Minute Windspeed Average 
INSERT INTO sensorparameters VALUES ('4','1','11');  -- Windsensor and Windspeed Peak 5 Minute
INSERT INTO sensorparameters VALUES ('5','1','12');  -- Windsensor and Direction of Windspeed Peak
INSERT INTO sensorparameters VALUES ('6','1','15');  -- Windsensor and 5 Minute Windspeed Average
INSERT INTO sensorparameters VALUES ('7','1','17');  -- Windsensor and Wind Chill

INSERT INTO sensorparameters VALUES ('8','2','3');   -- Temperatursensor and Outdoor Temp

INSERT INTO sensorparameters VALUES ('9','3','4');   -- Raingauge and Rain Long Term Total
INSERT INTO sensorparameters VALUES ('10','3','9');  -- Raingauge and Rain Today Total

INSERT INTO sensorparameters VALUES ('11','4','8');  -- Indoor Humiditysensor and Indoor Humidity

INSERT INTO sensorparameters VALUES ('12','5','3');  -- Outdoor H/T Sensor and Outdoor Temp
INSERT INTO sensorparameters VALUES ('13','5','7');  -- Outdoor H/T Sensor and Outdoor Humidity
INSERT INTO sensorparameters VALUES ('14','5','18'); -- Outdoor H/T Sensor and Dew Point

INSERT INTO sensorparameters VALUES ('15','6','5');  -- Indoor T/Pressure sensor and Barometer
INSERT INTO sensorparameters VALUES ('16','6','6');  -- Indoor T/Pressure sensor and Indoor Temperature
INSERT INTO sensorparameters VALUES ('17','6','14'); -- Indoor T/Pressure sensor and Barometer Delta
INSERT INTO sensorparameters VALUES ('18','6','16'); -- Indoor T/Pressure sensor and Barometer 3-Hour Pressure Change

--
-- Table flagname
-- -------------------
-- contains flag of sensor, e.g.
-- - over/underrange 
-- - low battery
-- 
CREATE TABLE flagnames
    (
      flag_no SERIAL PRIMARY KEY,
      flag_name VARCHAR(64) NOT NULL,
      flag_description VARCHAR(128) NOT NULL
    );
INSERT INTO flagnames VALUES ( 1, 'pressure_corr', 'Barometer Correction Factor');

-- Table sensorflags
-- -----------------------
-- relation of table sensornames to table flagnames
-- 
CREATE TABLE sensorflags
    (
      sensor_flag_no SERIAL PRIMARY KEY,
      sensor_no INTEGER NOT NULL,
      flag_no INTEGER NOT NULL,
      FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no),
      FOREIGN KEY (flag_no) REFERENCES flagnames (flag_no)
    );
INSERT INTO sensorflags VALUES ( 1, 6, 1); -- Indoor T/Pressure sensor and Barometer Correction

--
-- Table sensordata holds all measured values
--
CREATE TABLE sensordata
    (
      dataset_no SERIAL PRIMARY KEY, 
      dataset_date TIMESTAMP NOT NULL,
      sensor_meas_no INTEGER NOT NULL,
      meas_value FLOAT NOT NULL,
      FOREIGN KEY (sensor_meas_no) REFERENCES sensorparameters(sensor_meas_no)
    );
CREATE INDEX sdidx2 ON sensordata(dataset_date);

-- 
-- Table statusdata
-- ----------------
-- contains 
-- - time
-- - status value
-- 
CREATE TABLE statusdata
    (
      statusset_no SERIAL PRIMARY KEY,
      statusset_date TIMESTAMP NOT NULL,
      sensor_flag_no INTEGER NOT NULL,
      status_value FLOAT NOT NULL,
      FOREIGN KEY (sensor_flag_no) REFERENCES sensorflags (sensor_flag_no)
    );
CREATE INDEX ssidx ON statusdata(statusset_no,statusset_date,status_value);


--
-- table sensorupdate
--
CREATE TABLE sensorupdate
    (
      sensor_meas_no SERIAL PRIMARY KEY,
      last_update TIMESTAMP NOT NULL
    );

INSERT INTO sensorupdate VALUES ('1',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('2',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('3',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('4',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('5',TO_TIMESTAMP(0));

INSERT INTO sensorupdate VALUES ('6',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('7',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('8',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('9',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('10',TO_TIMESTAMP(0));

INSERT INTO sensorupdate VALUES ('11',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('12',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('13',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('14',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('15',TO_TIMESTAMP(0));

INSERT INTO sensorupdate VALUES ('16',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('17',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('18',TO_TIMESTAMP(0));
INSERT INTO sensorupdate VALUES ('19',TO_TIMESTAMP(0));
