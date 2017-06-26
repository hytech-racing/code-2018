#include "can_lib.h"
#include "time.h"
#include "stdio.h"
#include "signal.h"

static volatile bool running = true;

void sigterm(int signo) {
    running = false;
}

int main() {
    signal(SIGTERM, sigterm);
    signal( SIGHUP, sigterm);
    signal( SIGINT, sigterm);

    MCP_CAN can;

    /*
    canframe_t *frame;
    while (running) {
        frame = can.read();
        if (frame) {
            printf("ID: %d\n", frame->can_id);
            fflush(stdout);
        }
        usleep(100000);
    }
    */
    int id = 0x07;
    unsigned char mesg[] = {1,2,3,4,5,6,7,8};
    if (can.send(id, mesg)) {
        printf("sent the message!\n");
    } else {
        printf("something didn't work\n");
    }

    return 0;
}
