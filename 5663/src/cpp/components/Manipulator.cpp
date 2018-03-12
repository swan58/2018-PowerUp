#include "components/Manipulator.h"
#include <Spark.h>
using namespace components;

// Constructor for Manipulator class
Manipulator::Manipulator(int intakePortL, int intakePortR, int restrainerFwd, int restrainerRev) {
  intakeLeft = new Spark(intakePortL);
  intakeRight = new Spark(intakePortR);
  restrainer = new DoubleSolenoid(0, restrainerFwd, restrainerRev);
}

// Set speed of intake motor
void Manipulator::SetIntakeSpeed(double speed, double bias) {
  if(fabs(speed) > deadzone) {
    if(fabs(bias) > deadzone) {
      intakeSpeedL = speed+(bias*0.5);
      intakeSpeedR = speed-(bias*0.5);
    } else {
      intakeSpeedL = speed;
      intakeSpeedR = speed;
    }
  } else {
    intakeSpeedL = 0;
    intakeSpeedR = 0;
  }
}

// Extend actuator to close restrainer and restrain power cube
void Manipulator::Restrain() {
  restrainer->Set(restrainer->kForward);
  SmartDashboard::PutNumber("arm", restrainer->Get());
}

// Retract actuator to open restrainer and grab power cube
void Manipulator::Release() {
  if(!intakeOverride) {
    intakeSpeedL = -1;
    intakeSpeedR = -1;
  }
  restrainer->Set(restrainer->kReverse);
  SmartDashboard::PutNumber("arm", restrainer->Get());
}

void Manipulator::OverrideIntake(bool overridden) {
  intakeOverride = overridden;
}

void Manipulator::RunPeriodic() {
  intakeLeft->Set(intakeSpeedL);
  intakeRight->Set(intakeSpeedR);
  SmartDashboard::PutNumber("Intake Speed L", intakeSpeedL);
  SmartDashboard::PutNumber("Intake Speed R", intakeSpeedR);
  if(restrainer->Get() == restrainer->kReverse) SmartDashboard::PutBoolean("Open/Closed", true); //Open
  else SmartDashboard::PutBoolean("Open/Closed", false); //Closed
}
