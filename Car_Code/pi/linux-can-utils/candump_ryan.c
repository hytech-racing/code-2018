#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "terminal.h"
#include "lib.h"

#define MAXSOCK 1     /* max. number of CAN interfaces given on the cmdline */
#define MAXIFNAMES 30 /* size of receive name index to omit ioctls */
#define MAXCOL 6      /* number of different colors for colorized output */
#define ANYDEV "any"  /* name of interface to receive from any CAN interface */
#define ANL "\r\n"    /* newline in ASC mode */

#define SILENT_INI 42 /* detect user setting on commandline */
#define SILENT_OFF 0  /* no silent mode */
#define SILENT_ANI 1  /* silent mode with animation */
#define SILENT_ON  2  /* silent mode (completely silent) */

#define BOLD    ATTBOLD
#define RED     ATTBOLD FGRED
#define GREEN   ATTBOLD FGGREEN
#define YELLOW  ATTBOLD FGYELLOW
#define BLUE    ATTBOLD FGBLUE
#define MAGENTA ATTBOLD FGMAGENTA
#define CYAN    ATTBOLD FGCYAN

const char col_on [MAXCOL][19] = {BLUE, RED, GREEN, BOLD, MAGENTA, CYAN};
const char col_off [] = ATTRESET;

static char *cmdlinename[MAXSOCK];
static __u32 dropcnt[MAXSOCK];
static __u32 last_dropcnt[MAXSOCK];
static char devname[MAXIFNAMES][IFNAMSIZ+1];
static int  dindex[MAXIFNAMES];
static int  max_devname_len; /* to prevent frazzled device name output */

#define MAXANI 4
const char anichar[MAXANI] = {'|', '/', '-', '\\'};

extern int optind, opterr, optopt;

static volatile int running = 1;

void sigterm(int signo) {
    running = 0;
}

int idx2dindex(int ifidx, int socket) {

	int i;
	struct ifreq ifr;

	for (i=0; i < MAXIFNAMES; i++) {
		if (dindex[i] == ifidx)
			return i;
	}

	/* create new interface index cache entry */

	/* remove index cache zombies first */
	for (i=0; i < MAXIFNAMES; i++) {
		if (dindex[i]) {
			ifr.ifr_ifindex = dindex[i];
			if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
				dindex[i] = 0;
		}
	}

	for (i=0; i < MAXIFNAMES; i++)
		if (!dindex[i]) /* free entry */
			break;

	if (i == MAXIFNAMES) {
		fprintf(stderr, "Interface index cache only supports %d interfaces.\n",
		       MAXIFNAMES);
		exit(1);
	}

	dindex[i] = ifidx;

	ifr.ifr_ifindex = ifidx;
	if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
		perror("SIOCGIFNAME");

	if (max_devname_len < strlen(ifr.ifr_name))
		max_devname_len = strlen(ifr.ifr_name);

	strcpy(devname[i], ifr.ifr_name);

#ifdef DEBUG
	printf("new index %d (%s)\n", i, devname[i]);
#endif

	return i;
}

int main() {
    fd_set rdfs;
    int s[MAXSOCK];
    int bridge = 0;
    useconds_t bridge_delay = 0;
    unsigned char timestamp = 0;
    unsigned char dropmonitor = 0;
    unsigned char silent = SILENT_OFF;
    unsigned char silentani = 0;
    unsigned char color = 0;
    unsigned char view = 0;
    unsigned char log = 0;
    unsigned char logfrmt = 0;
    int count = 0;
    int rcvbuf_size = 0;
    int opt, ret;
    int currmax = 1;
    int numfilter;
    char *ptr, *nptr;
    struct sockaddr_can addr;
    struct sockaddr_can client_addr;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct can_filter *rfilter;
    can_err_mask_t err_mask;
    struct can_frame frame;
    int nbytes, i;
    struct ifreq ifr;
    struct timeval tv, last_tv;
    FILE *logfile = NULL;

    signal(SIGTERM, sigterm);
    signal(SIGHUP, sigterm);
    signal(SIGINT, sigterm);

    last_tv.tv_sec  = 0;
    last_tv.tv_usec = 200000;

    //******************************
    //Begin Ryan's copy-pasting
    //******************************

    s[i] = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s[i] < 0) {
            perror("socket");
            return 1;
    }

    addr.can_family = AF_CAN;

    memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    strcpy(ifr.ifr_name, "can0");

    if (strcmp(ANYDEV, ifr.ifr_name)) {
        if (ioctl(s[i], SIOCGIFINDEX, &ifr) < 0) {
            perror("SIOCGIFINDEX");
            exit(1);
        }
        addr.can_ifindex = ifr.ifr_ifindex;
    } else
        addr.can_ifindex = 0; /* any can interface */

    ret = bind(s[i], (struct sockaddr *)&addr, sizeof(addr));
    //printf("bind result: %d\n", addr.can_ifindex);
    //fflush(stdout);

    /* these settings are static and can be held out of the hot path */
    iov.iov_base = &frame;
    msg.msg_name = &addr;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &ctrlmsg;

    while (running) {
        i = 0;
        FD_ZERO(&rdfs);
        FD_SET(s[i], &rdfs);

        ret = select(s[currmax-1]+1, &rdfs, NULL, NULL, &last_tv);

        //printf("while loop start, ret:%d\n", ret);
        //fflush(stdout);

        if (ret < 0) {
            //perror("select");
            printf("select failed\n");
            running = 0;
            continue;
        }

        for (i = 0; i < currmax; i++) {  /* check all CAN RAW sockets */
            //printf("for loop start\n");
            //fflush(stdout);
            if (FD_ISSET(s[i], &rdfs)) {
                //printf("if statement hit\n");
                //fflush(stdout);
                int idx;

                /* these settings may be modified by recvmsg() */
                iov.iov_len = sizeof(frame);
                msg.msg_namelen = sizeof(addr);
                msg.msg_controllen = sizeof(ctrlmsg);
                msg.msg_flags = 0;

                nbytes = recvmsg(s[i], &msg, 0);
                //printf("nbytes: %d, sizeof struct: %d\n", nbytes, sizeof(struct can_frame));
                //fflush(stdout);
                if (nbytes < 0) {
                    perror("read");
                    return 1;
                }


                if (nbytes < sizeof(struct can_frame)) {
                    printf("read: incomplete CAN frame\n");
                    fflush(stdout);
                    return 1;
                }

                if (count && (--count == 0))
                    running = 0;

                if (bridge) {
                    if (bridge_delay)
                        usleep(bridge_delay);

                    nbytes = write(bridge, &frame, sizeof(struct can_frame));
                    if (nbytes < 0) {
                        perror("bridge write");
                        return 1;
                    } else if (nbytes < sizeof(struct can_frame)) {
                        fprintf(stderr,"bridge write: incomplete CAN frame\n");
                        return 1;
                    }
                }

                for (cmsg = CMSG_FIRSTHDR(&msg); cmsg && (cmsg->cmsg_level == SOL_SOCKET); cmsg = CMSG_NXTHDR(&msg,cmsg)) {
                    if (cmsg->cmsg_type == SO_TIMESTAMP)
                        tv = *(struct timeval *)CMSG_DATA(cmsg);
                    else if (cmsg->cmsg_type == SO_RXQ_OVFL)
                        dropcnt[i] = *(__u32 *)CMSG_DATA(cmsg);
                }


                /* check for (unlikely) dropped frames on this specific socket */
                if (dropcnt[i] != last_dropcnt[i]) {

                    __u32 frames;

                    if (dropcnt[i] > last_dropcnt[i])
                        frames = dropcnt[i] - last_dropcnt[i];
                    else
                        frames = 4294967295U - last_dropcnt[i] + dropcnt[i]; /* 4294967295U == UINT32_MAX */

                    if (silent != SILENT_ON)
                        printf("DROPCOUNT: dropped %d CAN frame%s on '%s' socket (total drops %d)\n",
                               frames, (frames > 1)?"s":"", cmdlinename[i], dropcnt[i]);

                    if (log)
                        fprintf(logfile, "DROPCOUNT: dropped %d CAN frame%s on '%s' socket (total drops %d)\n",
                            frames, (frames > 1)?"s":"", cmdlinename[i], dropcnt[i]);

                    last_dropcnt[i] = dropcnt[i];
                }

                idx = idx2dindex(addr.can_ifindex, s[i]);

                /*if (log) {
                                log CAN frame with absolute timestamp & device
                    fprintf(logfile, "(%ld.%06ld) ", tv.tv_sec, tv.tv_usec);
                    fprintf(logfile, "%*s ", max_devname_len, devname[idx]);
                                without seperator as logfile use-case is parsing
                    fprint_canframe(logfile, &frame, "\n", 0);
                }

                if (logfrmt) {
                             print CAN frame in log file style to stdout
                    printf("(%ld.%06ld) ", tv.tv_sec, tv.tv_usec);
                    printf("%*s ", max_devname_len, devname[idx]);
                    fprint_canframe(stdout, &frame, "\n", 0);
                    goto out_fflush;                                no other output to stdout
                }*/

                if (silent != SILENT_OFF){
                    if (silent == SILENT_ANI) {
                        printf("%c\b", anichar[silentani%=MAXANI]);
                        silentani++;
                    }
                    goto out_fflush; /* no other output to stdout */
                }
                //printf(" %s", (color>2)?col_on[idx%MAXCOL]:"");

                //printf(" %s", (color && (color<3))?col_on[idx%MAXCOL]:"");
                //printf("%*s", max_devname_len, devname[idx]);
                //printf("%s  ", (color==1)?col_off:"");

                fprint_long_canframe(stdout, &frame, NULL, view);

                //printf("%s", (color>1)?col_off:"");
                printf("\n");
            }
            out_fflush:
                fflush(stdout);
            }
        }
    //printf("not in the loop...\n");
    for (i=0; i<currmax; i++)
        close(s[i]);

    if (bridge)
        close(bridge);

    if (log)
        fclose(logfile);

    return 0;
}
