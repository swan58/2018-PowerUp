#pragma once

#include "WPILib.h"
#include <ctre/Phoenix.h>
#include <Spark.h>
/*
* Intake:
* Wheels In = negative value
* Wheels Out = postive value
* Grabber Close = kForward
* Grabber Open = kReverse
*/

namespace components {
  class Manipulator {
    DoubleSolenoid *restrainer;

    public:
      Manipulator(int intakePortL, int intakePortR, int restrainerFwd, int restrainerRev);
      Manipulator& operator=(const Manipulator&); //Copy Constructor
      void SetIntakeSpeed(double speed, double bias=0);
      void Restrain();
      void Release();
      void RunPeriodic();
      void OverrideIntake(bool overridden);

    private:
      TalonSRX *left, *right;
      bool intakeOverride = false;
      double intakeSpeedL = 0.0;
      double intakeSpeedR = 0.0;
      double deadzone = 0.1;
  };
}
