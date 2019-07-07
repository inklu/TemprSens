#include <TemprSens.h>

//// TemprSensDevice
byte TemprSensDevice::sid=0;

TemprSensDevice::TemprSensDevice(){
  initialize();
}

TemprSensDevice::TemprSensDevice(DallasTemperature *_dt,OneWire *_ow):dt(_dt),ow(_ow){
  initialize();
  initAlarmTemp();
}

void TemprSensDevice::initialize(){
  id = sid++;
  //_name = "Sensor"+String(id);
  name = "Sensor"+String(id);
}

void TemprSensDevice::initAlarmTemp(){
  DeviceAddress addr;
  dt->getAddress(addr,id);
  dt->setLowAlarmTemp(addr,minTempC);
  dt->setHighAlarmTemp(addr,maxTempC);
}

void TemprSensDevice::bindSensor(DallasTemperature *_dt,OneWire *_ow){
  dt=_dt; 
  ow=_ow;
  initAlarmTemp();
}

float TemprSensDevice::getTempC(const uint64_t _mls=0){
  uint64_t mls;

  if(_mls==0) mls = millis();
  else mls = _mls;

  if(mls >= mlsCycle) { 
    requestTempC();
    mlsCycle = mls + REQUEST_TEMP_EACH_MLS;
  }
  
  return tempC;
}

void TemprSensDevice::requestTempC(){
  DeviceAddress addr;
  dt->getAddress(addr,id);
  if(minTempC != dt->getLowAlarmTemp(addr)){
    dt->setLowAlarmTemp(addr,minTempC);
  }
  if(maxTempC != dt->getHighAlarmTemp(addr)){
    dt->setHighAlarmTemp(addr,maxTempC);
  }

  dt->requestTemperaturesByIndex(id);
  tempC = dt->getTempCByIndex(id);
  //average temp calc
  avgTempC = avgTempC ? ( avgTempC + tempC ) / 2 : tempC;
}

void TemprSensDevice::resetAvgTemp(){
  avgTempC = tempC;
}

//// TemprSens
TemprSensDevice& TemprSens::operator[](const byte i){
  if(i<getDeviceCount()) return devs[i];
}

TemprSensDevice& TemprSens::operator[](const DeviceAddress addr){
  for(byte i = 0; i < getDeviceCount();i++) {
    DeviceAddress da;
    getAddress(da,i);
    if(da==addr) return devs[i];
  }
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
/*
void TemprSens::processAlarms(){
  for(byte i = 0;i < getDeviceCount();i++){
    DeviceAddress addr;
    getAddress(addr,i);
    if(devs[i].minTempC != getLowAlarmTemp(addr)){
      setLowAlarmTemp(addr,devs[i].minTempC);
    }
    if(devs[i].maxTempC != getHighAlarmTemp(addr)){
      setHighAlarmTemp(addr,devs[i].maxTempC);
    }
  }
  DallasTemperature::processAlarms();
}
*/
