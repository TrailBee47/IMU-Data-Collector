#include <SerialCommands.h>
#define SERIAL_COMMANDS_DEBUG

int indx=0;
enum {MODE_NULL=0,MODE_SELECTOR=1, MODE_SHOT, MODE_MANUAL};
int opMode=MODE_NULL; //inital state set to 1
int prevMode=MODE_MANUAL;

int total_time_to_sample=5000; //10s= 10000ms
bool paramconfig=false;
bool paramcalib=false;

char serial_command_buffer_[32];
SerialCommands serial_commands_(&SerialBT, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");
void printCommandList()
{
 Serial.println("######## Dice-v1.0 ########");
 Serial.println("\tThis device is simple Data-Collector\n\tfor your machine learning data set.\n");                       
 Serial.println("Command list:");
 Serial.println("HELP\t\t :will print list of commands"); 
 Serial.println("SENSOR\t\t :All commands starting for sensor config"); 
 Serial.println("SENSOR RESET\t\t :reboots sensor"); 
 Serial.println("SENSOR CALIB\t\t :calibrate sensor");
 Serial.println("SENSOR CONFIG 12000\t :sets sensor sampling time 12000 milliseconds");
 Serial.println("SENSOR MODE <modes>\t :sets sensor data collection mode");
 Serial.println("\t\t<modes>\t\t\n\t\t SHOT\t :single shot mode, data collection starts on external trigger");
 Serial.println("\t\t MANUAL\t :manual mode, data collection start and stop manually on button");

 SerialBT.println("######## Dice-v1.0 ########");
 SerialBT.println("\tThis device is simple Data-Collector\n\tfor your machine learning data set.\n");                       
 SerialBT.println("Command list:");
 SerialBT.println("HELP\t\t :will print list of commands"); 
 SerialBT.println("SENSOR\t\t :All commands starting for sensor config"); 
 SerialBT.println("SENSOR RESET\t\t :reboots sensor");
 SerialBT.println("SENSOR CALIB\t\t :calibrate sensor");
 SerialBT.println("SENSOR CONFIG 12000\t :sets sensor sampling time 12000 milliseconds");
 SerialBT.println("SENSOR MODE <modes>\t :sets sensor data collection mode");
 SerialBT.println("\t\t<modes>\t\t\n\t\t SHOT\t :single shot mode, data collection starts on external trigger");
 SerialBT.println("\t\t MANUAL\t :manual mode, data collection start and stop manually on button");
}

//This is the default handler, and gets called when no other command matches. 
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}
/*
 * sensor reset
 * sensor config total_time_to_sample
 * sensor mode single_shot_samples(shot)/manual start-stop(start)
*/
void cmd_help(SerialCommands* sender)
{
  //just prints help
 printCommandList();
}
void cmd_sensor(SerialCommands* sender)
{
  //Note: Every call to Next moves the pointer to next parameter
  char* paramstr;
  
  while( (paramstr = sender->Next())!= NULL)
  {
    indx++;
    Serial.printf("params[%d]: ",indx);
    Serial.println(paramstr);
    sender->GetSerial()->printf("params[%d]: ",indx);
    sender->GetSerial()->println(paramstr);
    if(indx == 1 && (strcmp(paramstr,"RESET")==0 || strcmp(paramstr,"reset")==0))
    {
      //command is for reset
      Serial.println("command for reset");
      sender->GetSerial()->println("command for reset");
    }
    if(indx == 1 && (strcmp(paramstr,"CALIB")==0 || strcmp(paramstr,"calib")==0))
    {
      paramcalib = true;
    }
    if(indx == 1 && (strcmp(paramstr,"CONFIG")==0 || strcmp(paramstr,"config")==0))
    {
      paramconfig = true;
    }
    if(indx == 1 &&  (strcmp(paramstr,"MODE")==0 || strcmp(paramstr,"mode")==0))
    {
      //mode for sensor
      opMode = MODE_SELECTOR;
    }
    if(indx == 2 && paramconfig)
    {
      //total_time_to_sample value
      total_time_to_sample = atoi(paramstr);//how do we make sure the value is only numeric?
      Serial.printf("total time to sample: %d milliSeconds\n", total_time_to_sample);
      sender->GetSerial()->printf("total time to sample: %d milliSeconds\n", total_time_to_sample);
    }
    if(indx == 2 && opMode == MODE_SELECTOR)
    {
      //total_time_to_sample value
      if(strcmp(paramstr,"shot")==0 || strcmp(paramstr,"SHOT")==0)
      {
        opMode = MODE_SHOT;//how do we make sure the value is only numeric?
        Serial.println("single shot sampling");
        sender->GetSerial()->println("single shot sampling");
      }else if(strcmp(paramstr,"manual")==0 || strcmp(paramstr,"MANUAL")==0)
      {
        opMode = MODE_MANUAL;//how do we make sure the value is only numeric?
        Serial.println("manual start stop mode selected");
        sender->GetSerial()->println("manual start stop mode selected");
      }
    }
  }
  indx=0;
  paramconfig=0;
  //opMode=0;
}

SerialCommand cmd_sensor_("SENSOR", cmd_sensor);
SerialCommand cmd_help_("HELP", cmd_help);

void commandManagerInit()
{
  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_sensor_);
  serial_commands_.AddCommand(&cmd_help_); 
} 
