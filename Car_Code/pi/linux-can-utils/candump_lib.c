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

struct can_frame* get_frame(int s[], struct sockaddr_can *addr) {
    struct timeval *last_tv = (struct timeval*) malloc(sizeof(struct timeval));
    struct timeval *tv = (struct timeval*) malloc(sizeof(struct timeval));
    last_tv->tv_sec = 0;
    last_tv->tv_usec = 200000;
    struct can_frame *frame = (struct can_frame*) malloc(sizeof(struct can_frame));
    struct iovec *iov = (struct iovec*) malloc(sizeof(struct iovec));
    struct msghdr *msg = (struct msghdr*) malloc(sizeof(struct msghdr));
    struct cmsghdr *cmsg = (struct cmsghdr*) malloc(sizeof(struct cmsghdr));
    fd_set rdfs;
    int nbytes;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
    
    /* these settings are static and can be held out of the hot path */
    iov->iov_base = frame;
    msg->msg_name = addr;
    msg->msg_iov = iov;
    msg->msg_iovlen = 1;
    msg->msg_control = ctrlmsg;

    FD_ZERO(&rdfs); 
    FD_SET(s[0], &rdfs);
    int ret = select(s[0] + 1, &rdfs, NULL, NULL, last_tv);
    if (ret < 0) {
        printf("select failed\n");
    }

    if (FD_ISSET(s[0], &rdfs)) {
        int idx;

        /* these settings may be modified by recvmsg() */
        iov->iov_len = sizeof(*frame);
        msg->msg_namelen = sizeof(*addr);
        msg->msg_controllen = sizeof(*ctrlmsg);
        msg->msg_flags = 0;

        nbytes = recvmsg(s[0], msg, 0);
        /*
         * printf("nbytes: %d, sizeof struct: %d\n", nbytes, sizeof(struct can_frame));
        */
        fflush(stdout);
        if (nbytes < 0) {
            perror("read");
            return NULL;
        }

        if (nbytes < sizeof(struct can_frame)) {
            printf("read: incomplete CAN frame %d\n", nbytes);
            fflush(stdout);
            return NULL;
        }

        for (cmsg = CMSG_FIRSTHDR(msg); cmsg && (cmsg->cmsg_level == SOL_SOCKET); cmsg = CMSG_NXTHDR(msg,cmsg)) {
            if (cmsg->cmsg_type == SO_TIMESTAMP)
                tv = (struct timeval *)CMSG_DATA(cmsg);
            else if (cmsg->cmsg_type == SO_RXQ_OVFL)
                dropcnt[0] = *(__u32 *)CMSG_DATA(cmsg);
        }


        /* check for (unlikely) dropped frames on this specific socket */
        if (dropcnt[0] != last_dropcnt[0]) {

            __u32 frames;

            if (dropcnt[0] > last_dropcnt[0])
                frames = dropcnt[0] - last_dropcnt[0];
            else
                frames = 4294967295U - last_dropcnt[0] + dropcnt[0]; /* 4294967295U == UINT32_MAX */

            last_dropcnt[0] = dropcnt[0];
        }
    }
    return frame;
}

