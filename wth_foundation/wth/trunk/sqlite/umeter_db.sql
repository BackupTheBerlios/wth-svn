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
    parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    parameter_name VARCHAR(64) NOT NULL,
    description VARCHAR(64) NOT NULL,
    unit VARCHAR(64) NOT NULL,
    gain FLOAT NOT NULL,
    offset FLOAT NOT NULL
  );
-- datalogger mode parameters
INSERT INTO parameternames VALUES ('1','windspeed','Wind Speed','m s-1', 1.000, 0.000);
INSERT INTO parameternames VALUES ('2','winddirection','Wind Direction','deg', 1.000, 0.000);
INSERT INTO parameternames VALUES ('3','outdoor_temp','Outdoor Temperature','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('4','rain_total', 'Rain Longterm Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('5','pressure', 'Barometer','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('6','indoor_temp','Indoor Temperature','deg C', 1.000, 0.000);
INSERT INTO parameternames VALUES ('7','outdoor_hum','Outdoor Humidity','% rel.hum.', 1.000, 0.000);
INSERT INTO parameternames VALUES ('8','indoor_hum','Indoor Humidity','% rel.hum', 1.000, 0.000);
INSERT INTO parameternames VALUES ('9','rain_today', 'Rain Today Total','mm', 1.000, 0.000);
INSERT INTO parameternames VALUES ('10','windspeed_1min_avg','Wind Speed 1 Minute Average','m s-1', 1.000, 0.000);
-- packet mode parameters
INSERT INTO parameternames VALUES ('11','windspeed_5min_peak','Wind Speed 5 Minute Peak','m s-1', 1.000, 0.000);
INSERT INTO parameternames VALUES ('12','winddirection_peak', 'Wind Direction of Wind Speed Peak','deg', 1.000, 0.000);
INSERT INTO parameternames VALUES ('14','pressure_delta','Barometer Delta','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('15','pressure_corr','Barometer Correction Factor','', 1.000, 0.000);
INSERT INTO parameternames VALUES ('16','windspeed_5min_avg','Wind Speed 5 Minute Average','m s-1', 1.000, 0.000);
-- completerecord mode parameters
INSERT INTO parameternames VALUES ('17','pressure_3hr_chg','Barometer 3-Hour Pressure Change','hPa', 1.000, 0.000);
INSERT INTO parameternames VALUES ('18','windchill','Wind Chill','deg C', 1.000, 0.000); -- unclear, which T-sensor is used to calculate wind chill
INSERT INTO parameternames VALUES ('19','dewpoint','Dew Point','deg C', 1.000, 0.000); -- unclear, which HT-sensor is used to calculate dew point
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
INSERT INTO sensorparameters VALUES ('6','1','16');  -- Windsensor and 5 Minute Windspeed Average
INSERT INTO sensorparameters VALUES ('7','1','18');  -- Windsensor and Wind Chill

INSERT INTO sensorparameters VALUES ('8','2','3');   -- Temperatursensor and Outdoor Temp

INSERT INTO sensorparameters VALUES ('9','3','4');   -- Raingauge and Rain Long Term Total
INSERT INTO sensorparameters VALUES ('10','3','9');  -- Raingauge and Rain Today Total

INSERT INTO sensorparameters VALUES ('11','4','8');  -- Indoor Humiditysensor and Indoor Humidity

INSERT INTO sensorparameters VALUES ('12','5','3');  -- Outdoor H/T Sensor and Outdoor Temp
INSERT INTO sensorparameters VALUES ('13','5','7');  -- Outdoor H/T Sensor and Outdoor Humidity
INSERT INTO sensorparameters VALUES ('14','5','19'); -- Outdoor H/T Sensor and Dew Point

INSERT INTO sensorparameters VALUES ('15','6','5');  -- Indoor T/Pressure sensor and Barometer
INSERT INTO sensorparameters VALUES ('16','6','6');  -- Indoor T/Pressure sensor and Indoor Temperature
INSERT INTO sensorparameters VALUES ('17','6','14'); -- Indoor T/Pressure sensor and Barometer Delta
INSERT INTO sensorparameters VALUES ('18','6','15'); -- Indoor T/Pressure sensor and Barometer Correction Factor
INSERT INTO sensorparameters VALUES ('19','6','17'); -- Indoor T/Pressure sensor and Barometer 3-Hour Pressure Change

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
