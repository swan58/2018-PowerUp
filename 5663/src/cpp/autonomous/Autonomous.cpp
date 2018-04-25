#include "autonomous/Autonomous.h"

using namespace autonomous;
using namespace components;

// Constructor for Autonomous class
Autonomous::Autonomous(Drive *drive, Lift *lift, Manipulator *man) {
  autoDrive = drive;
  autoLift = lift;
  autoMan = man;
  timer = new Timer();
  timer->Start();
}

// Set whether to do far switch or not
void Autonomous::SetFarMode(int choice) {
  if(choice == 0) {
    //Disable Far Mode
    FarModeEnabled = false;
    StayHome = false;
  }
  if(choice == 1) {
    //Enable Far Mode
    FarModeEnabled = true;
    StayHome = false;
  }
  if(choice == 2) {
    //Don't cross field and stay close
    FarModeEnabled = false;
    StayHome = true;

  }
}

// void Autonomous::SetGameData(std::string data) {
//   if(data == "Auto") {
//     overrideData = false;
//     manualData = DriverStation::GetInstance().GetGameSpecificMessage();
//   } else {
//     manualData = data;
//     overrideData = true;
//   }
//}

// Set starting position and auto Mode
void Autonomous::SetStageOne(int mode, int startingPosition, int wait) {
  currentStage = 0;
  autoState = 0;
  waitStart = wait;
  //if(!overrideData) {
    gameData = DriverStation::GetInstance().GetGameSpecificMessage();
  //}
  if(gameData.length() > 0) {
  switch(mode) {
    case 0: //Baseline
      stage1 = [=](){return this->Baseline();};
      break;

    case 1: //Switch
      if(gameData[0] == 'L') {
        if(startingPosition == 1) stage1 = [=](){return this->Sw1L();};
        else if(startingPosition == 2) stage1 = [=](){return this->Sw2L();};
        else {
          if(FarModeEnabled) stage1 = [=](){return this->Sw3L();};
          //else if(gameData[1] == 'R' && StayHome == false) stage1 = [=](){return this->Sc3R();};
          else stage1 = [=](){return this->Baseline();};
        }
      } else {
        if(startingPosition == 1) {
          if(FarModeEnabled) stage1 = [=](){return this->Sw1R();};
          //else if(gameData[1] == 'L' && StayHome == false) stage1 = [=](){return this->Sc1L();};
          else stage1 = [=](){return this->Baseline();};
        }
        else if(startingPosition == 2) stage1 = [=](){return this->Sw2R();};
        else stage1 = [=](){return this->Sw3R();};
      }
      break;

    case 2: //Scale
      if(gameData[1] == 'L') {
        if(startingPosition == 1) stage1 = [=](){return this->Sc1L();};
        else if(startingPosition == 2) {
          if(gameData[0] == 'L') stage1 = [=](){return this->Sw2L();};
          else stage1 = [=](){return this->Sw2R();};
        }
        else {
          if(FarModeEnabled) stage1 = [=](){return this->Sc3L();};
          else if(gameData[0] == 'R') stage1 = [=](){return this->Sw3R();};
          else stage1 = [=](){return this->Baseline();};
        }
      } else {
        if(startingPosition == 1) {
          if(FarModeEnabled) stage1 = [=](){return this->Sc1R();};
          if(gameData[0] == 'L') stage1 = [=](){return this->Sw1L();};
          else stage1 = [=](){return this->Baseline();};
        }
        else if(startingPosition == 2) {
          if(gameData[0] == 'L') stage1 = [=](){return this->Sw2L();};
          else stage1 = [=](){return this->Sw2R();};
        }
        else stage1 = [=](){return this->Sc3R();};
      }
      break;

    case 3:
      stage1 = [=](){return this->Stop();};
      break;
  }
  }
}

void Autonomous::SetStageTwo(int mode) {
  switch(mode) {

  }
}

void Autonomous::SetStageThree(int mode) {
  switch(mode) {

  }
}

// Choose the best autonomous routine
void Autonomous::ChooseStage() {
  autoState = 0; waiting = false;
  switch(currentStage) {
      case 0:
        autoFunction = [=](){return this->BackDrive();};
        break;
      case 1:
        autoFunction = stage1;
        break;
      case 2:
        autoFunction = stage2;
        break;
      case 3:
        autoFunction = stage3;
        break;
  }
}

void Autonomous::RunPeriodic() {
  gameData = DriverStation::GetInstance().GetGameSpecificMessage(); //Get specific match data
  // gameData will be an array with 3 characters, eg. "LRL"
  // check https://wpilib.screenstepslive.com/s/currentCS/m/getting_started/l/826278-2018-game-data-details
  SmartDashboard::PutString("switch", &gameData[0]);
  if(gameData[0] == 'L') SmartDashboard::PutBoolean("switchBool", true);
  else SmartDashboard::PutBoolean("switchBool", false);
  if(gameData[0] == 'L') SmartDashboard::PutBoolean("scaleBool", true);
  else SmartDashboard::PutBoolean("scaleBool", false);
  SmartDashboard::PutString("scale", &gameData[1]);
  SmartDashboard::PutNumber("autoState", autoState);
  SmartDashboard::PutNumber("currentStage", currentStage);
  if(autoFunction()) {
    currentStage++;
    ChooseStage();
  }
  autoDrive->RunPeriodic();
  autoMan->RunPeriodic();
  autoLift->RunPeriodic();
}

bool Autonomous::Stop() {
  autoDrive->Stop();
  return true;
}

bool Autonomous::Wait(int delay) {
  if(waiting) {
    if(timer->Get() > delay) {
        waiting = false;
        return true;
    }
  } else {
    timer->Reset();
    waiting = true;
  }
  return false;
}

bool Autonomous::BackDrive() {
  switch (autoState) {
    case 0:
      autoMan->SetIntakeSpeed(-1);
      autoMan->Restrain();
      autoDrive->TankDrive(-0.5, -0.5);
      if(Wait(0.3)) autoState++;
      break;
    case 1:
      autoDrive->TankDrive(0, 0);
      if(Wait(0.3)) autoState++;
      break;
    case 2:
      autoDrive->TankDrive(0, 0);
      autoMan->SetIntakeSpeed(0);
      autoLift->SetMidPosition();
      autoState++;
      break;
    case 3:
      if(Wait(waitStart+0.5)) autoState++;
      break;
    default:
      Stop();
      return true;
    }
    return false;
}

bool Autonomous::Baseline() {
  switch (autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 3.4, 5)) autoState++;
      break;
    // case 1:
    //   if(autoDrive->EncoderTurn(1, 90)) autoState++;
    //   break;
    default:
      Stop();
      return true;
  }
  return false;
}

// Routine: Initial (1) > Switch (left)
bool Autonomous::Sw1L() {
  switch (autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 3.7)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.8, 90, 0.8)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 0.4, 0.5)) autoState++;
      break;
    case 3:
      //autoMan->Release();
      autoMan->SetIntakeSpeed(0.8);
      if(Wait(1)) autoState++;
      break;
    case 4:
      //autoMan->Restrain();
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.5)) autoState++;
      break;
    case 5:
      autoLift->SetLowPosition();
      if(Wait(0.3)) {
        autoState++;
        if(StayHome == true) {
          autoState = 20;
        }
      }
      break;
    case 6:
      if(autoDrive->EncoderTurn(0.7, -90, 1.0)) autoState++;
      break;
    case 7:
      if(autoDrive->DriveDistance(1, 1)) autoState++;
      break;
    case 8:
      if(autoDrive->EncoderTurn(0.65, 100, 1.2)) autoState++;
      break;
    case 9:
      autoMan->Release();
      if(autoDrive->DriveDistance(1, 0.8, 0.4)) autoState++;
      break;
    case 10:
      autoMan->Restrain();
      if(Wait(0.6)) autoState++;
      break;
    case 11:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.5)) autoState++;
      break;
    case 12:
      autoLift->SetMidPosition();
      autoState++;
      break;
    case 13:
      if(autoDrive->EncoderTurn(1, 20, 0.3)) autoState++;
      break;
    case 14:
      autoMan->SetIntakeSpeed(1);
      if(autoDrive->DriveDistance(1, 0.2)) autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

// Routine: Initial (2) > Switch (left)
bool Autonomous::Sw2L() {
  SmartDashboard::PutBoolean("one sw2", true);
  switch(autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 0.25)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(1, -50, 1)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 1.9)) autoState++; // USED TO BE 2.0m
      break;
    case 3:
      if(autoDrive->EncoderTurn(1, 55, 1)) autoState++;
      break;
    case 4:
      if(autoDrive->DriveDistance(1, 1, 1.5)) autoState++;
      break;
    case 5:
      autoMan->SetIntakeSpeed(1);
      if(Wait(0.8)) autoState++;
      break;
    case 6:
      if(autoDrive->DriveDistance(1, -0.30)) autoState++;
      break;
    case 7:
      autoMan->SetIntakeSpeed(0);
      autoLift->SetLowPosition();
      autoState++;
      break;
    case 8:
      if(autoDrive->EncoderTurn(0.6, 80, 1.5)) autoState++;
      break;
    case 9:
      autoMan->Release();
      if(autoDrive->DriveDistance(1, 0.8, 1)) autoState++;
      break;
    case 10:
      autoMan->Restrain();
      if(Wait(0.7)) autoState++;
      break;
    case 11:
      if(autoDrive->DriveDistance(1, -0.75)) autoState++;
      break;
    case 12:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->EncoderTurn(0.6, -80, 1.5)) autoState++;
      break;
    case 13:
      autoLift->SetMidPosition();
      autoState++;
      break;
    case 14:
      if(Wait(0.3)) autoState++;
      break;
    case 15:
      if(autoDrive->DriveDistance(1, 0.5, 0.6)) autoState++;
      break;
    case 16:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    case 17:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.5)) autoState++;
      break;
    case 18:
      autoLift->SetLowPosition();
      autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

// Routine: Initial (3) > Switch (left)
bool Autonomous::Sw3L() {
  switch(autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 5.5)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.8, -90, 1)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 4.4)) autoState++;
      break;
    case 3:
      if(autoDrive->EncoderTurn(0.8, -90, 1)) autoState++;
      break;
    case 4:
      if(autoDrive->DriveDistance(1, 0.2, 0.2)) autoState++;
      break;
    case 5:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

// Routine: Initial (1) > Switch (right)
bool Autonomous::Sw1R() {
  switch(autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 5.5)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.8, 90, 1)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 4.4)) autoState++;
      break;
    case 3:
      if(autoDrive->EncoderTurn(0.8, 90, 1)) autoState++;
      break;
    case 4:
      if(autoDrive->DriveDistance(1, 0.2, 0.2)) autoState++;
      break;
    case 5:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

// Routine: Initial (2) > Switch (right)
bool Autonomous::Sw2R() {
  switch(autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 0.5)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(1, 50, 1)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 1.9, 1.5)) autoState++;
      break;
    case 3:
      if(autoDrive->EncoderTurn(1, -40, 1)) autoState++;
      break;
    case 4:
      if(autoDrive->DriveDistance(1, 0.8, 1)) autoState++;
      break;
    case 5:
      autoMan->SetIntakeSpeed(1);
      if(Wait(0.8)) autoState++;
      break;
    case 6:
      if(autoDrive->DriveDistance(1, -0.30)) autoState++;
      break;
    case 7:
      autoMan->SetIntakeSpeed(0);
      autoLift->SetLowPosition();
      autoState++;
      break;
    case 8:
      if(autoDrive->EncoderTurn(0.6, -70, 1.5)) autoState++;
      break;
    case 9:
      autoMan->Release();
      if(autoDrive->DriveDistance(1, 0.75, 1)) autoState++;
      break;
    case 10:
      autoMan->Restrain();
      if(Wait(0.7)) autoState++;
      break;
    case 11:
      if(autoDrive->DriveDistance(1, -0.75)) autoState++;
      break;
    case 12:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->EncoderTurn(0.6, 82, 1.5)) autoState++;
      break;
    case 13:
      autoLift->SetMidPosition();
      autoState++;
      break;
    case 14:
      if(Wait(0.3)) autoState++;
      break;
    case 15:
      if(autoDrive->DriveDistance(1, 0.5, 0.6)) autoState++;
      break;
    case 16:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    case 17:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.5)) autoState++;
      break;
    case 18:
      autoLift->SetLowPosition();
      autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

// Routine: Initial (3) > Switch (right)
bool Autonomous::Sw3R() {
  switch (autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 3.7)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.8, -90, 0.8)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 0.4, 0.5)) autoState++;
      break;
    case 3:
      //autoMan->Release();
      autoMan->SetIntakeSpeed(0.8);
      if(Wait(1)) autoState++;
      break;
    case 4:
      //autoMan->Restrain();
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.5)) autoState++;
      break;
    case 5:
      autoLift->SetLowPosition();
      if(Wait(0.3)) {
        autoState++;
        if(StayHome == true) {
          autoState = 20;
        }
      }
      break;
    case 6:
      if(autoDrive->EncoderTurn(0.7, 90, 1.0)) autoState++;
      break;
    case 7:
      if(autoDrive->DriveDistance(1, 1)) autoState++;
      break;
    case 8:
      if(autoDrive->EncoderTurn(0.65, -100, 1.2)) autoState++;
      break;
    case 9:
      autoMan->Release();
      if(autoDrive->DriveDistance(1, 0.8, 0.4)) autoState++;
      break;
    case 10:
      autoMan->Restrain();
      if(Wait(0.6)) autoState++;
      break;
    case 11:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.5)) autoState++;
      break;
    case 12:
      autoLift->SetMidPosition();
      autoState++;
      break;
    case 13:
      if(autoDrive->EncoderTurn(1, -20, 0.3)) autoState++;
      break;
    case 14:
      autoMan->SetIntakeSpeed(1);
      if(autoDrive->DriveDistance(1, 0.2)) autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

bool Autonomous::Sc1L() {
  switch (autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 6.0)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.65, 55, 1.7)) autoState++;
      break;
    case 2:
      autoLift->SetHighPosition();
      autoState++;
      break;
    case 3:
      if(Wait(1.5)) autoState++;
      break;
    case 4:
      if(autoDrive->DriveDistance(0.4, 0.5, 1.2)) autoState++;
      break;
    case 5:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    case 6:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(0.35, -0.5)) autoState++;
      break;
    case 7:
      autoLift->SetLowPosition();
      autoState++;
      break;
    case 8:
      if(Wait(1)) autoState++;
      break;
    case 9:
      if(autoDrive->EncoderTurn(0.7, 100, 1.5)) autoState++;
      break;
    case 10:
      autoMan->Release();
      if(autoDrive->DriveDistance(1, 1.8, 1.5)) autoState++;
      break;
    case 11:
      autoMan->Restrain();
      if(Wait(0.3)) autoState++;
      break;
    case 12:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.3)) autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

bool Autonomous::Sc2L() {
  switch (autoState) {
    case 0:
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

bool Autonomous::Sc3L() {
  switch (autoState) {
    case 0:
      if(autoDrive->DriveDistance(1, 6.0)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.8, -90, 1.2)) autoState++;
      break;
    case 2:
      if(autoDrive->DriveDistance(1, 5.0)) autoState++;
      break;
    case 3:
      if(autoDrive->EncoderTurn(0.8, 130, 2)) autoState++;
      break;
    case 4:
      autoLift->SetHighPosition();
      autoState++;
      break;
    case 5:
      if(Wait(1.3)) autoState++;
      break;
    case 6:
      if(autoDrive->DriveDistance(0.4, 0.5, 1)) autoState++;
      break;
    case 7:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    case 8:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(0.4, -0.5, 1)) autoState++;
      break;
    case 9:
      autoLift->SetLowPosition();
      autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

bool Autonomous::Sc1R() {
  switch (autoState) {
    case 0:
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

bool Autonomous::Sc2R() {
  switch (autoState) {
    case 0:
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

bool Autonomous::Sc3R() {
  switch (autoState) {
    case 0:
      if(autoDrive->DriveDistance(0.9, 6.0)) autoState++;
      break;
    case 1:
      if(autoDrive->EncoderTurn(0.7, -35, 1)) autoState++;
      break;
    case 2:
      autoLift->SetHighPosition();
      autoState++;
      break;
    case 3:
      if(Wait(1.5)) autoState++;
      break;
    case 4:
      if(autoDrive->DriveDistance(0.4, 0.5, 1.2)) autoState++;
      break;
    case 5:
      autoMan->SetIntakeSpeed(1);
      if(Wait(1)) autoState++;
      break;
    case 6:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(0.35, -0.5)) autoState++;
      break;
    case 7:
      autoLift->SetLowPosition();
      autoState++;
      break;
    case 8:
      if(Wait(1)) autoState++;
      break;
    case 9:
      if(autoDrive->EncoderTurn(0.7, -80, 1.5)) autoState++;
      break;
    case 10:
      autoMan->Release();
      if(autoDrive->DriveDistance(1, 1.5, 1.5)) autoState++;
      break;
    case 11:
      autoMan->Restrain();
      if(Wait(0.3)) autoState++;
      break;
    case 12:
      autoMan->SetIntakeSpeed(0);
      if(autoDrive->DriveDistance(1, -0.3)) autoState++;
      break;
    default:
      Stop();
      return true;
  }
  return false;
}

// case 0:                //Scale from side
//   if(autoDrive->DriveDistance(1, 8)) autoState++;
//   break;
// case 1:
//   if(autoDrive->EncoderTurn(0.8, -90, 1.5)) autoState++;
//   break;
// case 2:
//   if(autoDrive->DriveDistance(1, -0.5, 1)) autoState++;
//   break;
// case 3:
//   autoLift->SetHighPosition();
//   if(Wait(1)) autoState++;
//   break;
// case 4:
//   if(autoDrive->DriveDistance(0.5, 0.6)) autoState++;
//   break;
// case 5:
//   autoMan->SetIntakeSpeed(1);
//   if(Wait(0.5)) autoState++;
//   break;
// case 6:
//   if(autoDrive->DriveDistance(0.5, -1, 1)) autoState++;
//   break;
// case 7:
//   autoLift->SetLowPosition();
//   autoState++;
//   break;
