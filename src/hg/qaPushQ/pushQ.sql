# pushQ.sql was originally generated by the autoSql program, which also 
# generated pushQ.c and pushQ.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Push Queue
CREATE TABLE pushQ (
    qid char(6) not null,	# Queue Id
    pqid char(6) not null,	# Parent Queue Id if split off another
    priority char(1) not null,	# Priority
    rank int unsigned not null,	# Rank for display sort
    qadate char(10) not null,	# QA (re)-submission date
    newYN char(1) not null,	# new (track)?
    track varchar(255) not null,	# Track
    dbs varchar(255) not null,	# Databases
    tbls longblob not null,	# Tables
    cgis varchar(255) not null,	# CGI(s)
    files varchar(255) not null,	# File(s)
    sizeMB int unsigned not null,	# Size MB
    currLoc char(20) not null,	# Current Location
    makeDocYN char(1) not null,	# MakeDoc verified?
    onlineHelp char(50) not null,	# Online Help
    ndxYN char(1) not null,	# Index verified?
    joinerYN char(1) not null,	# all.joiner verified?
    stat varchar(255) not null,	# Status
    sponsor char(50) not null,	# Sponsor
    reviewer char(50) not null,	# QA Reviewer
    extSource char(50) not null,	# External Source
    openIssues longblob not null,	# Open issues
    notes longblob not null,	# Notes
    pushState char(1) not null,	# Push State = NYD = No, Yes, Done
    initdate char(10) not null,	# Initial Submission Date
    bounces int unsigned not null,	# Bounce-count (num times bounced back to developer)
    lockUser char(8) not null,	# User who has locked record, or empty string
    lockDateTime char(16) not null,	# When user placed lock, or empty string
              #Indices
    PRIMARY KEY(qid)
);

#PushQ Users
CREATE TABLE users (
    user char(8) not null,	# User
    password char(34) not null,	# Password
    role char(8) not null,	# Role=admin,dev,qa
    contents longblob not null,	# pushq-cart contents
              #Indices
    PRIMARY KEY(user)
);
