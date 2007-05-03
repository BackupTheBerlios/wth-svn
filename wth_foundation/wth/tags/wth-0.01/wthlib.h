/* wthlib.h */
#include "global.h" 
#include "config.h"
#include "const.h" 
#include "dates.h"
#include "serial.h"
#include "util.h"
#include "unp.h"


int datacorr(u_char *data, int *mdat);
int chkframe(u_char *data, int *mdat);
int getcd(char *data, int *mdat, struct cmd *pcmd);
int getrd(char *data, int *mdat, struct cmd *pcmd);
int wstat(char *data, int mdat, struct DCFstruct *DCF, struct sensor sens[], struct wstatus *setting);
int dcftime(u_char *data, int ndat);
int getone(char *data, int ndat, struct sensor sens[], long setno, struct DCFstruct DCF);
int pdata(struct sensor sens[], int snum);
int wcmd (struct sensor sens[], struct cmd *pcmd, int *setno);
int initsens(struct sensor sens[]);
