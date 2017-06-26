#include "candump_lib.c"
#include "time.h"

int main() {
    struct sockaddr_can addr;
    struct ifreq ifr;

    signal(SIGTERM, sigterm);
    signal(SIGHUP, sigterm);
    signal(SIGINT, sigterm);

    int s[5];
    s[0] = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s[0] < 0) {
        printf("socket error\n");
        return 1;
    }
    addr.can_family = AF_CAN;

    memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    strcpy(ifr.ifr_name, "can0");

    if (strcmp(ANYDEV, ifr.ifr_name)) {
        if (ioctl(s[0], SIOCGIFINDEX, &ifr) < 0) {
            printf("SIOCGIFINDEX\n");
            return 1;
        }
        addr.can_ifindex = ifr.ifr_ifindex;
    } else
        addr.can_ifindex = 0;

    int ret = bind(s[0], (struct sockaddr *)&addr, sizeof(addr));
    struct can_frame *frame;
    while(1) {
        frame = get_frame(s, &addr);
        if (frame) {
            printf("ID: %d\n", frame->can_id);
            fflush(stdout);
        }
        usleep(100000);
    }
 
    return 0;
}
