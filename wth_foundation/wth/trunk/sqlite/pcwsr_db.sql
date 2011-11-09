--
-- pcwsr_db.sql
-- create SQLITE database for pcwsr weatherstation
--
-- $Id$
--
-- SQLITE Database description for 
-- - PCWSR weatherstation receiver
--
-- PCWSR weatherstation has
--     - sensors which
--         - measure parameter values
--
-- Copyright (C) 2008 Volker Jahns, volker@thalreit.de
--

CREATE TABLE
sensordata
(
  dataset_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  dataset_date DATE NOT NULL,
  sensor_meas_no INTEGER NOT NULL,
  meas_value FLOAT NOT NULL,
  FOREIGN KEY (sensor_meas_no) REFERENCES sensorparameters (sensor_meas_no)
);

CREATE INDEX sdidx2 ON sensordata(dataset_date);

create table
sensornames
(
  sensor_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  sensor_name VARCHAR(255) NOT NULL,
  address INTEGER NOT NULL,
  version VARCHAR(8) NOT NULL,
  description VARCHAR(255) NOT NULL
);

create table
sensorupdate
(
  sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  last_update DATE NOT NULL
);

create table
parameternames
(
  parameter_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  parameter_name VARCHAR(64) NOT NULL,
  parameter_unit VARCHAR(64) NOT NULL
);

create table
sensorparameters
(
  sensor_meas_no INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  sensor_no INTEGER NOT NULL,
  parameter_no INTEGER NOT NULL,
  FOREIGN KEY (parameter_no) REFERENCES parameternames (parameter_no),
  FOREIGN KEY (sensor_no) REFERENCES sensornames (sensor_no)
);

--
-- insert table values which do not change
-- table parameternames
INSERT INTO parameternames VALUES ('1','Temperature','°C');
INSERT INTO parameternames VALUES ('2','Relative Humidity','%');
INSERT INTO parameternames VALUES ('3','Precipitation','mm/m²');
INSERT INTO parameternames VALUES ('4','Windspeed','m/s');
INSERT INTO parameternames VALUES ('5','Winddir. Variation','°');
INSERT INTO parameternames VALUES ('6','Winddirection','°') ;
INSERT INTO parameternames VALUES ('7','Pressure','hPa') ;
INSERT INTO parameternames VALUES ('8','Brightness','klx');
INSERT INTO parameternames VALUES ('9','RadiationPower','arb.unit') ;

-- table sensornames
INSERT INTO sensornames VALUES ('0','Outdoor','0x00','V1.1') ;
INSERT INTO sensornames VALUES ('1','Outdoor','0x01','V1.1') ;
INSERT INTO sensornames VALUES ('2','Outdoor','0x02','V1.1');
INSERT INTO sensornames VALUES ('3','Outdoor','0x03','V1.1');
INSERT INTO sensornames VALUES ('4','Outdoor','0x04','V1.1');
INSERT INTO sensornames VALUES ('5','Outdoor','0x05','V1.1');
INSERT INTO sensornames VALUES ('6','Outdoor','0x06','V1.1');
INSERT INTO sensornames VALUES ('7','Outdoor','0x07','V1.1');
-- 0x08 - 0x0f outdoor T only V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('8','Outdoor','0x00','V1.2');
INSERT INTO sensornames VALUES ('9','Outdoor','0x01','V1.2');
INSERT INTO sensornames VALUES ('10','Outdoor','0x02','V1.2');
INSERT INTO sensornames VALUES ('11','Outdoor','0x03','V1.2');
INSERT INTO sensornames VALUES ('12','Outdoor','0x04','V1.2');
INSERT INTO sensornames VALUES ('13','Outdoor','0x05','V1.2');
INSERT INTO sensornames VALUES ('14','Outdoor','0x06','V1.2');
INSERT INTO sensornames VALUES ('15','Outdoor','0x07','V1.2');

-- 0x10 - 0x17 outdoor T/H V1.1 address 0 .. 7
INSERT INTO sensornames VALUES ('16','Outdoor','0x00','V1.1');
INSERT INTO sensornames VALUES ('17','Outdoor','0x01','V1.1');
INSERT INTO sensornames VALUES ('18','Outdoor','0x02','V1.1');
INSERT INTO sensornames VALUES ('19','Outdoor','0x03','V1.1');
INSERT INTO sensornames VALUES ('20','Outdoor','0x04','V1.1');
INSERT INTO sensornames VALUES ('21','Outdoor','0x05','V1.1');
INSERT INTO sensornames VALUES ('22','Outdoor','0x06','V1.1');
INSERT INTO sensornames VALUES ('23','Outdoor','0x07','V1.1');
--  0x18 - 0x1f outdoor T/H V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('24','Outdoor','0x00','V1.2');
INSERT INTO sensornames VALUES ('25','Outdoor','0x01','V1.2');
INSERT INTO sensornames VALUES ('26','Outdoor','0x02','V1.2');
INSERT INTO sensornames VALUES ('27','Outdoor','0x03','V1.2');
INSERT INTO sensornames VALUES ('28','Outdoor','0x04','V1.2');
INSERT INTO sensornames VALUES ('29','Outdoor','0x05','V1.2');
INSERT INTO sensornames VALUES ('30','Outdoor','0x06','V1.2');
INSERT INTO sensornames VALUES ('31','Outdoor','0x07','V1.2');

-- 0x20 - 0x27 rain V1.1 address 0 .. 7
INSERT INTO sensornames VALUES ('32','Rain','0x00','V1.1');
INSERT INTO sensornames VALUES ('33','Rain','0x01','V1.1');
INSERT INTO sensornames VALUES ('34','Rain','0x02','V1.1');
INSERT INTO sensornames VALUES ('35','Rain','0x03','V1.1');
INSERT INTO sensornames VALUES ('36','Rain','0x04','V1.1');
INSERT INTO sensornames VALUES ('37','Rain','0x05','V1.1');
INSERT INTO sensornames VALUES ('38','Rain','0x06','V1.1');
INSERT INTO sensornames VALUES ('39','Rain','0x07','V1.1');
-- 0x28 - 0x2f rain V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('40','Rain','0x00','V1.2');
INSERT INTO sensornames VALUES ('41','Rain','0x01','V1.2');
INSERT INTO sensornames VALUES ('42','Rain','0x02','V1.2');
INSERT INTO sensornames VALUES ('43','Rain','0x03','V1.2');
INSERT INTO sensornames VALUES ('44','Rain','0x04','V1.2');
INSERT INTO sensornames VALUES ('45','Rain','0x05','V1.2');
INSERT INTO sensornames VALUES ('46','Rain','0x06','V1.2');
INSERT INTO sensornames VALUES ('47','Rain','0x07','V1.2');

-- 0x30 - 0x37 wind V1.1 address 0 .. 7
INSERT INTO sensornames VALUES ('48','Wind','0x00','V1.1');
INSERT INTO sensornames VALUES ('49','Wind','0x01','V1.1');
INSERT INTO sensornames VALUES ('50','Wind','0x02','V1.1');
INSERT INTO sensornames VALUES ('51','Wind','0x03','V1.1');
INSERT INTO sensornames VALUES ('52','Wind','0x04','V1.1');
INSERT INTO sensornames VALUES ('53','Wind','0x05','V1.1');
INSERT INTO sensornames VALUES ('54','Wind','0x06','V1.1');
INSERT INTO sensornames VALUES ('55','Wind','0x07','V1.1');
-- 0x38 - 0x3f wind V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('56','Wind','0x00','V1.2');
INSERT INTO sensornames VALUES ('57','Wind','0x01','V1.2');
INSERT INTO sensornames VALUES ('58','Wind','0x02','V1.2');
INSERT INTO sensornames VALUES ('59','Wind','0x03','V1.2');
INSERT INTO sensornames VALUES ('60','Wind','0x04','V1.2');
INSERT INTO sensornames VALUES ('61','Wind','0x05','V1.2');
INSERT INTO sensornames VALUES ('62','Wind','0x06','V1.2');
INSERT INTO sensornames VALUES ('63','Wind','0x07','V1.2');
-- 0x40 - 0x47 indoor V1.1 address 0 .. 7
INSERT INTO sensornames VALUES ('64','Indoor','0x00','V1.1');
INSERT INTO sensornames VALUES ('65','Indoor','0x01','V1.1');
INSERT INTO sensornames VALUES ('66','Indoor','0x02','V1.1');
INSERT INTO sensornames VALUES ('67','Indoor','0x03','V1.1');
INSERT INTO sensornames VALUES ('68','Indoor','0x04','V1.1');
INSERT INTO sensornames VALUES ('69','Indoor','0x05','V1.1');
INSERT INTO sensornames VALUES ('70','Indoor','0x06','V1.1');
INSERT INTO sensornames VALUES ('71','Indoor','0x07','V1.1');
-- 0x48 - 0x4f indoor V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('72','Indoor','0x00','V1.2');
INSERT INTO sensornames VALUES ('73','Indoor','0x01','V1.2');
INSERT INTO sensornames VALUES ('74','Indoor','0x02','V1.2');
INSERT INTO sensornames VALUES ('75','Indoor','0x03','V1.2');
INSERT INTO sensornames VALUES ('76','Indoor','0x04','V1.2');
INSERT INTO sensornames VALUES ('77','Indoor','0x05','V1.2');
INSERT INTO sensornames VALUES ('78','Indoor','0x06','V1.2');
INSERT INTO sensornames VALUES ('79','Indoor','0x07','V1.2');
-- 0x58 - 0x5f brightness V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('88','Brightness','0x00','V1.2');
INSERT INTO sensornames VALUES ('89','Brightness','0x01','V1.2');
INSERT INTO sensornames VALUES ('90','Brightness','0x02','V1.2');
INSERT INTO sensornames VALUES ('91','Brightness','0x03','V1.2');
INSERT INTO sensornames VALUES ('92','Brightness','0x04','V1.2');
INSERT INTO sensornames VALUES ('93','Brightness','0x05','V1.2');
INSERT INTO sensornames VALUES ('94','Brightness','0x06','V1.2');
INSERT INTO sensornames VALUES ('95','Brightness','0x07','V1.2');
-- 0x68 - 0x6f pyranometer V1.2 address 0 .. 7
INSERT INTO sensornames VALUES ('104','Pyranometer','0x00','V1.2');
INSERT INTO sensornames VALUES ('105','Pyranometer','0x01','V1.2');
INSERT INTO sensornames VALUES ('106','Pyranometer','0x02','V1.2');
INSERT INTO sensornames VALUES ('107','Pyranometer','0x03','V1.2');
INSERT INTO sensornames VALUES ('108','Pyranometer','0x04','V1.2');
INSERT INTO sensornames VALUES ('109','Pyranometer','0x05','V1.2');
INSERT INTO sensornames VALUES ('110','Pyranometer','0x06','V1.2');
INSERT INTO sensornames VALUES ('111','Pyranometer','0x07','V1.2');

-- table sensorparameters
-- outdoor V1.1 and V1.2 type 0x00 .. 0x0f
INSERT INTO  sensorparameters VALUES ('1','0','1');
INSERT INTO  sensorparameters VALUES ('2','1','1');
INSERT INTO  sensorparameters VALUES ('3','2','1');
INSERT INTO  sensorparameters VALUES ('4','3','1');
INSERT INTO  sensorparameters VALUES ('5','4','1');
INSERT INTO  sensorparameters VALUES ('6','5','1');
INSERT INTO  sensorparameters VALUES ('7','6','1');
INSERT INTO  sensorparameters VALUES ('8','7','1');
INSERT INTO  sensorparameters VALUES ('9','8','1');
INSERT INTO  sensorparameters VALUES ('10','9','1');
INSERT INTO  sensorparameters VALUES ('11','10','1');
INSERT INTO  sensorparameters VALUES ('12','11','1');
INSERT INTO  sensorparameters VALUES ('13','12','1');
INSERT INTO  sensorparameters VALUES ('14','13','1');
INSERT INTO  sensorparameters VALUES ('15','14','1');
INSERT INTO  sensorparameters VALUES ('16','15','1');


-- outdoor V1.1 and V1.2 type 0x10 .. 0x1f
INSERT INTO  sensorparameters VALUES ('17','16','1');
INSERT INTO  sensorparameters VALUES ('18','16','2');

INSERT INTO  sensorparameters VALUES ('19','17','1');
INSERT INTO  sensorparameters VALUES ('20','17','2');

INSERT INTO  sensorparameters VALUES ('21','18','1');
INSERT INTO  sensorparameters VALUES ('22','18','2');

INSERT INTO  sensorparameters VALUES ('23','19','1');
INSERT INTO  sensorparameters VALUES ('24','19','2');

INSERT INTO  sensorparameters VALUES ('25','20','1');
INSERT INTO  sensorparameters VALUES ('26','20','2');

INSERT INTO  sensorparameters VALUES ('27','21','1');
INSERT INTO  sensorparameters VALUES ('28','21','2');

INSERT INTO  sensorparameters VALUES ('29','22','1');
INSERT INTO  sensorparameters VALUES ('30','22','2');

INSERT INTO  sensorparameters VALUES ('31','23','1');
INSERT INTO  sensorparameters VALUES ('32','23','2');

INSERT INTO  sensorparameters VALUES ('33','24','1');
INSERT INTO  sensorparameters VALUES ('34','24','2');

INSERT INTO  sensorparameters VALUES ('35','25','1');
INSERT INTO  sensorparameters VALUES ('36','25','2');

INSERT INTO  sensorparameters VALUES ('37','26','1');
INSERT INTO  sensorparameters VALUES ('38','26','2');

INSERT INTO  sensorparameters VALUES ('39','27','1');
INSERT INTO  sensorparameters VALUES ('40','27','2');

INSERT INTO  sensorparameters VALUES ('41','28','1');
INSERT INTO  sensorparameters VALUES ('42','28','2');

INSERT INTO  sensorparameters VALUES ('43','29','1');
INSERT INTO  sensorparameters VALUES ('44','29','2');

INSERT INTO  sensorparameters VALUES ('45','30','1');
INSERT INTO  sensorparameters VALUES ('46','30','2');

INSERT INTO  sensorparameters VALUES ('47','31','1');
INSERT INTO  sensorparameters VALUES ('48','31','2');

-- rain V1.1 and V1.2 type 0x20 .. 0x2f
INSERT INTO  sensorparameters VALUES ('49','32','3');
INSERT INTO  sensorparameters VALUES ('50','33','3');
INSERT INTO  sensorparameters VALUES ('51','34','3');
INSERT INTO  sensorparameters VALUES ('52','35','3');
INSERT INTO  sensorparameters VALUES ('53','36','3');
INSERT INTO  sensorparameters VALUES ('54','37','3');
INSERT INTO  sensorparameters VALUES ('55','38','3');
INSERT INTO  sensorparameters VALUES ('56','39','3');
INSERT INTO  sensorparameters VALUES ('57','40','3');
INSERT INTO  sensorparameters VALUES ('58','41','3');
INSERT INTO  sensorparameters VALUES ('59','42','3');
INSERT INTO  sensorparameters VALUES ('60','43','3');
INSERT INTO  sensorparameters VALUES ('61','44','3');
INSERT INTO  sensorparameters VALUES ('62','45','3');
INSERT INTO  sensorparameters VALUES ('63','46','3');
INSERT INTO  sensorparameters VALUES ('64','47','3');

-- wind V1.1 and V1.2 type 0x30 .. 0x3f
INSERT INTO  sensorparameters VALUES ('65','48','4');
INSERT INTO  sensorparameters VALUES ('66','48','5');
INSERT INTO  sensorparameters VALUES ('67','48','6');
INSERT INTO  sensorparameters VALUES ('68','49','4');
INSERT INTO  sensorparameters VALUES ('69','49','5');
INSERT INTO  sensorparameters VALUES ('70','49','6');
INSERT INTO  sensorparameters VALUES ('71','50','4');
INSERT INTO  sensorparameters VALUES ('72','50','5');
INSERT INTO  sensorparameters VALUES ('73','50','6');
INSERT INTO  sensorparameters VALUES ('74','51','4');
INSERT INTO  sensorparameters VALUES ('75','51','5');
INSERT INTO  sensorparameters VALUES ('76','51','6');
INSERT INTO  sensorparameters VALUES ('77','52','4');
INSERT INTO  sensorparameters VALUES ('78','52','5');
INSERT INTO  sensorparameters VALUES ('79','52','6');
INSERT INTO  sensorparameters VALUES ('80','53','4');
INSERT INTO  sensorparameters VALUES ('81','53','5');
INSERT INTO  sensorparameters VALUES ('82','53','6');
INSERT INTO  sensorparameters VALUES ('83','54','4');
INSERT INTO  sensorparameters VALUES ('84','54','5');
INSERT INTO  sensorparameters VALUES ('85','54','6');
INSERT INTO  sensorparameters VALUES ('86','55','4');
INSERT INTO  sensorparameters VALUES ('87','55','5');
INSERT INTO  sensorparameters VALUES ('88','55','6');
INSERT INTO  sensorparameters VALUES ('89','56','4');
INSERT INTO  sensorparameters VALUES ('90','56','5');
INSERT INTO  sensorparameters VALUES ('91','56','6');
INSERT INTO  sensorparameters VALUES ('92','57','4');
INSERT INTO  sensorparameters VALUES ('93','57','5');
INSERT INTO  sensorparameters VALUES ('94','57','6');
INSERT INTO  sensorparameters VALUES ('95','58','4');
INSERT INTO  sensorparameters VALUES ('96','58','5');
INSERT INTO  sensorparameters VALUES ('97','58','6');
INSERT INTO  sensorparameters VALUES ('98','59','4');
INSERT INTO  sensorparameters VALUES ('99','59','5');
INSERT INTO  sensorparameters VALUES ('100','59','6');
INSERT INTO  sensorparameters VALUES ('101','60','4');
INSERT INTO  sensorparameters VALUES ('102','60','5');
INSERT INTO  sensorparameters VALUES ('103','60','6');
INSERT INTO  sensorparameters VALUES ('104','61','4');
INSERT INTO  sensorparameters VALUES ('105','61','5');
INSERT INTO  sensorparameters VALUES ('106','61','6');
INSERT INTO  sensorparameters VALUES ('107','62','4');
INSERT INTO  sensorparameters VALUES ('108','62','5');
INSERT INTO  sensorparameters VALUES ('109','62','6');
INSERT INTO  sensorparameters VALUES ('110','63','4');
INSERT INTO  sensorparameters VALUES ('111','63','5');
INSERT INTO  sensorparameters VALUES ('112','63','6');


-- indoor V1.1 and V1.2 type 0x40 .. 0x4f
INSERT INTO  sensorparameters VALUES ('113','64','1');
INSERT INTO  sensorparameters VALUES ('114','64','2');
INSERT INTO  sensorparameters VALUES ('115','64','7');

INSERT INTO  sensorparameters VALUES ('116','65','1');
INSERT INTO  sensorparameters VALUES ('117','65','2');
INSERT INTO  sensorparameters VALUES ('118','65','7');

INSERT INTO  sensorparameters VALUES ('119','66','1');
INSERT INTO  sensorparameters VALUES ('120','66','2');
INSERT INTO  sensorparameters VALUES ('121','66','7');

INSERT INTO  sensorparameters VALUES ('122','67','1');
INSERT INTO  sensorparameters VALUES ('123','67','2');
INSERT INTO  sensorparameters VALUES ('124','67','7');

INSERT INTO  sensorparameters VALUES ('125','68','1');
INSERT INTO  sensorparameters VALUES ('126','68','2');
INSERT INTO  sensorparameters VALUES ('127','68','7');

INSERT INTO  sensorparameters VALUES ('128','69','1');
INSERT INTO  sensorparameters VALUES ('129','69','2');
INSERT INTO  sensorparameters VALUES ('130','69','7');

INSERT INTO  sensorparameters VALUES ('131','70','1');
INSERT INTO  sensorparameters VALUES ('132','70','2');
INSERT INTO  sensorparameters VALUES ('133','70','7');

INSERT INTO  sensorparameters VALUES ('134','71','1');
INSERT INTO  sensorparameters VALUES ('135','71','2');
INSERT INTO  sensorparameters VALUES ('136','71','7');

INSERT INTO  sensorparameters VALUES ('137','72','1');
INSERT INTO  sensorparameters VALUES ('138','72','2');
INSERT INTO  sensorparameters VALUES ('139','72','7');

INSERT INTO  sensorparameters VALUES ('140','73','1');
INSERT INTO  sensorparameters VALUES ('141','73','2');
INSERT INTO  sensorparameters VALUES ('142','73','7');

INSERT INTO  sensorparameters VALUES ('143','74','1');
INSERT INTO  sensorparameters VALUES ('144','74','2');
INSERT INTO  sensorparameters VALUES ('145','74','7');

INSERT INTO  sensorparameters VALUES ('146','75','1');
INSERT INTO  sensorparameters VALUES ('147','75','2');
INSERT INTO  sensorparameters VALUES ('148','75','7');

INSERT INTO  sensorparameters VALUES ('149','76','1');
INSERT INTO  sensorparameters VALUES ('150','76','2');
INSERT INTO  sensorparameters VALUES ('151','76','7');

INSERT INTO  sensorparameters VALUES ('152','77','1');
INSERT INTO  sensorparameters VALUES ('153','77','2');
INSERT INTO  sensorparameters VALUES ('154','77','7');

INSERT INTO  sensorparameters VALUES ('155','78','1');
INSERT INTO  sensorparameters VALUES ('156','78','2');
INSERT INTO  sensorparameters VALUES ('157','78','7');

INSERT INTO  sensorparameters VALUES ('158','79','1');
INSERT INTO  sensorparameters VALUES ('159','79','2');
INSERT INTO  sensorparameters VALUES ('160','79','7');

-- brightness V1.2 type 0x58 .. 0x5f
INSERT INTO  sensorparameters VALUES ('161','88','8');
INSERT INTO  sensorparameters VALUES ('162','89','8');
INSERT INTO  sensorparameters VALUES ('163','90','8');
INSERT INTO  sensorparameters VALUES ('164','91','8');
INSERT INTO  sensorparameters VALUES ('165','92','8');
INSERT INTO  sensorparameters VALUES ('166','93','8');
INSERT INTO  sensorparameters VALUES ('167','94','8');
INSERT INTO  sensorparameters VALUES ('168','95','8');

-- pyranometer V1.2 type 0x68 .. 0x6f
INSERT INTO  sensorparameters VALUES ('169','104','9');
INSERT INTO  sensorparameters VALUES ('170','105','9');
INSERT INTO  sensorparameters VALUES ('171','106','9');
INSERT INTO  sensorparameters VALUES ('172','107','9');
INSERT INTO  sensorparameters VALUES ('173','108','9');
INSERT INTO  sensorparameters VALUES ('174','109','9');
INSERT INTO  sensorparameters VALUES ('175','110','9');
INSERT INTO  sensorparameters VALUES ('176','111','9');

-- initial values of table sensorupdate
INSERT INTO sensorupdate VALUES ('0','0');
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
INSERT INTO sensorupdate VALUES ('25','0');
INSERT INTO sensorupdate VALUES ('26','0');
INSERT INTO sensorupdate VALUES ('27','0');

INSERT INTO sensorupdate VALUES ('28','0');
INSERT INTO sensorupdate VALUES ('29','0');
INSERT INTO sensorupdate VALUES ('30','0');
INSERT INTO sensorupdate VALUES ('31','0');
INSERT INTO sensorupdate VALUES ('32','0');
INSERT INTO sensorupdate VALUES ('33','0');
INSERT INTO sensorupdate VALUES ('34','0');
INSERT INTO sensorupdate VALUES ('35','0');

INSERT INTO sensorupdate VALUES ('36','0');
INSERT INTO sensorupdate VALUES ('37','0');
INSERT INTO sensorupdate VALUES ('38','0');
INSERT INTO sensorupdate VALUES ('39','0');
INSERT INTO sensorupdate VALUES ('40','0');
INSERT INTO sensorupdate VALUES ('41','0');
INSERT INTO sensorupdate VALUES ('42','0');
INSERT INTO sensorupdate VALUES ('43','0');
INSERT INTO sensorupdate VALUES ('44','0');
INSERT INTO sensorupdate VALUES ('45','0');
INSERT INTO sensorupdate VALUES ('46','0');
INSERT INTO sensorupdate VALUES ('47','0');

INSERT INTO sensorupdate VALUES ('48','0');
INSERT INTO sensorupdate VALUES ('49','0');
INSERT INTO sensorupdate VALUES ('50','0');
INSERT INTO sensorupdate VALUES ('51','0');
INSERT INTO sensorupdate VALUES ('52','0');
INSERT INTO sensorupdate VALUES ('53','0');
INSERT INTO sensorupdate VALUES ('54','0');
INSERT INTO sensorupdate VALUES ('55','0');

INSERT INTO sensorupdate VALUES ('56','0');
INSERT INTO sensorupdate VALUES ('57','0');
INSERT INTO sensorupdate VALUES ('58','0');
INSERT INTO sensorupdate VALUES ('59','0');
INSERT INTO sensorupdate VALUES ('60','0');
INSERT INTO sensorupdate VALUES ('61','0');
INSERT INTO sensorupdate VALUES ('62','0');
INSERT INTO sensorupdate VALUES ('63','0');
INSERT INTO sensorupdate VALUES ('64','0');
INSERT INTO sensorupdate VALUES ('65','0');
INSERT INTO sensorupdate VALUES ('66','0');
INSERT INTO sensorupdate VALUES ('67','0');

INSERT INTO sensorupdate VALUES ('68','0');
INSERT INTO sensorupdate VALUES ('69','0');
INSERT INTO sensorupdate VALUES ('70','0');
INSERT INTO sensorupdate VALUES ('71','0');
INSERT INTO sensorupdate VALUES ('72','0');
INSERT INTO sensorupdate VALUES ('73','0');
INSERT INTO sensorupdate VALUES ('74','0');
INSERT INTO sensorupdate VALUES ('75','0');

INSERT INTO sensorupdate VALUES ('76','0');
INSERT INTO sensorupdate VALUES ('77','0');
INSERT INTO sensorupdate VALUES ('78','0');
INSERT INTO sensorupdate VALUES ('79','0');
INSERT INTO sensorupdate VALUES ('80','0');
INSERT INTO sensorupdate VALUES ('81','0');
INSERT INTO sensorupdate VALUES ('82','0');
INSERT INTO sensorupdate VALUES ('83','0');
INSERT INTO sensorupdate VALUES ('84','0');
INSERT INTO sensorupdate VALUES ('85','0');
INSERT INTO sensorupdate VALUES ('86','0');
INSERT INTO sensorupdate VALUES ('87','0');

INSERT INTO sensorupdate VALUES ('88','0');
INSERT INTO sensorupdate VALUES ('89','0');
INSERT INTO sensorupdate VALUES ('90','0');
INSERT INTO sensorupdate VALUES ('91','0');
INSERT INTO sensorupdate VALUES ('92','0');
INSERT INTO sensorupdate VALUES ('93','0');
INSERT INTO sensorupdate VALUES ('94','0');
INSERT INTO sensorupdate VALUES ('95','0');

INSERT INTO sensorupdate VALUES ('96','0');
INSERT INTO sensorupdate VALUES ('97','0');
INSERT INTO sensorupdate VALUES ('98','0');
INSERT INTO sensorupdate VALUES ('99','0');
INSERT INTO sensorupdate VALUES ('100','0');
INSERT INTO sensorupdate VALUES ('101','0');
INSERT INTO sensorupdate VALUES ('102','0');
INSERT INTO sensorupdate VALUES ('103','0');
INSERT INTO sensorupdate VALUES ('104','0');
INSERT INTO sensorupdate VALUES ('105','0');
INSERT INTO sensorupdate VALUES ('106','0');
INSERT INTO sensorupdate VALUES ('107','0');

INSERT INTO sensorupdate VALUES ('108','0');
INSERT INTO sensorupdate VALUES ('109','0');

INSERT INTO sensorupdate VALUES ('110','0');
INSERT INTO sensorupdate VALUES ('111','0');
INSERT INTO sensorupdate VALUES ('112','0');
INSERT INTO sensorupdate VALUES ('113','0');
INSERT INTO sensorupdate VALUES ('114','0');
INSERT INTO sensorupdate VALUES ('115','0');
INSERT INTO sensorupdate VALUES ('116','0');
INSERT INTO sensorupdate VALUES ('117','0');

INSERT INTO sensorupdate VALUES ('118','0');
INSERT INTO sensorupdate VALUES ('119','0');
INSERT INTO sensorupdate VALUES ('120','0');
INSERT INTO sensorupdate VALUES ('121','0');
INSERT INTO sensorupdate VALUES ('122','0');
INSERT INTO sensorupdate VALUES ('123','0');
INSERT INTO sensorupdate VALUES ('124','0');
INSERT INTO sensorupdate VALUES ('125','0');

INSERT INTO sensorupdate VALUES ('126','0');
INSERT INTO sensorupdate VALUES ('127','0');
INSERT INTO sensorupdate VALUES ('128','0');
INSERT INTO sensorupdate VALUES ('129','0');
INSERT INTO sensorupdate VALUES ('130','0');
INSERT INTO sensorupdate VALUES ('131','0');
INSERT INTO sensorupdate VALUES ('132','0');
INSERT INTO sensorupdate VALUES ('133','0');
INSERT INTO sensorupdate VALUES ('134','0');
INSERT INTO sensorupdate VALUES ('135','0');
INSERT INTO sensorupdate VALUES ('136','0');
INSERT INTO sensorupdate VALUES ('137','0');

INSERT INTO sensorupdate VALUES ('138','0');
INSERT INTO sensorupdate VALUES ('139','0');

INSERT INTO sensorupdate VALUES ('140','0');
INSERT INTO sensorupdate VALUES ('141','0');
INSERT INTO sensorupdate VALUES ('142','0');
INSERT INTO sensorupdate VALUES ('143','0');
INSERT INTO sensorupdate VALUES ('144','0');
INSERT INTO sensorupdate VALUES ('145','0');
INSERT INTO sensorupdate VALUES ('146','0');
INSERT INTO sensorupdate VALUES ('147','0');

INSERT INTO sensorupdate VALUES ('148','0');
INSERT INTO sensorupdate VALUES ('149','0');
INSERT INTO sensorupdate VALUES ('150','0');
INSERT INTO sensorupdate VALUES ('151','0');
INSERT INTO sensorupdate VALUES ('152','0');
INSERT INTO sensorupdate VALUES ('153','0');
INSERT INTO sensorupdate VALUES ('154','0');
INSERT INTO sensorupdate VALUES ('155','0');

INSERT INTO sensorupdate VALUES ('156','0');
INSERT INTO sensorupdate VALUES ('157','0');
INSERT INTO sensorupdate VALUES ('158','0');
INSERT INTO sensorupdate VALUES ('159','0');
INSERT INTO sensorupdate VALUES ('160','0');
INSERT INTO sensorupdate VALUES ('161','0');
INSERT INTO sensorupdate VALUES ('162','0');
INSERT INTO sensorupdate VALUES ('163','0');
INSERT INTO sensorupdate VALUES ('164','0');
INSERT INTO sensorupdate VALUES ('165','0');
INSERT INTO sensorupdate VALUES ('166','0');
INSERT INTO sensorupdate VALUES ('167','0');

INSERT INTO sensorupdate VALUES ('168','0');
INSERT INTO sensorupdate VALUES ('169','0');
INSERT INTO sensorupdate VALUES ('170','0');
INSERT INTO sensorupdate VALUES ('171','0');
INSERT INTO sensorupdate VALUES ('172','0');
INSERT INTO sensorupdate VALUES ('173','0');
INSERT INTO sensorupdate VALUES ('174','0');
INSERT INTO sensorupdate VALUES ('175','0');

INSERT INTO sensorupdate VALUES ('176','0');
