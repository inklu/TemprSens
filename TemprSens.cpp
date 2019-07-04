#include <TemprSens.h>

byte TemprSensDevice::sid=0;

TemprSensDevice::TemprSensDevice(DallasTemperature *_dt,OneWire *_ow):dt(_dt),ow(_ow){
  id = sid++;
  _name = "Sensor"+String(id);
}

float TemprSensDevice::getTempC(){
  dt->requestTemperaturesByIndex(id);
  return dt->getTempCByIndex(id);
}

TemprSens::TemprSens(const byte _pin): _ow(_pin) {
  setOneWire(&_ow);
}

void TemprSens::begin(){
  DallasTemperature::begin();
  if(devs!=nullptr) delete [] devs;
  byte cnt=getDeviceCount();
  devs = new TemprSensDevice[cnt];
  for(byte i = 0;i < cnt;i++){
    devs[i].bindSensor(this,&ow);
  }
}

TemprSensDevice& TemprSens::operator[](const byte i){
  if(i<getDeviceCount()) return devs[i];
}

