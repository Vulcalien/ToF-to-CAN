#pragma once

#include "main.h"

#define TRANSMISSION_TIMING_ON_DEMAND  0
#define TRANSMISSION_TIMING_CONTINUOUS 1

extern int transmission_timing;

extern int transmission_set_timing(int timing);
