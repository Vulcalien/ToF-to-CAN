/*
 * game_timer.cpp
 */

#include "game_timer.h"
//#include "telemetry.h"
#include <nuttx/clock.h>
#include <stdio.h>

#ifdef ROBOT_grande
#include "automation_grande.h"
#include "goals_grande.h"
#endif

#ifdef ROBOT_piccolo
#include "automation_piccolo.h"
#endif

void GameTimer::run() {
  m_starttime = TICK2SEC((float)clock_systime_ticks());
  m_started   = true;
  for (int i = 0; i < 100 / 5; i++) {
    sleep(5);
    printf("[GameTimer] %d secondi\n", (i + 1) * 5);
    // convalida la posizione solo dopo i primi 5 secondi
    // if (i == 0)
    //   pose_sender->set_position_valid(true);
  }
  m_game_end = true;

  //Automation::all_off_end_game();
  printf("[GameTimer] FINE GARA!!\n");
  for (;;) {
    delay_ms(1000);
  }
}

float GameTimer::get() {
  if (m_started) {
    float f = TICK2SEC((float)clock_systime_ticks());
    return f - m_starttime;
  }

  return TICK2SEC((float)clock_systime_ticks());
}
