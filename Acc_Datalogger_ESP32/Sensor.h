

#include <MPU9250_WE.h>
#include <Wire.h>
#include <RingBuf.h>

#define MPU9250_ADDR 0x68

const int intPin = 26;
volatile bool fifoFull = false;

bool printData=true;

typedef struct mpuData{
  xyzFloat acc;
  xyzFloat gyr;
  unsigned long int idx;
  }mpuData_t;

mpuData_t dataAccGyr{ .idx=0};
mpuData_t dataReadAccGyr;

#define BUFFER_SIZE (42*10)
RingBuf<mpuData_t, BUFFER_SIZE> myBuffer;

int SAMPLE_DIV = 4;

/* There are several ways to create your MPU9250 object:
 * MPU9250_WE myMPU9250 = MPU9250_WE()              -> uses Wire / I2C Address = 0x68
 * MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR)  -> uses Wire / MPU9250_ADDR
 * MPU9250_WE myMPU9250 = MPU9250_WE(&wire2)        -> uses the TwoWire object wire2 / MPU9250_ADDR
 * MPU9250_WE myMPU9250 = MPU9250_WE(&wire2, MPU9250_ADDR) -> all together
 * Successfully tested with two I2C busses on an ESP32
 */
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

void IRAM_ATTR eventISR() {
  fifoFull = true;
}

void calibSensor()
{
    /* The slope of the curve of acceleration vs measured values fits quite well to the theoretical 
   * values, e.g. 16384 units/g in the +/- 2g range. But the starting point, if you position the 
   * MPU9250 flat, is not necessarily 0g/0g/1g for x/y/z. The autoOffset function measures offset 
   * values. It assumes your MPU9250 is positioned flat with its x,y-plane. The more you deviate 
   * from this, the less accurate will be your results.
   * The function also measures the offset of the gyroscope data. The gyroscope offset does not   
   * depend on the positioning.
   * This function needs to be called at the beginning since it can overwrite your settings!
   */
  Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
  SerialBT.println("Position you MPU9250 flat and don't move it - calibrating...");
  delay(3000);
  myMPU9250.autoOffsets();
  Serial.println("Done!");
  SerialBT.println("Done!");
}

void setFilter()
{
  Serial.println("setting sensor filter");
   /*  You can enable or disable the digital low pass filter (DLPF). If you disable the DLPF, you 
   *  need to select the bandwidth, which can be either 8800 or 3600 Hz. 8800 Hz has a shorter delay,
   *  but higher noise level. If DLPF is disabled, the output rate is 32 kHz.
   *  MPU9250_BW_WO_DLPF_3600 
   *  MPU9250_BW_WO_DLPF_8800
   */
  myMPU9250.enableGyrDLPF();
  //myMPU9250.disableGyrDLPF(MPU9250_BW_WO_DLPF_8800); // bandwidth without DLPF
  
  /*  Digital Low Pass Filter for the gyroscope must be enabled to choose the level. 
   *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
   *  
   *  DLPF    Bandwidth [Hz]   Delay [ms]   Output Rate [kHz]
   *    0         250            0.97             8
   *    1         184            2.9              1
   *    2          92            3.9              1
   *    3          41            5.9              1
   *    4          20            9.9              1
   *    5          10           17.85             1
   *    6           5           33.48             1
   *    7        3600            0.17             8
   *    
   *    You achieve lowest noise using level 6  
   */
  myMPU9250.setGyrDLPF(MPU9250_DLPF_5);

    /*  Enable/disable the digital low pass filter for the accelerometer 
   *  If disabled the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
   */
  myMPU9250.enableAccDLPF(true);

  /*  Digital low pass filter (DLPF) for the accelerometer, if enabled 
   *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
   *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
   *     0           460               1.94           1
   *     1           184               5.80           1
   *     2            92               7.80           1
   *     3            41              11.80           1
   *     4            20              19.80           1
   *     5            10              35.70           1
   *     6             5              66.96           1
   *     7           460               1.94           1
   */
  myMPU9250.setAccDLPF(MPU9250_DLPF_0);
}

void setRange()
{
  /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
   *  Sample rate = Internal sample rate / (1 + divider) 
   *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
   *  Divider is a number 0...255
   */
   Serial.println("setting Range");
  myMPU9250.setSampleRateDivider(SAMPLE_DIV);

  /*  MPU9250_GYRO_RANGE_250       250 degrees per second (default)
   *  MPU9250_GYRO_RANGE_500       500 degrees per second
   *  MPU9250_GYRO_RANGE_1000     1000 degrees per second
   *  MPU9250_GYRO_RANGE_2000     2000 degrees per second
   */
  myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_250);

  /*  MPU9250_ACC_RANGE_2G      2 g   (default)
   *  MPU9250_ACC_RANGE_4G      4 g
   *  MPU9250_ACC_RANGE_8G      8 g   
   *  MPU9250_ACC_RANGE_16G    16 g
   */
  myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
}
void setupInterrupt()
{
  Serial.println("Setting interrupt");
  pinMode(intPin,INPUT);
 /*  Set the interrupt pin:
   *  MPU9250_ACT_LOW  = active-low
   *  MPU9250_ACT_HIGH = active-high (default) 
   */
  //myMPU9250.setIntPinPolarity(MPU9250_ACT_LOW); 

  /*  If latch is enabled the interrupt pin level is held until the interrupt status 
   *  is cleared. If latch is disabled the interrupt pulse is ~50µs (default).
   */
  myMPU9250.enableIntLatch(true);

  /*  The interrupt can be cleared by any read or it will only be cleared if the interrupt 
   *  status register is read (default).
   */
  //myMPU9250.enableClearIntByAnyRead(true); 

  /*  Enable/disable interrupts:
   *  MPU9250_DATA_READY 
   *  MPU9250_FIFO_OVF   
   *  MPU9250_WOM_INT    
   *  
   *  You can enable all interrupts.
   */
  myMPU9250.enableInterrupt(MPU9250_FIFO_OVF); 
  //myMPU9250.disableInterrupt(MPU9250_FIFO_OVF);

  /*  Set the wake on motion threshold (WOM threshold)
   *  Choose 1 (= 4 mg) ..... 255 (= 1020 mg); 
   */
  //myMPU9250.setWakeOnMotionThreshold(170);
  
  /*  Enable/disable wake on motion (WOM) and  WOM mode:
   *  MPU9250_WOM_DISABLE
   *  MPU9250_WOM_ENABLE
   *  ***
   *  MPU9250_WOM_COMP_DISABLE   // reference is the starting value
   *  MPU9250_WOM_COMP_ENABLE    // reference is tha last value
   */
  //myMPU9250.enableWakeOnMotion(MPU9250_WOM_ENABLE, MPU9250_WOM_COMP_DISABLE);

  attachInterrupt(intPin, eventISR, RISING);
  myMPU9250.setFifoMode(MPU9250_STOP_WHEN_FULL);
  myMPU9250.enableFifo(true);
}
void initSensor()
{
  Wire.begin();
  if(!myMPU9250.init()){
    Serial.println("MPU9250 does not respond");
    SerialBT.println("MPU9250 does not respond");
  }
  else{
    Serial.println("MPU9250 is connected");
    SerialBT.println("MPU9250 is connected");
  }
}
void fetchFifo(){
  int count = myMPU9250.getFifoCount();
  int dataSets = myMPU9250.getNumberOfFifoDataSets(); 
//  Serial.print("Bytes in Fifo: ");
//  Serial.println(count);
//  Serial.print("Data Sets: ");
//  Serial.println(dataSets);
//  SerialBT.print("Bytes in Fifo: ");
//  SerialBT.println(count);
//  SerialBT.print("Data Sets: ");
//  SerialBT.println(dataSets);

  for(int i=0; i<dataSets; i++){
    xyzFloat gValue = myMPU9250.getGValuesFromFifo();
    xyzFloat gyr = myMPU9250.getGyrValuesFromFifo();
    
    if(!myBuffer.isFull())
    {
      dataAccGyr.acc = gValue;
      dataAccGyr.gyr = gyr;
      dataAccGyr.idx++;
      myBuffer.push(dataAccGyr);
    }else
    {
      Serial.println("buffer is full");
      SerialBT.print("buffer is full");
    }
  }
}
void countDown(){
  Serial.println("Ready to collect interesting data");
  Serial.println();
  delay(1000);
  Serial.print("Fifo collection begins in 3, "); 
  delay(1000);
  Serial.print("2, "); 
  delay(1000);
  Serial.print("1, "); 
  delay(1000);
  Serial.println("Now!");
}
void loadSensor()
{
  Serial.println("loading Sensor");
  countDown();
  myMPU9250.readAndClearInterrupts();
}
void reloadSensor()
{
  myMPU9250.readAndClearInterrupts();
}
void runSensor()
{
  Serial.println("Running sensor");
  //call loadSensor before runSensor is called for first time
  fifoFull = false;
  myMPU9250.startFifo(MPU9250_FIFO_ACC_GYR);
  unsigned long int startstartTime= millis();
  while(!fifoFull){
      if(!myBuffer.isEmpty())
      {
       myBuffer.pop(dataReadAccGyr); 
       if(printData){
//          Serial.print("Data set ");
//          Serial.print(i+1);
//          Serial.println(":");
//          SerialBT.print("Data set ");
//          SerialBT.print(i+1);
//          SerialBT.println(":");
    
          Serial.printf("[%ld] ",(dataReadAccGyr.idx));
          Serial.print(dataReadAccGyr.acc.x);
          Serial.print("   ");
          Serial.print(dataReadAccGyr.acc.y);
          Serial.print("   ");
          Serial.print(dataReadAccGyr.acc.z);
          Serial.print("   ");
          SerialBT.print(dataReadAccGyr.acc.x);
          SerialBT.print("   ");
          SerialBT.print(dataReadAccGyr.acc.y);
          SerialBT.print("   ");
          SerialBT.print(dataReadAccGyr.acc.z);
          SerialBT.print("   ");
          
          Serial.print(dataReadAccGyr.gyr.x);
          Serial.print("   ");
          Serial.print(dataReadAccGyr.gyr.y);
          Serial.print("   ");
          Serial.println(dataReadAccGyr.gyr.z);
          SerialBT.print(dataReadAccGyr.gyr.x);
          SerialBT.print("   ");
          SerialBT.print(dataReadAccGyr.gyr.y);
          SerialBT.print("   ");
          SerialBT.println(dataReadAccGyr.gyr.z);
        }
      }
    }
  unsigned long int startTime= millis();
  myMPU9250.stopFifo();
  fetchFifo();
  myMPU9250.resetFifo();
  Serial.printf("time delta: %ld %ld\n",(startTime-startstartTime),(millis()-startTime));
//  Serial.println("For another series of measurements, enter any key and send");
//  while(!(Serial.available())){}
//  Serial.read();
//  Serial.println(); 
}
