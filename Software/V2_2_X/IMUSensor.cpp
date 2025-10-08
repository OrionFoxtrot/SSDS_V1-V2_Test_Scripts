#include "IMUSensor.h"

IMUSensor::IMUSensor() {}

bool IMUSensor::begin(uint8_t address)
{
  Wire.begin();
  if (!imu.begin(0x4A))
  {
    debug.println("IMU init failed!");
    return false;
  }
  myIMU.begin(address);
  Wire.setClock(400000);
  myIMU.enableLinearAccelerometer(50);
  return true;
}

bool IMUSensor::available()
{
  return imu.dataAvailable();
}

String IMUSensor::readData()
{
  String str = "";

  x = myIMU.getLinAccelX();
  y = myIMU.getLinAccelY();
  z = myIMU.getLinAccelZ();
  linAccuracy = myIMU.getLinAccelAccuracy();

  str = str + String(x) + ',' + String(y) + ',' + String(z);
  return (str);
}
