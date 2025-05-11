#pragma once

#include "main.h"

// returns 0 on success, nonzero otherwise
extern int tof_init(void);

extern int tof_set_resolution(int resolution);
extern int tof_set_ranging_frequency(int frequency_hz);
extern int tof_set_ranging_mode(int mode);
extern int tof_set_sharpener(int sharpener_percent);
