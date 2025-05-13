#pragma once

#include "main.h"

#define TRANSMIT_ON_DEMAND  0
#define TRANSMIT_CONTINUOUS 1

extern int transmit_timing;

extern int transmit_set_timing(int timing);
