CREATE TABLE keggMapDesc (
  mapID varchar(40) NOT NULL default '',
  description varchar(255) NOT NULL default '',
  KEY mapID (mapID)
) TYPE=MyISAM;

