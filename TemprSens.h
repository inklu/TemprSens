#ifndef TEMPRSENS_H
#define TEMPRSENS_H

#ifndef DEBUG
#define DEBUG
#endif

#if ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

//#define ONE_WIRE_BUS 7
//#define TEMPERATURE_PRECISION 10
#define MLS_CHANGE_CYCLE 5000

class TemprSensDevice {
    static byte sid;
    String _name;
    byte id;
    OneWire *ow;
    DallasTemperature *dt;
  public:
    String &name = _name;
    TemprSensDevice(DallasTemperature *_dt,OneWire *_ow);
    TemprSensDevice(){ id = sid++; _name = "Sensor"+String(id); };
    void bindSensor(DallasTemperature *_dt,OneWire *_ow){ dt=_dt; ow=_ow; }
    const String getName(){ return _name; }
    void setName(const String _nm){ _name = _nm; }
    TemprSensDevice operator=(String _nm){ _name = _nm; }
    operator float(){ return getTempC(); }
    float getTempC();
};

class TemprSens: public DallasTemperature {
    OneWire _ow;
    TemprSensDevice *devs=nullptr;
  public:
    const OneWire &ow = _ow;
    TemprSens(const byte _pin);
    void begin();
    TemprSensDevice& operator[](const byte i);
};


#endif
