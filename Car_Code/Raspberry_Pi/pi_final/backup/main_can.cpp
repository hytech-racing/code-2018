#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "can_lib.h"
#include "bt_lib.h"

#define RED_CONSOLE "\033[31;1m"
#define GREEN_CONSOLE "\033[32;1m"
#define YELLOW_CONSOLE "\033[33;1m"
#define BLUE_CONSOLE "\033[34;1m"
#define NO_COLOR_CONSOLE "\033[0m"

const std::string CONSOLE_COLORS[] = {RED_CONSOLE, GREEN_CONSOLE,
                                      YELLOW_CONSOLE, BLUE_CONSOLE};

// function prototypes
void print(canframe_t* frame);

int main() {
    CAN can;
    BT bt(10);
    bt.connect();

    canframe_t *frame = (canframe_t*) malloc(sizeof(canframe_t));
    uint8_t buffer[] = {70, 85, 67, 75, 32, 89, 79, 85};

    while (1) {
        if (can.read(frame) > 0) {
            std::cout << "Error reading message" << std::endl;
        }
        print(frame);
        if (can.send(0x60, buffer, 8) > 0) {
            std::cout << "Error sending message" << std::endl;
        }
        bt.send(buffer, 2);
        usleep(10000);
    }

    free(frame);
    bt.disconnect();
    return 0;
}

void print(canframe_t* frame) {
    if (frame && frame->can_dlc > 0) {
        for (uint8_t i = 0; i < frame->can_dlc; ++i) {
            std::cout << CONSOLE_COLORS[rand() % 4];
            std::cout << (int) frame->data[i] << " ";
        }
        std::cout << NO_COLOR_CONSOLE << std::endl;
    } else {
        std::cout << "Can frame is NULL or has an empty message" << std::endl;
    }
}
