#pragma once

#include "WPILib.h"

#define XBOX_CONTROL 1

namespace Map { // Map ports
  struct Controllers {
    #ifdef XBOX_CONTROL
    static const int xbox = 0;
    static const double deadzone = 0.04;

    #elif JOY_CONTROL
    static const int joy[2] = {0, 1};
    static const double deadzone = 0.15;

    #endif
  };

  struct Sensors { };

  struct Motors {
    static constexpr int left_motors[2] = {32, 34};
    static constexpr int right_motors[2] = {35, 36};

    static constexpr int belev_motors[1] = {37};

    static constexpr int intake_motors[2][1] = {{38}, {39}};
  };

  struct Pneumatics {
    static constexpr int intake_solenoids[2][2] = {{1, 2}, {2, 4}};
    static constexpr int claw_solenoids[1][2] = {{0, 0}};
  };
}
