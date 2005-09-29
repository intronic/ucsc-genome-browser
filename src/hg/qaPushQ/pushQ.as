table pushQ
"Push Queue"
    (
    char [6]  qid;       "Queue Id"
    char [6]  pqid;       "Parent Queue Id if split off another"
    char [1]  priority;   "Priority"
    uint      rank;       "Rank for display sort"
    char [10] qadate;     "QA (re)-submission date"
    char [1]  newYN;      "new (track)?"
    string    track;      "Track"
    string    dbs;        "Databases"
    lstring   tbls;       "Tables"
    string    cgis;       "CGI(s)"
    lstring   files;      "File(s)"
    uint      sizeMB;     "Size MB"
    char [20] currLoc;    "Current Location"
    char [1]  makeDocYN;  "MakeDoc verified?"
    char [50] onlineHelp; "Online Help"
    char [1]  ndxYN;      "Index verified?"
    char [1]  joinerYN;   "all.joiner verified?"
    string    stat;       "Status"
    char [50] sponsor;    "Sponsor"
    char [50] reviewer;   "QA Reviewer"
    char [128] extSource; "External Source"
    lstring   openIssues; "Open issues"
    lstring   notes;      "Notes"
    char [1]  pushState;  "Push State = NYD = No, Yes, Done"
    char [10] initdate;   "Initial Submission Date"
    char [10] lastdate;   "Last QA Date"
    uint      bounces;    "Bounce-count (num times bounced back to developer)"
    char[8]   lockUser;   "User who has locked record, or empty string"
    char[16]  lockDateTime; "When user placed lock, or empty string"
    lstring   releaseLog;  "Release Log entry"
    )

table users
"PushQ Users"
    (
    char[8]  user;        "User"
    char[34] password;    "Password" 
    char[8]  role;        "Role=admin,dev,qa"
    lstring  contents;    "pushq-cart contents"
    )
