#include "SerialBT.h"
#include "commandManager.h"
#include "Sensor.h"
#include "buttonInf.h"

#define BOOT_BUTTON 0 /*reusing boot gpio as input button*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  BTsetup();
  commandManagerInit();
  delay(3000);
  printCommandList();
  initSensor();
  calibSensor();
  setFilter();
  setRange();
  setupInterrupt();
  setupButton();
}

void loop() {
  // put your main code here, to run repeatedly:
//  if(waitCommand())
//    {
//      //commandManger();
//    }
//  clearBuf();
 serial_commands_.ReadSerial();
 buttonLoop();
 if(opMode == 2 )//single shot
 {
  Serial.println("## Operating single shot Mode ##\n waiting for input");
  SerialBT.println("## Operating single shot Mode ##\n waiting for input");

  Serial.printf("## run time: %d\n",total_time_to_sample);
  SerialBT.printf("## run time: %d\n",total_time_to_sample);
  while(buttonRead() == UNPRESSED){buttonLoop();}//wait for button activity
  loadSensor();
  unsigned long int startTime= millis();
  while((millis()-startTime)<=total_time_to_sample)
  {
    Serial.print("Timer:");
    Serial.println(millis());
    reloadSensor();
    runSensor(); 
  }
  Serial.println("Single Shot done. double press to run again");
  SerialBT.println("Single Shot done. double press to run again");
  prevMode=opMode;
  opMode=0;//don't run again unless configured again
 }else if(opMode == 3)//manual
 {
  Serial.println("## Operating Manual mode ##\n waiting for input");
  SerialBT.println("## Operating Manual mode ##\n waiting for input");
   loadSensor();
   while(buttonRead() == UNPRESSED){buttonLoop();}//wait for button activity
   while(buttonRead() == UNPRESSED)
  {
   Serial.print("Timer:");
   Serial.println(millis());
   reloadSensor();
   runSensor(); 
   buttonLoop();
  }
  Serial.println("Manual mode done. double press to run again");
  SerialBT.println("Manual mode done. double press to run again");
  prevMode=opMode;
  opMode=0;//don't run again unless configured again
 }else if(opMode == 0)
 {
  if(buttonRead()==DOUBLE_PRESSED)//if you double press once its out of any of sensor modes it goes back to previous mode
    {
      opMode=prevMode;
      Serial.println("Selecting previous mode");
      SerialBT.println("Selecting previous mod");
    }
 }
}
