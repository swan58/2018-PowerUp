#include <iostream>

#include "WPILib.h"
#include "AutoBelevStarategy.h"
#include "..\ControlMap.h"

#include "..\Map.h"

using namespace frc;
using namespace std;

AutoBelevStarategy::AutoBelevStarategy(CurtinTalonSRX *_belev_motor, float _position) {
  belev_motor = _belev_motor;
  target = _position * Map::Robot::belev_ticks_per_metre;
}

void AutoBelevStarategy::start() {
  belev_motor->Set(0);
  belev_motor->SetControlMode(CurtinTalonSRX::ControlMode::Position);
  belev_motor->configure_pidf(0.01, 0.001, 0.0, 0.0);
  belev_motor->Set(target);
}

void AutoBelevStarategy::tick(double time) {
  if (abs(belev_motor->GetEncoder() - target) < 10) this->done = true; // 10 is temp const
}

void AutoBelevStarategy::stop() {
  belev_motor->SetControlMode(CurtinTalonSRX::ControlMode::PercentOutput);
  belev_motor->configure_pidf(0.0, 0.0, 0.0, 0.0);
  belev_motor->Set(0);
}
