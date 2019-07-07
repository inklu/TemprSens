#include <LCDMenuController.h>
#include <DTime.h>
#include <TemprSens.h>
// Data wire is plugged into port 7 on the Arduino
#define ONE_WIRE_BUS 7
#define TEMPERATURE_PRECISION 10
//Cycle display devices&clock on LCD
#define MLS_CHANGE_CYCLE 3000
//Degrees of Celsius sign
#define LCD_TXT_TEMPC String((char)223)+"C"
//Display on\off time for blinking
#define LCD_BLINK_MLS 500

//derive class to overload create method 
class MyMenu:public MenuController::Menu {
  public:
    //MyMenu(){ }
    void create();
};

//global date&time for clock
DTime dtime(2019,05,27,10,20,30);
unsigned long cycle,change,mls;
byte id=0;

//void resetAlarms();
void resetAvgTemp();

//global Temperature Sensors
TemprSens sensors(ONE_WIRE_BUS);

//create menu
void MyMenu::create() {
  MenuLine::MenuLeaf::MenuLeaf_time *mlt;  //time
  MenuLine::MenuLeaf::MenuLeaf_date *mld;  //date
  MenuLine::MenuLeaf::MenuLeaf_str *mls;  //string
  MenuLine::MenuLeaf::MenuLeaf_num<int8_t> *mlf1,*mlf2;  //float
  //MenuLine::MenuLeaf::MenuLeaf_bool *mlb;  //bool
  MenuLine::MenuLeaf::MenuLeaf_func *mlc;  //function call
  MenuLine::MenuNode *mn; //node
  MenuLine *ml;  //any line

  pFirstLine = mlt = new MenuLine::MenuLeaf::MenuLeaf_time;//newMenuLine(mtTime);  //create leaf of time
  mlt->name = "Time";     //leaf name
  mlt->setValue(&dtime);    //set value as a pointer to global variable
  mn->pFirstChild = mlt;    //the first line of the node
  mlt->pParentLine = mn;    //parent is node
  
  mld = new MenuLine::MenuLeaf::MenuLeaf_date;//newMenuLine(mtDate);      //create leaf of date
  mld->name = "Date";         //leaf name
  mld->setValue(&dtime);        //set value as a pointer to global variable
  mld->pParentLine = mn;        //parent is node
  mn->pFirstChild->pNextLine = mld;   //follows the time line
  mld->pPreviousLine = mn->pFirstChild; //predecessor - time line

  ml = mld;
  for(byte i = 0;i<sensors.getDeviceCount();i++){
    //one wire sensor
    mn = new MenuLine::MenuNode;
    mn->name = sensors[i].name;
    ml->pNextLine = mn;
    mn->pPreviousLine = ml;
    //entrails of one wire sensor: devices
    //changeable device name
    mls = new MenuLine::MenuLeaf::MenuLeaf_str;
    mls->name = "Name";
    mls->setValue(&sensors[i].name);
    mn->pFirstChild = mls;
    mls->pParentLine = mn;
///*
    //set minimum temperature for alarm
    mlf1 = new MenuLine::MenuLeaf::MenuLeaf_num<int8_t>();
    mlf1->name = "Alarm Min Tempr";
    mlf1->setValue(0,1,-55,125,&sensors[i].minTempC); 
    mlf1->pPreviousLine = mls;
    mls->pNextLine = mlf1;
    mlf1->pParentLine = mn;
//*/
///*
    //set maximum temperature for alarm
    mlf2 = new MenuLine::MenuLeaf::MenuLeaf_num<int8_t>();
    mlf2->name = "Alarm Max Tempr";
    mlf2->setValue(0,1,-55,125,&sensors[i].maxTempC); 
    mlf2->pPreviousLine = mlf1;
    mlf1->pNextLine = mlf2;
    mlf2->pParentLine = mn;
//*/
    //next temperature device
    ml = mn;
  }
///*
  //reset the average temperatures
  mlc = new MenuLine::MenuLeaf::MenuLeaf_func();
  mlc->name = "Reset Avg Temp";
  mlc->setValue(resetAvgTemp);
  mlc->pPreviousLine = mn;
  mn->pNextLine = mlc;
//*/
  pActiveLine = pFirstLine; //active line is the first line
}

MyMenu m; //define menu object
byte clkPin=9,dtPin=10,swPin=8; //rotary encoder parms
MenuEncoderController mec(clkPin,dtPin,swPin);//define encoder controller

//LCD parms
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const LiquidCrystal &lcd = m.newDisplay(rs,en,d4,d5,d6,d7,16,2); //create lcd for menu

////blinking display
bool swBlinkLCD = false;
unsigned long mlsBlinkLCD;

void blinkSwitchLCD(const bool sw){ 
  swBlinkLCD = sw; 
  if(!swBlinkLCD) lcd.display(); 
}

void blinkLCD(const unsigned long _mls=0){
  static bool blk = false;
  static unsigned long mlsBlk;
  unsigned long mls;
  if(swBlinkLCD){
    if(!_mls) mls = millis();
    else mls = _mls;
    if(mlsBlk<mls){
      if(blk) lcd.display();
      else lcd.noDisplay();
      mlsBlk = mls + LCD_BLINK_MLS;
      blk = !blk;
    }
  }
}

void resetAvgTemp(){
  for(byte i=0;i<sensors.getDeviceCount();i++) {
    sensors[i].resetAvgTemp();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Average Temp");
  lcd.setCursor(0,1);
  lcd.print("are reseted");
}

void setup() {
  // put your setup code here, to run once:
/*
  Serial.begin(9600);
  Serial.println("Start");
*/
  sensors.begin();
  
  m.create();
  mec.bindMenu(m);// bind menu to controller

  cycle = change = (uint64_t)millis() + 1000;
}

void loop() {
  // put your main code here, to run repeatedly:
  mls = millis();
  
  mec.run(mls); //holding OK enables menu, holding Back disables
  
  //if (!m.isActive()) { lcd... } //if menu is disabled the lcd can be used for other purposes
  if (mls >= cycle) {
    cycle += 1000;
    dtime.tick(); //+1sec to clock
//display date&time when menu is disabled 
    if (!m.isActive()){
      if(id==sensors.getDeviceCount()) {
        blinkSwitchLCD(false);        
        lcd.clear();
        lcd.noCursor();
        lcd.noBlink();
        lcd.setCursor(0,0);
        lcd.print(decimate(dtime.hour)+":"+decimate(dtime.minute)+":"+decimate(dtime.second));
        lcd.setCursor(0,1);
        lcd.print((String) dtime.year+"-"+decimate(dtime.month)+"-"+decimate(dtime.day));
      }
//display tempr sensors info     
      else {
        DeviceAddress da;
        sensors.getAddress(da,id);
        if(sensors.hasAlarm(da)) blinkSwitchLCD(true);
        else blinkSwitchLCD(false);
        lcd.clear();
        lcd.noCursor();
        //current temp
        lcd.setCursor(0,0);
        lcd.print(sensors[id].name);
        lcd.setCursor(0,1);
        lcd.print(sensors[id].getTempC(),2);
        lcd.print(LCD_TXT_TEMPC);
        //avg temp
        lcd.setCursor(9,0);
        lcd.print("Avg");
        lcd.setCursor(9,1);
        lcd.print(sensors[id].getAvgTempC(),2);
        lcd.print(LCD_TXT_TEMPC);
      }
      sensors.processAlarms();
    } 
  }
//blink LCD
  if(!m.isActive()) blinkLCD();  
//cycle on display
  if(mls >= change){
    change += MLS_CHANGE_CYCLE;
    if(++id > sensors.getDeviceCount()) id = 0;
  }
}
