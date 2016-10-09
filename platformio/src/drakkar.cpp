#include "drakkar.h"
#include "config.h"


Drakkar::Drakkar(int potentiometer_pin, int currentSensor_pin, int endstop_pin,
                 int up_pin, int down_pin, int speed_pin,
                 int EMGFront_pin, int EMGBack_pin): pid(1,0,0,0,1024){
  this->potentiometer_pin = potentiometer_pin;
  this->currentSensor_pin = currentSensor_pin;
  this->endstop_pin = endstop_pin;
  this->up_pin = up_pin;
  this->down_pin = down_pin;
  this->speed_pin = speed_pin;
  this->EMGFront_pin = EMGFront_pin;
  this->EMGBack_pin = EMGBack_pin;
  this->pid.Initialize();
  this->lastTime = 0;
  this->endstop_status = digitalRead(this->endstop_pin);
  this->output = 0;
}

void Drakkar::debug(){
  Serial.print(analogRead(this->potentiometer_pin));
  Serial.print(" , ");
  Serial.print(digitalRead(this->endstop_pin));
  Serial.print(" , ");
  Serial.print(analogRead(this->EMGFront_pin));
  Serial.print(" , ");
  Serial.print(analogRead(this->EMGBack_pin));
  this->output = this->pid.Compute(500, analogRead(this->potentiometer_pin));
  Serial.print(" , ");
  Serial.println(this->output);
}

int Drakkar::run(){
  unsigned long now = millis();
  int position = (analogRead(this->potentiometer_pin)/AnalogResolution) * PotentiometerLong;
  EMGInfo emg_info = this->readEMG();
  double new_position = (double)position +((double)(MaxSpeed*emg_info.speed) * (double)(now-lastTime));
  this->output = this->pid.Compute(new_position, position/AnalogResolution)*AnalogResolution;
  this->writeMotor();
  this->lastTime = now;
  return 0;
}

void Drakkar::writeMotor(){
  this->endstop_status = digitalRead(this->endstop_pin);
  digitalWrite(this->up_pin, (int)(this->output > 0));
  digitalWrite(this->down_pin, (int)(this->output < 0));
  if(this->output > 0) this->output = this->output * this->endstop_status;
  analogWrite(this->speed_pin, abs(this->output));
}

void Drakkar::endstop(){
  this->writeMotor();
}

EMGInfo Drakkar::readEMG(){
  int quadriceps = analogRead(this->EMGFront_pin);
  int hamstring = analogRead(this->EMGFront_pin);

  float speed = (quadriceps - hamstring) / AnalogResolution;

  float power = 0.0;
  if((quadriceps > EMGMin) or (hamstring > EMGMin)){
    if (speed >= 0.0) power = quadriceps/AnalogResolution;
    else power = hamstring/AnalogResolution;
  }

  EMGInfo info = {power, speed};
  return info;
}
