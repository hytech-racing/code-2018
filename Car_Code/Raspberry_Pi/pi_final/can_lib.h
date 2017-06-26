#ifndef __CAN_LIB_H__
#define __CAN_LIB_H__

#include <stdint.h>
#include <net/if.h>
#include <linux/can/raw.h>

typedef struct sockaddr_can sockaddrcan_t;
typedef struct can_frame canframe_t;
typedef struct msghdr msghdr_t;

class CAN {
public:
    CAN();
    int read(canframe_t* read_frame);
    int send(int id, uint8_t *data, uint8_t msg_len);

private:
    int sock;
    sockaddrcan_t addr;
    canframe_t recv_frame;
    canframe_t send_frame;
    msghdr_t msg;
    fd_set rdfs;
    struct iovec iov;
    struct ifreq ifr;
    struct timeval timeout;
};

#endif
