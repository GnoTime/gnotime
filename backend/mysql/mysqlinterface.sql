DROP DATABASE IF EXISTS timereg;

CREATE DATABASE IF NOT EXISTS timereg;

USE timereg;

CREATE TABLE default_config (
	min_interval		int	default     3,
	auto_merge_interval	int	default    60,
	auto_merge_gap		int	default    60,
	billrate		double	default   800.00,
	overtime_rate		double	default  1000.00,
	overover_rate		double	default  1500.00,
	flat_fee		double	default 10000.00
);

CREATE TABLE project (
	id			int				AUTO_INCREMENT	PRIMARY KEY,
	title			varchar(255)	default ''	NOT NULL,
	description		varchar(255)	default NULL,
	notes			text		default NULL,
	customerid		int		default NULL,
	min_interval		int		default 3,
	auto_merge_interval	int		default 60,
	auto_merge_gap		int		default 60,
	billrate		double		default 800.00,
	overtime_rate		double		default 1000.00,
	overover_rate		double		default 1500.00,
	flat_fee		double		default 10000.00
);

CREATE TABLE task (
	id			int				AUTO_INCREMENT 	PRIMARY KEY,
	projectid		int		default NULL	REFERENCES project,
	billable		int		default 0,
	billrate		int		default 0,
	done			int		default 0,
	billunit		int		default 900,
	topic			varchar(55)	default '',
	description		text		default '',
	username		varchar(55)
);

CREATE TABLE notes (
	id			int				AUTO_INCREMENT	PRIMARY KEY,
	taskid			int				REFERENCES task,
	projectid		int				REFERENCES project,
	topic			varchar(55)	default '',
	note			text		default '',
	username		varchar(55)
);

CREATE TABLE intervals (
	id			int				AUTO_INCREMENT	PRIMARY KEY,
	start_time		int		default 0	NOT NULL,
	stop_time		int		default 0,
	fuzzy			int		default 300,
	running			int		default 1,
	taskid			int				REFERENCES task,
	projectid		int				REFERENCES project,
	username		varchar(55)
);
