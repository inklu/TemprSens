#include <LCDMenuController.h>
#include <DTime.h>
#include <TemprSens.h>
// Include the libraries we need
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 7
#define TEMPERATURE_PRECISION 10

//derive class to overload create method 
class MyMenu:public MenuController::Menu {
  public:
    //MyMenu(){ }
    void create();
};

//global date&time for clock
DTime dtime(2019,05,27,10,20,30);
uint64_t cycle,change;
byte id=0;

//global Temperature Sensors
TemprSens sensors(ONE_WIRE_BUS);

//create menu
void MyMenu::create() {
  MenuLine::MenuLeaf::MenuLeaf_time *mlt;  //time
  MenuLine::MenuLeaf::MenuLeaf_date *mld;  //date
  MenuLine::MenuLeaf::MenuLeaf_str *mls;  //string
  MenuLine::MenuNode *mn; //node
  MenuLine *ml;  //any line

  pFirstLine = mlt = newMenuLine(mtTime);  //create leaf of time
  mlt->name = "Time";     //leaf name
  mlt->setValue(&dtime);    //set value as a pointer to global variable
  mn->pFirstChild = mlt;    //the first line of the node
  mlt->pParentLine = mn;    //parent is node
  
  mld = newMenuLine(mtDate);      //create leaf of date
  mld->name = "Date";         //leaf name
  mld->setValue(&dtime);        //set value as a pointer to global variable
  mld->pParentLine = mn;        //parent is node
  mn->pFirstChild->pNextLine = mld;   //follows the time line
  mld->pPreviousLine = mn->pFirstChild; //predecessor - time line

  ml = mld;
  for(byte i = 0;i<sensors.getDeviceCount();i++){
    //датчик
    mn = newMenuLine(mtNode);
    mn->name = sensors[i].name;
    ml->pNextLine = mn;
    mn->pPreviousLine = ml;
    //внутренности датчика
    mls = newMenuLine(mtString);
    mls->name = "Name";
    mls->setValue(&sensors[i].name);
    mn->pFirstChild = mls;
    mls->pParentLine = mn;
    //следующий датчик
    ml = mn;
  }
  pActiveLine = pFirstLine; //active line is the first line
}

MyMenu m; //define menu object
byte clkPin=9,dtPin=10,swPin=8; //rotary encoder parms
MenuEncoderController mec(clkPin,dtPin,swPin);//define encoder controller

//LCD parms
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const LiquidCrystal &lcd = m.newDisplay(rs,en,d4,d5,d6,d7,16,2); //create lcd for menu

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Start");

  sensors.begin();
  m.create();
  mec.bindMenu(m);// bind menu to controller

  cycle = change = (uint64_t)millis() + 1000;

}

void loop() {
  // put your main code here, to run repeatedly:
  mec.run(); //holding OK enables menu, holding Back disables
  
  //if (!m.isActive()) { lcd... } //if menu is disabled the lcd can be used for other purposes
  uint64_t mls = millis();
  
  if (mls >= cycle) {
    cycle += 1000;
    dtime.tick(); //+1sec to clock
//display date&time when menu is disabled 
    if (!m.isActive() && id==sensors.getDeviceCount()){
        lcd.clear();
        lcd.noCursor();
        lcd.setCursor(0,0);
        lcd.print(decimate(dtime.hour)+":"+decimate(dtime.minute)+":"+decimate(dtime.second));
        lcd.setCursor(0,1);
        lcd.print((String) dtime.year+"-"+decimate(dtime.month)+"-"+decimate(dtime.day));
    } 
  }
//display temperature from each sensors
  if(mls >= change){
    change += MLS_CHANGE_CYCLE;
    if(!m.isActive() && id<sensors.getDeviceCount()){
        lcd.clear();
        lcd.noCursor();
        lcd.setCursor(0,0);
        lcd.print(sensors[id].name);
        lcd.setCursor(0,1);
        lcd.print(sensors[id].getTempC(),DEC);
        if(++id > sensors.getDeviceCount()) id = 0;
    }
  }
}
