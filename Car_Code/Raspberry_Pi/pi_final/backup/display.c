#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define DATA_LENGTH 256
#define PORT 10

// variables
struct sockaddr_rc loc_addr = { 0 };
struct sockaddr_rc rem_addr = { 0 };
socklen_t opt = sizeof(rem_addr);

int sock;
int client;

int bytes_read;
char buf[DATA_LENGTH] = { 0 };

// function prototypes
void init_connection();
void close_connection();

// function definitions
void init_connection() {
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = PORT;

    bind(sock, (struct sockaddr *) &loc_addr, sizeof(loc_addr));
    listen(sock, PORT);

    client = accept(sock, (struct sockaddr *) &rem_addr, &opt);
}

void close_connection() {
    close(client);
    close(sock);
}

// main()
int main(int argc, char **argv)
{
    printf("Waiting for client\n");
    init_connection();
    printf("Connected\n");
    fflush(stdout);

    while (1) {
        printf("Data to send: ");
        scanf("%s", buf);
        write(client, buf, strlen(buf));
    }

    close_connection();
    return 0;
}
