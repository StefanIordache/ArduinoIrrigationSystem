#include <LiquidCrystal.h>
#include <IRremote.h>
#include <EEPROM.h>

//Define Pins
#define PIN_LED_RED 10
#define PIN_LED_BLUE 2
#define PIN_IR_RECEIVER 11
#define LCD_RS 8
#define LCD_EN 9
#define LCD_DIGIT_4 4
#define LCD_DIGIT_5 5
#define LCD_DIGIT_6 6
#define LCD_DIGIT_7 7
#define PIN_LCD_LIGHT 3
#define PIN_PUMP_SWITCH 13
#define PIN_HUMIDTY_SENSOR_VALUE A1
#define PIN_HUMIDITY_SENSOR_VCC  12

//Define IR Codes
//0
#define IR_POWER_PUMP 0xFF6897
//CH
#define IR_LCD_DISPLAY_POWER 0xFF629D
//EQ
#define IR_CHANGE_MODE 0xFF906F
//+
#define IR_PLUS 0xFFA857
//-
#define IR_MINUS 0xFFE01F
//CH+
#define IR_SAVE_CONFIGURATION 0xFFE21D
//CH-
#define IR_CANCEL_CONFIGURATION 0xFFA25D
//1
#define IR_INFORMATIONS_1 0xFF30CF
//2
#define IR_INFORMATIONS_2 0xFF18E7
//3
#define IR_MENU_CRITICAL_LEVEL 0xFF7A85
//4
#define IR_MENU_PUMP_TIME 0xFF10EF

//LCD Defines
#define LCD_ON_LEVEL 255
#define LCD_OFF_LEVEL 0
#define IN_INFORMATIONS_1 1
#define IN_INFORMATIONS_2 2
#define IN_MENU_CRITICAL_LEVEL 3
#define IN_MENU_PUMP_TIME 4

IRrecv irrecv(PIN_IR_RECEIVER);
decode_results results;

LiquidCrystal lcd (LCD_RS, LCD_EN, LCD_DIGIT_4, LCD_DIGIT_5, LCD_DIGIT_6, LCD_DIGIT_7);

unsigned long currentArduinoTime;

//Configurations are used to save the local state of the machine and the memory state
//configuration = state inside the EEPROM
//localConfiguration = state outside the EEPROM. This state can be modified and saved in order to be secured if we disconnect or unplug the machine
const int configurationAddress = 0;

struct Configuration
{
  int criticalLevel;
  int pumpTime;
  bool mode;
} configuration, localConfiguration;

class HumiditySensor 
{
private:
  unsigned int humidity;

public:
  HumiditySensor()
  {
    this->humidity = 0;
  }
  
  unsigned int getHumidity()
  {
    return this->humidity;
  }

  void setHumidity()
  {
    digitalWrite(PIN_HUMIDITY_SENSOR_VCC, HIGH);
    Serial.println(analogRead(PIN_HUMIDTY_SENSOR_VALUE));
    this->humidity = map(analogRead(PIN_HUMIDTY_SENSOR_VALUE),0,1023,0,100);
    Serial.println(this->humidity);
    digitalWrite(PIN_HUMIDITY_SENSOR_VCC, LOW);
  }
  
} humiditySensor;


//Display class is used in order to control our irrigation system
//It contains 4 types of screens: 2 with informations about current state of system and 2 for changing variables (critical level of humidity and pump time)
class Display 
{
private:
  bool lcdIsOn;
  unsigned long lastLcdActionTime;
  bool initialSetupDone;
  //4 possible displays
  int screenState;

public:
  Display()
  {
    this->lcdIsOn = true;
    this->lastLcdActionTime = 0;
    this->initialSetupDone = false;
    this->screenState = IN_INFORMATIONS_1;
  }

  int getScreenState()
  {
    return this->screenState;
  }

  int setScreenState(int newScreenState)
  {
    this->screenState = newScreenState;
    return this->screenState;
  }

  bool getLcdIsOn()
  {
    return this->lcdIsOn;
  }

  bool getInitialSetupState()
  {
    return this->initialSetupDone;
  }

  unsigned long setLastLcdActionTime(unsigned long newTime)
  {
    this->lastLcdActionTime = newTime;
    return this->lastLcdActionTime;
  }

  void displayInformations1()
  {
    //EEPROM.get(configurationAddress, configuration);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Humidity: ");
    lcd.setCursor(10,0);
    lcd.print(humiditySensor.getHumidity());
    lcd.setCursor(0, 1);
    lcd.print("Mode: ");
    lcd.setCursor(6, 1);
    if (configuration.mode == true)
    {
      lcd.print("Auto");
    }
    else
    {
      lcd.print("Manual");
    }
  }

  void displayInformations2()
  {
    //EEPROM.get(configurationAddress, configuration);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Crit. level: ");
    lcd.setCursor(13,0);
    lcd.print(configuration.criticalLevel);
    lcd.setCursor(0,1);
    lcd.print("Pump time: ");
    lcd.setCursor(12,1);
    lcd.print(configuration.pumpTime/1000);
    lcd.setCursor(14,1);
    lcd.print("s");
  }

  void displayCriticalLevelMenu()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New Crit. Lvl.");
    lcd.setCursor(0,1);
    lcd.print(localConfiguration.criticalLevel);
  }

  void displayPumpTimeMenu()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New pump time");
    lcd.setCursor(0,1);
    lcd.print(localConfiguration.pumpTime/1000);
  }

  void turnOnLcd()
  {
    if (this->lcdIsOn == false)
    {
      analogWrite(PIN_LCD_LIGHT, LCD_ON_LEVEL);
      lastLcdActionTime = currentArduinoTime;

      displayInformations1();

      this->lcdIsOn = true;
    }
      analogWrite(PIN_LCD_LIGHT, LCD_ON_LEVEL);
    return;
  }

  void turnOffLcd()
  {
    if (this->lcdIsOn == true)
    {
      analogWrite(PIN_LCD_LIGHT, LCD_OFF_LEVEL);
      this->lcdIsOn = false;
    }
    return;
  }

  bool turnOffLcdIfNeeded()
  {
    if (this->lcdIsOn == true && currentArduinoTime - this->lastLcdActionTime >= 10000)
    {
      turnOffLcd();
      return true;
    }
    return false;
  }

  void initialSetup()
  {
    turnOnLcd();
    this->initialSetupDone = true;
    return;
  }
} display;


//Led class with methods for switching leds on and off
class Led 
{
private:
  int pin;
  bool on;

public:
  Led() 
  {
    this->on = false; 
  }
  
  void setPin(int pin)
  {
    this->pin = pin;
  }
  
  bool getState()
  {
    return this->on;
  }

  void turnOn()
  {
    digitalWrite(this->pin, HIGH);
    this->on = true;
    return;
  }
  
  void turnOff()
  {
    digitalWrite(this->pin, LOW);
    this->on = false;
    return;
  }
} ledRed, ledBlue;

//Pump class with methods for monitoring pump activity (last start and stop times + activity status)
class Pump
{
private:
  unsigned long lastStartTime;
  unsigned long lastStopTime;
  bool active;

public:
  Pump() 
  {
    this->active = false;
    this->lastStartTime = 0;
    this->lastStopTime = 0;
  }

  unsigned long setLastStopTime(unsigned long newLastStopTime)
  {
    this->lastStopTime = newLastStopTime;
    return this->lastStopTime;
  }

  bool getActiveState()
  {
    return this->active;
  }

  bool changeActiveState()
  {
    this->active = !this->active;
    return this->active;
  }

  unsigned int getLastStartTime()
  {
    return this->lastStartTime;
  }

  unsigned int setLastStartTime(unsigned int newLastStartTime)
  {
    this->lastStartTime = newLastStartTime;
    return this->lastStartTime;
  }

  unsigned int getLastStopTime()
  {
    return this->lastStopTime;
  }

  unsigned int setLastStopTime(unsigned int newLastStopTime)
  {
    this->lastStopTime = newLastStopTime;
    return this->lastStopTime;
  }

  void start()
  {
    this->lastStartTime = currentArduinoTime;
    digitalWrite(PIN_PUMP_SWITCH, HIGH);
    this->active = true;
    return;
  }

  void stop()
  {
    this->lastStopTime = currentArduinoTime;
    digitalWrite(PIN_PUMP_SWITCH, LOW);
    this->active = false;
    return;
  }

  bool checkIfPumpTimeIsOver()
  {
      if (currentArduinoTime - this->lastStartTime >= configuration.pumpTime)
        return true;
      else
        return false;
  }
} pump;

void setup() 
{
  //Functions used to save initial system parameters inside the memory
  /*for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }*/

  /*configuration.mode = false;
  configuration.pumpTime = 5000;
  configuration.criticalLevel = 50;
  EEPROM.put(configurationAddress, configuration);*/

  //Get configuration when machine starts
  EEPROM.get(configurationAddress, configuration);
  localConfiguration = configuration;
  
  pinMode(PIN_LCD_LIGHT, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_HUMIDITY_SENSOR_VCC, OUTPUT);

  digitalWrite(PIN_HUMIDITY_SENSOR_VCC, LOW);
  
  Serial.begin(9600);
  irrecv.enableIRIn(); 

  ledRed.setPin(PIN_LED_RED);
  ledBlue.setPin(PIN_LED_BLUE);

  lcd.begin(16,2);
  display.turnOffLcd();
}

void loop() 
{
  //Update arduino time
  currentArduinoTime = millis();

  //Turn off LCD only if we are not in configuration menu
  if (display.getLcdIsOn() && (display.getScreenState() == IN_INFORMATIONS_1 || display.getScreenState() == IN_INFORMATIONS_2))
  {
      display.turnOffLcdIfNeeded();
  }
  
  //Initial setup when plugging the arduino board
  if (display.getInitialSetupState() == false && currentArduinoTime > 1000)
  {
    display.initialSetup();
  }

  //Get humidity from soil each second.
  if (currentArduinoTime > 0 && currentArduinoTime % 1000 == 0)
  {
    humiditySensor.setHumidity();
    if (display.getScreenState() == IN_INFORMATIONS_1)
    {
      display.displayInformations1();
      display.setScreenState(IN_INFORMATIONS_1);
    }
  }

  menuControl();
  pumpControl();
}

void menuControl()
{
  //Decode IR Remote signals 
  if(irrecv.decode(&results)) 
  {
    Serial.println(results.value, HEX);
    
    int displayScreenState = display.getScreenState();
    
    //Update last action on the lcd screen
    display.setLastLcdActionTime(currentArduinoTime);
    
    //Turn on display if it's off and we press the on key
    if (display.getLcdIsOn() == false && results.value == IR_LCD_DISPLAY_POWER)
    {
      display.turnOnLcd();
    }
    //Power pump on button press if we the system is on manual mode
    else if (results.value == IR_POWER_PUMP && (displayScreenState == IN_INFORMATIONS_1 || displayScreenState == IN_INFORMATIONS_2) && !configuration.mode && !pump.getActiveState())
    {
      pump.start();
      ledBlue.turnOn();
      if(ledRed.getState())
      {
        ledRed.turnOff();
      }
    }
    //If Lcd display is on and we press on the remote
    else if (display.getLcdIsOn())
    {
      //Change pump mode
      if (results.value == IR_CHANGE_MODE && displayScreenState == IN_INFORMATIONS_1)
      {
        localConfiguration.mode = !localConfiguration.mode;
        configuration = localConfiguration;
        EEPROM.put(configurationAddress, configuration);
        display.displayInformations1();
        display.setScreenState(IN_INFORMATIONS_1);
        if(ledRed.getState())
        {
          ledRed.turnOff();
        }
        if (pump.getActiveState())
        {
          pump.stop();
          ledBlue.turnOff();
        }
        pump.setLastStopTime(currentArduinoTime);
      }
      //Display first information panel
      if (results.value == IR_INFORMATIONS_1 && displayScreenState == IN_INFORMATIONS_2)
      {
        display.displayInformations1();
        display.setScreenState(IN_INFORMATIONS_1);
      }
      //Display second informatio panel
      else if (results.value == IR_INFORMATIONS_2 && displayScreenState == IN_INFORMATIONS_1)
      {
        display.displayInformations2();
        display.setScreenState(IN_INFORMATIONS_2);
      }
      //Enter menu
      else if ((results.value == IR_MENU_CRITICAL_LEVEL || results.value == IR_MENU_PUMP_TIME) && (displayScreenState == IN_INFORMATIONS_1 || displayScreenState == IN_INFORMATIONS_2))
      {
        if(ledRed.getState())
        {
          ledRed.turnOff();
        }
        if (pump.getActiveState())
        {
          pump.stop();
          ledBlue.turnOff();
        }
        if (results.value == IR_MENU_CRITICAL_LEVEL)
        {
          display.setScreenState(IN_MENU_CRITICAL_LEVEL);
          display.displayCriticalLevelMenu();
        }
        else
        {
          display.setScreenState(IN_MENU_PUMP_TIME);
          display.displayPumpTimeMenu();
        }
      }
      //Menu plus
      else if ((displayScreenState == IN_MENU_CRITICAL_LEVEL || displayScreenState == IN_MENU_PUMP_TIME) && results.value == IR_PLUS)
      {
        if (displayScreenState == IN_MENU_CRITICAL_LEVEL)
        {
          if (localConfiguration.criticalLevel < 99)
          {
            ++localConfiguration.criticalLevel;
            display.displayCriticalLevelMenu();
          }
        }
        else
        {
          if (localConfiguration.pumpTime < 99000)
          {
            localConfiguration.pumpTime += 1000;
            display.displayPumpTimeMenu();
          }
        }
      }
      //Menu minus
      else if ((displayScreenState == IN_MENU_CRITICAL_LEVEL || displayScreenState == IN_MENU_PUMP_TIME) && results.value == IR_MINUS)
      {
        if (displayScreenState == IN_MENU_CRITICAL_LEVEL)
        {
          if (localConfiguration.criticalLevel > 1)
          {
            --localConfiguration.criticalLevel;
            display.displayCriticalLevelMenu();
          }
        }
        else
        {
          if (localConfiguration.pumpTime > 1000)
          {
            localConfiguration.pumpTime -= 1000;
            display.displayPumpTimeMenu();
          }
        }
      }
      //Save configuration 
      else if ((displayScreenState == IN_MENU_CRITICAL_LEVEL || displayScreenState == IN_MENU_PUMP_TIME) && results.value == IR_SAVE_CONFIGURATION)
      {
        configuration = localConfiguration;
        EEPROM.put(configurationAddress, configuration);
        display.displayInformations1();
        display.setScreenState(IN_INFORMATIONS_1);
        pump.setLastStopTime(currentArduinoTime);
      }
      else if ((displayScreenState == IN_MENU_CRITICAL_LEVEL || displayScreenState == IN_MENU_PUMP_TIME) && results.value == IR_CANCEL_CONFIGURATION)
      {
        localConfiguration = configuration;
        display.displayInformations1();
        pump.setLastStopTime(currentArduinoTime);
      }
    }
    
    irrecv.resume();    
  }
  return;
}

void pumpControl()
{
  int displayScreenState = display.getScreenState();
  //Pump control
  if (currentArduinoTime > 10000 && (displayScreenState == IN_INFORMATIONS_1 || displayScreenState == IN_INFORMATIONS_2))
  {
    //Check if pump is active and also check if it should pe stopped
    if (pump.getActiveState())
    {
      //If pump is active and we are in manual mode turn off the red led
      if (!configuration.mode)
      {
        if (ledRed.getState())
        {
          ledRed.turnOff();
        }
        if (pump.checkIfPumpTimeIsOver())
        {
          pump.stop();
          ledBlue.turnOff();
        }
      }
      //Pump is in automatic mode
      else
      {
        if (pump.checkIfPumpTimeIsOver())
        {
          pump.stop();
          ledBlue.turnOff();
        }
      }
    }

    //Else, check by mode if any action should be taken
    else if (!pump.getActiveState() && currentArduinoTime - pump.getLastStopTime() > 10000 && configuration.criticalLevel > humiditySensor.getHumidity())
    //else if (!pump.getActiveState() && currentArduinoTime - pump.getLastStopTime() > 10000 && 5<6)
    {
      if (!configuration.mode && !ledRed.getState())
      {
        ledRed.turnOn();
      }
      else if (configuration.mode)
      {
        pump.start();
        ledBlue.turnOn();
      }
    }
  }

  return;
}

