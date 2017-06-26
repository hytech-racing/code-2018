#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "bt_lib.h"

#define BDADDR_ANY_INITIALIZER {{0, 0, 0, 0, 0, 0}}
#define BDADDR_ANY (&(bdaddr_t) BDADDR_ANY_INITIALIZER)


BT::BT(int p) {
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    loc_addr.rc_family = AF_BLUETOOTH;
    bdaddr_t any = BDADDR_ANY_INITIALIZER;
    loc_addr.rc_bdaddr = any;
    port = p;
    loc_addr.rc_channel = port;
}

// function definitions
void BT::connect() {
    bind(sock, (struct sockaddr *) &loc_addr, sizeof(struct sockaddr_rc));
    listen(sock, port);

    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
}

void BT::disconnect() {
    close(client);
    close(sock);
}

int BT::read() {
    return 0;
}

int BT::send(uint8_t *data) {
    return write(client, data, DATA_LENGTH);
}
