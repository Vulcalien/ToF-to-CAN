/*
 * game_timer.h
 */

#pragma once

#include "sporadic_task.h"

class GameTimer : public SporadicThread {
public:
  GameTimer()
      : SporadicThread("game_timer"), m_started(false), m_game_end(false){};
  float        get();
  virtual void run();
  bool         is_game_started() { return m_started; };
  bool         game_end() { return m_game_end; };

private:
  bool  m_started;
  bool  m_game_end;
  float m_starttime;
};

extern GameTimer *game_timer;

#define delay_ms(x) usleep((x)*1000l)
