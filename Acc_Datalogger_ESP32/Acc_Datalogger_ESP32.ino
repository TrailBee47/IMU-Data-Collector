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
  Serial.print("Task1 running on core: ");
  Serial.println(xPortGetCoreID());
  xTaskCreatePinnedToCore(sensor_task, "sensor", 10000, NULL, 1, NULL,  0); 
}

void loop() {
  // put your main code here, to run repeatedly:
 serial_commands_.ReadSerial();
 buttonLoop();
 printDataToSerial();
 if(opMode == MODE_NULL)
 {
  if(buttonRead()==DOUBLE_PRESSED)//if you double press once its out of any of sensor modes it goes back to previous mode
    {
      opMode=prevMode;
      Serial.println("Selecting previous mode");
      SerialBT.println("Selecting previous mode");
    }
 }
 //Serial.println(".");
 vTaskDelay(1);
}

void sensor_task(void * parameter)
{
Serial.print("Task0 running on core: ");
Serial.println(xPortGetCoreID());
 for(;;) {//infinite loop
  if(opMode == MODE_SHOT )//single shot
     {
      Serial.println("## Operating single shot Mode ##\n waiting for input");
      SerialBT.println("## Operating single shot Mode ##\n waiting for input");
    
      Serial.printf("## run time: %d\n",total_time_to_sample);
      SerialBT.printf("## run time: %d\n",total_time_to_sample);
      while(buttonRead() == UNPRESSED){
       vTaskDelay(1);
      }//wait for button activity
      loadSensor();
      unsigned long int startTime= millis();
      while((millis()-startTime)<=total_time_to_sample)
      {
        Serial.print("Timer:");
        Serial.println(millis());
        reloadSensor();
        runSensor();
        vTaskDelay(1); 
      }
      Serial.println("Single Shot done. double press to run again");
      SerialBT.println("Single Shot done. double press to run again");
      prevMode=opMode;
      opMode=0;//don't run again unless configured again
     }
  else if(opMode == MODE_MANUAL)//manual
     {
      Serial.println("## Operating Manual mode ##\n waiting for input");
      SerialBT.println("## Operating Manual mode ##\n waiting for input");
       loadSensor();
       while(buttonRead() == UNPRESSED){
        vTaskDelay(1);
       }//wait for button activity
       while(buttonRead() == UNPRESSED)
      {
       Serial.print("Timer:");
       Serial.println(millis());
       reloadSensor();
       runSensor(); 
       buttonLoop();
       vTaskDelay(1);
      }
      Serial.println("Manual mode done. double press to run again");
      SerialBT.println("Manual mode done. double press to run again");
      prevMode = opMode;
      opMode = MODE_NULL;//don't run again unless configured again
     }
     // Serial.println("#");
     vTaskDelay(1);
  }
}

 /*Core: 0
 * Task0:(sensor_task)
 * read sensors
 */
 
 /*Core: 1
 *Task1:(main/arduino loop)
 *read button input
 *read serial input
 *read bluetooth serial
 *print Sensor data 
 *
 */
