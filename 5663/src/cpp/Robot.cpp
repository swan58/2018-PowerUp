/* Need to Test / add
* - full diagnostics check
* - get dashboards up and running add imu indicator
*  - turn with encoders autoFunction if imu not connected
*  - far auto modes
*/


#include "WPILib.h"
#include "AHRS.h"
#include "curtinfrc/math.h"
#include <iostream>
#include <string>
#include <SmartDashboard/SmartDashboard.h>
#include <PowerDistributionPanel.h>
#include <PIDOutput.h>
#include <I2C.h>
#include <pathfinder.h>
#include <Timer.h>
// #include <networktables/NetworkTable.h>
// #include <networktables/NetworkTableInstance.h>
#include "components/Drive.h"
#include "components/Lift.h"
#include "components/Ramp.h"
#include "components/Manipulator.h"
#include "components/Climber.h"
#include "autonomous/Autonomous.h"

using namespace frc;
using namespace curtinfrc;
using namespace std;
using namespace components;
using namespace cs;
using namespace autonomous;
using namespace nt;

class Robot : public IterativeRobot {
  UsbCamera camera;
  XboxController *xbox, *xbox2;
  PowerDistributionPanel *pdp;
  SendableChooser<int*> *AutoChooser; // Choose auto mode
  SendableChooser<int*> *StartingPosition; // Choose starting position
  SendableChooser<int*> *AutoWait; // Time to wait before starting auto
  SendableChooser<int*> *FarMode; // Chooser to disable far switch/scale
  Drive *drive;
  Lift *lift;
  Ramp *ramp;
  Manipulator *man;
  Climber *climber;
  Compressor *compressor;
  Autonomous *auton;
  I2C *arduino;
  Timer *timer;
  Joystick *station;

public:
  uint8_t message = 72;
  double maxspeed = 1;
  bool pressedTurn = false;

  void RobotInit() {
    camera = CameraServer::GetInstance()->StartAutomaticCapture();
    camera.SetResolution(640, 480);

    xbox = new XboxController(0);
    xbox2 = new XboxController(1);

    pdp = new PowerDistributionPanel(0);

    AutoChooser = new SendableChooser<int*>;
    StartingPosition = new SendableChooser<int*>;
    AutoWait = new SendableChooser<int*>;

    drive = new Drive(1, 2, 3,  //left
                      6, 5, 4,  //right
                      0, 1);    //solenoid
    lift = new Lift(7, 8);
    ramp = new Ramp(6, 7, 4, 5);
    man = new Manipulator(0, 2, 3 );

    compressor = new Compressor(0);
    compressor->SetClosedLoopControl(true);

    auton = new Autonomous(drive, lift, man);

    arduino = new I2C(arduino->kOnboard, 100);
    arduino->WriteBulk(&message, 1);

    timer = new Timer(); timer->Start();

    AutoChooser->AddObject("Cross Baseline",(int*) 0);
    AutoChooser->AddDefault("Switch",(int*) 1);
    AutoChooser->AddObject("Scale",(int*) 2);
    SmartDashboard::PutData("AutoChooser", AutoChooser);

    StartingPosition->AddObject("Left (1)", (int*) 1);
    StartingPosition->AddDefault("Middle (2)", (int*) 2);
    StartingPosition->AddObject("Right (3)", (int*) 3);
    SmartDashboard::PutData("StartingPosition", StartingPosition);

    AutoWait->AddDefault("0S",(int*) 0);
    AutoWait->AddObject("1S",(int*) 1);
    AutoWait->AddObject("2S",(int*) 2);
    AutoWait->AddObject("3S",(int*) 3);
    AutoWait->AddObject("4S",(int*) 4);
    AutoWait->AddObject("5S",(int*) 5);
    AutoWait->AddObject("6S",(int*) 6);
    AutoWait->AddObject("7S",(int*) 7);
    AutoWait->AddObject("8S",(int*) 8);
    AutoWait->AddObject("9S",(int*) 9);
    SmartDashboard::PutData("AutoWait", AutoWait);

    FarMode->AddDefault("Enable Far Switch",(int*) 1);
    FarMode->AddObject("Disable Far Switch",(int*) 0);
    SmartDashboard::PutData("FarMode", FarMode);

    station = new Joystick(2);
  }

  void AutonomousInit() {
    drive->SetFastGear();
    drive->Stop();
    drive->ResetEncoder();
    lift->ResetEncoder();
    lift->SetLowPosition();
    ramp->Reset();

    auton->SetFarMode((int)FarMode->GetSelected());
    auton->SetStageOne((int)AutoChooser->GetSelected(), (int)StartingPosition->GetSelected(), (int)AutoWait->GetSelected());
    auton->ChooseStage();
  }

  void AutonomousPeriodic() {
    message = 72;
    SmartDashboard::PutBoolean("transaction", arduino->Transaction(&message, 1, NULL, 0));
    auton->RunPeriodic();
  }

  void TeleopInit() {
    drive->SetFastGear();
    drive->Stop();
    lift->Stop();
    man->SetIntakeSpeed(0);
    ramp->Reset();
    timer->Reset();
  }

  void TeleopPeriodic() {
//———[controller 1]—————————————————————————————————————————————————————————————
  //———[drivetrain]—————————————————————————————————————————————————————————————
    if(lift->GetLiftPosition() > 11000) drive->SetSlowGear();

    if(lift->GetLiftPosition() > 14000) maxspeed = 0.35;
    else if(9000 <= lift->GetLiftPosition() && lift->GetLiftPosition() <= 14000) {
      maxspeed = (-(1.0/300.0) * (45.0 - ( sqrt(15.0) * sqrt(15835.0-lift->GetLiftPosition()) ) )) - 0.1;
    }
    else if(lift->GetLiftPosition() < 9000) maxspeed = 1;

    if(xbox->GetAButton() && lift->GetLiftPosition() < 9000 && !pressedTurn) {
      pressedTurn = drive->TurnAngle(1, 180);
    }
    else {
      drive->TankDrive(-xbox->GetY(xbox->kLeftHand) + xbox->GetTriggerAxis(xbox->kLeftHand)*0.5,
        -xbox->GetY(xbox->kRightHand) + xbox->GetTriggerAxis(xbox->kRightHand)*0.5, true, maxspeed);
      drive->turning = false;
      if(!xbox->GetAButton()) pressedTurn = false;
    }

    // if(xbox->GetYButtonPressed() || xbox->GetBumperPressed(xbox->kRightHand) || xbox->GetBumperPressed(xbox->kLeftHand)) {
    //   drive->ToggleGear();
    // }
    if(xbox->GetYButton() || xbox->GetBumper(xbox->kRightHand) || xbox->GetBumper(xbox->kLeftHand)) {
      drive->SetSlowGear();
    } else {
      drive->SetFastGear();
    }

//———[controller 2]—————————————————————————————————————————————————————————————
  //———[lift]———————————————————————————————————————————————————————————————————
    if(xbox2->GetAButton()) {
      lift->SetLowPosition();
    } else if(xbox2->GetBButton()) {
      lift->SetMidPosition();
    } else if(xbox2->GetYButton()) {
      lift->SetHighPosition();
    }
    lift->SetSpeed(-xbox2->GetY(xbox2->kRightHand));

    // Lift Safety Override
    lift->OverrideLift(station->GetRawButton(4));

  //———[manipulator]————————————————————————————————————————————————————————————
    if(xbox2->GetBumper(xbox2->kLeftHand)) {
      if(fabs(xbox2->GetY(xbox2->kLeftHand)) > 0.18) {
        man->OverrideIntake(true);
        man->SetIntakeSpeed(-xbox2->GetY(xbox2->kLeftHand));
      } else {
        man->OverrideIntake(false);
      }
      man->Release();
    } else {
      man->Restrain();
      man->SetIntakeSpeed(-xbox2->GetY(xbox2->kLeftHand));
    }

  //———[ramp]———————————————————————————————————————————————————————————————————
    if(xbox->GetXButton() && xbox2->GetXButton() && timer->GetMatchTime() < 30)  ramp->ConfirmIntentionalDeployment();
    //if(xbox->GetXButton() && xbox2->GetXButton() && timer->HasPeriodPassed(135)) ramp->ConfirmIntentionalDeployment(); //Use if above line broke

    if(xbox->GetStartButton() || xbox2->GetStartButton()) ramp->ReleaseFoulStopper();
    if(xbox->GetBackButton() || xbox2->GetBackButton()) ramp->ResetFoulStopper();

    if(station->GetRawButton(4)) ramp->ReleaseFoulStopper();
    if(station->GetRawButton(1)) ramp->ConfirmIntentionalDeployment();

  //———[periodic]———————————————————————————————————————————————————————————————
    message = 76;
    SmartDashboard::PutBoolean("Joystickstation1,", station->GetRawButton(1));
    SmartDashboard::PutBoolean("Joystickstation4,", station->GetRawButton(4));
    SmartDashboard::PutBoolean("Joystickstation3,", station->GetRawButton(3));

    SmartDashboard::PutBoolean("transaction", arduino->Transaction(&message, 1, NULL, 0));
    SmartDashboard::PutBoolean("rampsReady", (timer->GetMatchTime() < 30));

    drive->RunPeriodic();
    lift->RunPeriodic();
    man->RunPeriodic();
   }
};

START_ROBOT_CLASS(Robot)
