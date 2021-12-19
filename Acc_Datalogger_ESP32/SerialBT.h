#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include <WiFi.h>

BluetoothSerial SerialBT;

char msgBuf[256];
int16_t indxx=0;

void BTsetup()
{
  Serial.println("LOG: BTsetup\n");
  String mac = WiFi.macAddress();
  String nameDev="DICE-"+mac;
  SerialBT.begin(nameDev); //Bluetooth device name
}

bool waitCommand()
{
  char buf;
  if(SerialBT.available())
    {
      do{
          buf = SerialBT.read();
          if(buf=='\n')
            msgBuf[indxx++]='\0';
          else
            msgBuf[indxx++] = buf;
        }while( SerialBT.available()>0 || buf!='\n' );
        SerialBT.flush();//flush rest of the message
        Serial.print("Comm: ");Serial.println(msgBuf);
        return true;
    }
    delay(20);
    return false;
}
void clearBuf()
{
  indxx=0;
  msgBuf[indxx]='\0';
}
