#ifndef __BT_LIB_H__
#define __BT_LIB_H__

#include <stdint.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

class BT {
public:
    BT(int p);
    void connect();
    void disconnect();
    int read();
    int send(uint8_t* data);
    int send(uint8_t* data, uint8_t length);
    static const int DATA_LENGTH = 8;

private:
    int sock;
    int client;
    int port;

    uint8_t buf[DATA_LENGTH] = { 0 };

    struct sockaddr_rc loc_addr = { 0 };
    struct sockaddr_rc rem_addr = { 0 };
    socklen_t opt = sizeof(rem_addr);
};

#endif
