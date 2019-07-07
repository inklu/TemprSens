#ifndef TEMPRSENS_H
#define TEMPRSENS_H

/*
#ifndef DEBUG
#define DEBUG
#endif
*/

#if ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#define REQUEST_TEMP_EACH_MLS 30000
//smoothing factor
#define COEF_K 0.01

class TemprSensDevice {
    static byte sid;    //device count actualy
    float tempC=0,         //temperature C
          avgTempC=0;
    /*
    String _name;       //changable device name for display
    int8_t _minTempC=-50,  //minimum temperature for alarm
           _maxTempC=100;  //maximum temperature for alarm
    */
    //bool _alarmActive=false; //alarm active indicator
    byte id;                 //device ID

    OneWire *ow;
    DallasTemperature *dt;

    inline void initialize();
    inline void initAlarmTemp();
    uint64_t mlsCycle=0;
    void requestTempC();
  public:
    /*
    String &name = _name; //references to name,temps that are needed for changes through menu
    int8_t &minTempC = _minTempC,
           &maxTempC = _maxTempC;
    */
    String name;     //changable device name for display
    int8_t minTempC=-55,  //minimum temperature for alarm
           maxTempC=125; //maximum temperature for alarm
    TemprSensDevice();
    TemprSensDevice(DallasTemperature *_dt,OneWire *_ow);
    void bindSensor(DallasTemperature *_dt,OneWire *_ow);
    //const String getName(){ return _name; }
    //void setName(const String _nm){ _name = _nm; }
    //TemprSensDevice operator=(String _nm){ _name = _nm; }
    //operator float(){ return getTempC(); }
    float getTempC(const uint64_t _mls=0);
    float getAvgTempC(){ return avgTempC; }
    void resetAvgTemp();
};

class TemprSens: public DallasTemperature {
    OneWire _ow;
    TemprSensDevice *devs;
  public:
    const OneWire &ow = _ow;
    TemprSens(const byte _pin);
    void begin();
    TemprSensDevice& operator[](const byte i);
    TemprSensDevice& operator[](const DeviceAddress addr);
};


#endif
