struct DCFstruct {
    int stat;
    int sync;
    time_t time;
};

struct wstatus {
    int numsens;
    int intv;
    int version;
};

struct dataset {
    time_t time;
    int sign;
    int hundreds;
    int tens;
    int units;
    int tenths;
};

struct sensor {
    int status;
    char *type;
    struct dataset mess[MAXDATA];
};

struct cmd {
    int command;
    int argcmd;
    int netflg;
    char *hostname;
    char *port;
};


