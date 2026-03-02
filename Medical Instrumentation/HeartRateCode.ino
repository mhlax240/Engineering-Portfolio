#include <DFRobot_Heartrate.h>

int hrPin = A6;

DFRobot_Heartrate heartrate(DIGITAL_MODE);

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  uint8_t rateValue;
  heartrate.getValue(hrPin);
  rateValue = heartrate.getRate();
  if (rateValue)
  {
    Serial.println(rateValue);
  }
  delay(500);
}
