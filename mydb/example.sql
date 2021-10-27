CREATE TABLE IF NOT EXISTS example (
    id int unsigned NOT NULL auto_increment,
    name varchar(255) NOT NULL,
    info text NULL,
    pi double NOT NULL default 3.14,
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
