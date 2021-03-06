#include "components/Lift.h"
#include <SmartDashboard/SmartDashboard.h>
using namespace components;

// Constructor for Lift class
Lift::Lift(int m1, int m2) {
  motor1 = new TalonSRX(m1);
  motor2 = new TalonSRX(m2);

  motor1->ConfigContinuousCurrentLimit(30, 0);
  motor2->ConfigContinuousCurrentLimit(30, 0);
  motor1->ConfigPeakCurrentLimit(40, 0);
  motor2->ConfigPeakCurrentLimit(40, 0);
  motor1->ConfigPeakCurrentDuration(75, 0);
  motor2->ConfigPeakCurrentDuration(75, 0);
  motor1->EnableCurrentLimit(true);
  motor2->EnableCurrentLimit(true);

  motor2->Set(ControlMode::Follower, m1);

  motor1->ConfigNominalOutputForward(0, 0);
	motor1->ConfigNominalOutputReverse(0, 0);
	motor1->ConfigPeakOutputForward(1, 0);
	motor1->ConfigPeakOutputReverse(-1, 0);
  motor1->Config_kF(0, 1023.0/maxVelocity, 10);
  motor1->Config_kP(0, 0.25, 10); // USED TO BE 0.2
  motor1->Config_kI(0, 0, 10);
  motor1->Config_kD(0, 0, 10);
  motor1->ConfigMotionAcceleration(maxVelocity*4, 10);
  motor1->ConfigMotionCruiseVelocity(maxVelocity*5, 10);
  motor1->SetSensorPhase(true);
  motor1->SetSelectedSensorPosition(0, 0, 10);

  topSwitch = new DigitalInput(1);
  lowSwitch = new DigitalInput(0);
}

// Move lift to high position (for scale)
void Lift::SetHighPosition() {
  if(!encoderOverride) {
    pos = 2;
    if(highPosition < GetLiftPosition()) {
      motor1->ConfigMotionAcceleration(maxVelocity*2, 10);   //USED TO BE 2
      motor1->ConfigMotionCruiseVelocity(maxVelocity*2, 10);  //USED TO BE 2
      lastpos = pos;
    } else if(highPosition > GetLiftPosition()) {
      motor1->ConfigMotionAcceleration(maxVelocity*10, 10);    //USED TO BE 5
      motor1->ConfigMotionCruiseVelocity(maxVelocity*10, 10);  //USED TO BE 6
      lastpos = pos;
    }
    //motor1->Set(ControlMode::PercentOutput, 0); COMMENTED THESE LINES OUT!!!!
    motor1->Set(ControlMode::MotionMagic, highPosition);
    manualMode = false;
  }
}

// Move lift to mid position (for switch)
void Lift::SetMidPosition() {
  if(!encoderOverride) {
    pos = 1;
    if(midPosition < GetLiftPosition()) {
      motor1->ConfigMotionAcceleration(maxVelocity*2, 10);   //USED TO BE 2
      motor1->ConfigMotionCruiseVelocity(maxVelocity*2, 10);  //USED TO BE 2
      lastpos = pos;
    } else if(midPosition > GetLiftPosition()) {
      motor1->ConfigMotionAcceleration(maxVelocity*10, 10);    //USED TO BE 5
      motor1->ConfigMotionCruiseVelocity(maxVelocity*10, 10);  //USED TO BE 6
      lastpos = pos;
    }
    //motor1->Set(ControlMode::PercentOutput, 0);
    motor1->Set(ControlMode::MotionMagic, midPosition);
    manualMode = false;
  }
}

// Move lift to low position
void Lift::SetLowPosition() {
  if(!encoderOverride) {
    pos = 0;
    if(0 < GetLiftPosition()) {
      motor1->ConfigMotionAcceleration(maxVelocity*2, 10);   //USED TO BE 2
      motor1->ConfigMotionCruiseVelocity(maxVelocity*2, 10);  //USED TO BE 2
      lastpos = pos;
    } else if(0 > GetLiftPosition()) {
      motor1->ConfigMotionAcceleration(maxVelocity*10, 10);    //USED TO BE 5
      motor1->ConfigMotionCruiseVelocity(maxVelocity*10, 10);  //USED TO BE 6
      lastpos = pos;
    }
    //motor1->Set(ControlMode::PercentOutput, 0);
    motor1->Set(ControlMode::MotionMagic, 0);
    manualMode = false;
  }
}

void Lift::SetCustomPosition(int pos) {
  if(!encoderOverride) {
    //motor1->Set(ControlMode::PercentOutput, 0)
    motor1->Set(ControlMode::MotionMagic, pos);
    pos = 3;
    manualMode = false;
  }
}

void Lift::Stop() {
  if(!encoderOverride) {
    manualMode = true;
    SetSpeed(0);
  }
}

// Set speed of Lift class motors
void Lift::SetSpeed(double speed) {
   if(fabs(speed) < deadzone) {
     speed = 0;
     if(manualMode) {
       if(!encoderOverride) motor1->Set(ControlMode::MotionMagic, GetLiftPosition());
       manualMode = false;
     }
   } else {
     manualMode = true;
     speed *= fabs(speed);

     //Check if limit switches are enabled
     if(!encoderOverride) {
       //Lower lift boundries
       if(lowSwitch->Get() && speed < 0) speed = 0; //limit switch
       if(GetLiftPosition() < 6000 && speed < 0) speed *= 0.2;  //Soft speed limit
       if(motor1->GetSelectedSensorVelocity(0) < -300 && GetLiftPosition() < 7000) speed = 0;  //Hardstop
       //Upper lift boundries
       if(topSwitch->Get() && speed > 0) speed = 0; //limit switch
       if(GetLiftPosition() > 21000 && speed > 0) speed *= 0.85; //Soft speed limit
       if(motor1->GetSelectedSensorVelocity(0) > 800 && GetLiftPosition() > 22000) speed = 0.2; //Hardstop
       if(motor1->GetSelectedSensorVelocity(0) > 1200 && GetLiftPosition() > 22000) speed = 0.05; //Hardstop
     }

     motor1->Set(ControlMode::PercentOutput, speed);
     pos = 3;
   }
 }

// Reset Lift class motor encoder
void Lift::ResetEncoder() {
  motor1->SetSelectedSensorPosition(0, 0, 10);
}

// Override MotionMagic and limit switch control in favour of PercentOutput
void Lift::OverrideLift(bool overridden) {
  encoderOverride = overridden;
}

// Run periodic tasks
void Lift::RunPeriodic() {
  if(lowSwitch->Get() && motor1->GetSelectedSensorVelocity(0) > 0) ResetEncoder();

  if(GetLiftPosition() < 4000) motor1->ConfigPeakOutputReverse(-0.3, 0);
  else motor1->ConfigPeakOutputReverse(-1, 0);

  // if(pos < lastpos) {
  //   motor1->ConfigMotionAcceleration(maxVelocity*2.5, 10);   //USED TO BE 2
  //   motor1->ConfigMotionCruiseVelocity(maxVelocity*2.5, 10);  //USED TO BE 2
  //   lastpos = pos;
  // } else if(pos > lastpos) {
  //   motor1->ConfigMotionAcceleration(maxVelocity*10, 10);    //USED TO BE 5
  //   motor1->ConfigMotionCruiseVelocity(maxVelocity*10, 10);  //USED TO BE 6
  //   lastpos = pos;
  // }

  SmartDashboard::PutNumber("Lift encoder", GetLiftPosition());
  SmartDashboard::PutNumber("Lift velocity", motor1->GetSelectedSensorVelocity(0));
  SmartDashboard::PutNumber("Lift percent", (motor1->GetSelectedSensorPosition(0))/25000.0);
  SmartDashboard::PutBoolean("topSwitch", topSwitch->Get());
  SmartDashboard::PutBoolean("inMiddle", !topSwitch->Get()&&!lowSwitch->Get());
  SmartDashboard::PutBoolean("lowSwitch", lowSwitch->Get());
  SmartDashboard::PutNumber("Lift speed", motor1->GetMotorOutputPercent());
  SmartDashboard::PutBoolean("Manual Mode", manualMode);
  switch(pos) {
    case 0:
      SmartDashboard::PutString("Lift Position", "Low");
      break;
    case 1:
      SmartDashboard::PutString("Lift Position", "Mid");
      break;
    case 2:
      SmartDashboard::PutString("Lift Position", "High");
      break;
    default:
      SmartDashboard::PutString("Lift Position", "Manual");
      break;
  }
}
