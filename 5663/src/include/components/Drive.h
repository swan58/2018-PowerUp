#pragma once

#include <ctre/Phoenix.h>
#include <AHRS.h>
#include <PIDOutput.h>

class gyroPID : public PIDOutput {
  private:
    double thisisoutput = 0;

  public:
    void PIDWrite(double output);
    double GetOutput();
};

namespace components {
  class Drive {
    AHRS *imu;
    PIDController *turn;
    gyroPID *out;
    DoubleSolenoid *gearMode; //Solenoids for gears

    public:
      Drive(int l1, int l2, int l3, int r1, int r2, int r3); // Constructor
      void TankDrive(double left, double right, bool square=false);
      void Stop();
      bool TurnAngle(double speed, double angle);
      bool DriveDistance(double speed, double distance, double timeout=0);
      void SetSlowGear();
      void SetFastGear();
      void ToggleGear();
      void RunPeriodic();

    private:
      TalonSRX *left1, *left2, *left3, *right1, *right2, *right3;

      bool turning = false, driving = false; // State variables
      double turnTolerance = 2.0, driveTolerance = 40; // Tolerance variables
      double kP = 0.025, kI = 0.0004, kD = 0.04, kM = (80*26.041666667)/0.4787787204; // need to check kM
      int slowGear = gearMode->kForward;
      int fastGear = gearMode->kReverse;
      double deadzone = 0.015;
      bool currentGear = false;
      int leftFinalDistance= 0, rightFinalDistance = 0;
      double starting_time;
  };
}

//nt::NetworkTableInstance::GetDefault().GetTable("table")->GetEntry("turn").SetDouble(0);
//var = nt::NetworkTableInstance::GetDefault().GetTable("table")->GetEntry("turn").GetDouble(0);
// #include <networktables/NetworkTableInstance.h>
