#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define BOARD_RED_LED   0
#define BOARD_GREEN_LED 1
extern void board_userled(int led, bool ledon);
