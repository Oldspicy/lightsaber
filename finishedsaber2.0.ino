#include <Wire.h>
#include <SPI.h>
#include <Audio.h>
#include <SD.h>
#include <SerialFlash.h>
#include <WS2812Serial.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!


// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav3;     //xy=154,392
AudioPlaySdWav           playSdWav2;     //xy=159,355
AudioPlaySdWav           playSdWav1;     //xy=384,2003
AudioMixer4              mixer1;         //xy=456,406
AudioOutputTDM           tdm1;           //xy=558,2017
AudioOutputAnalogStereo  dacs1;          //xy=659,403
AudioOutputAnalog        dac1;           //xy=775,2058
AudioConnection          patchCord3(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord4(playSdWav2, 1, mixer1, 0);
AudioConnection          patchCord5(playSdWav1, 0, mixer1, 2);
AudioConnection          patchCord6(playSdWav1, 1, mixer1, 3);
AudioConnection          patchCord7(mixer1, 0, dacs1, 0);
AudioConnection          patchCord8(mixer1, 0, dacs1, 1);
// GUItool: end automatically generated code



#define SDCARD_CS_PIN     BUILTIN_SDCARD
#define SDCARD_MOSI_PIN   11  // not actually used
#define SDCARD_SCK_PIN    13  // not actually used
#define PIN               1
#define NUMPIXELS         130
#define BUTTON            33
#define COLORBUTTON       34
#define CLASHINTERRUPT    26
#define GYROPOWER         22
#define GYROGROUND        20




byte drawingMemory[NUMPIXELS*3];         //  3 bytes per LED
DMAMEM byte displayMemory[NUMPIXELS*12]; // 12 bytes per LED
int previousMillisInterrupt = 0;
int previousMillisAccel = 0;
int currentMillisAccel = 0;
int xSwingThresholdPositive = 100;
int zSwingThresholdPositive = 100;
int xSwingThresholdNegative = -100;
int zSwingThresholdNegative = -100;
int clashThreshold = 24;
int clashThresholdNegative = -24;
int flashTime = 75;
int function = 0;
int r = 0;
int g = 100;
int b = 0;
int colorFlag = 0;
int brightnessCounter = 200;
int brightnessFlag = 1;






Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
WS2812Serial leds(NUMPIXELS, displayMemory, drawingMemory, PIN, WS2812_GRB);


#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif


/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void setup() 
{
  Serial.begin(9600);
  pinMode(GYROPOWER, OUTPUT);
  pinMode(GYROGROUND, OUTPUT);
  pinMode(COLORBUTTON, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(CLASHINTERRUPT, INPUT_PULLUP);

  pinMode(2,0);
  pinMode(4,0);
  digitalWrite(GYROPOWER, 1);
  digitalWrite(GYROGROUND, 0);
  delay(250);


//  while (!Serial) {
//    delay(1); // will pause Zero, Leonardo, etc until serial console opens
//  }
  Serial.println("LSM9DS1 data read demo");
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS1 9DOF");
  setupSensor();
 
  leds.begin();
  for(int i=NUMPIXELS;i>-10;i--)
  {
    leds.setPixel(i, Color(0,0,0));
    leds.show();
  }

  
  AudioMemory(8);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) 
  {
    while (1) 
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  mixer1.gain(0, 0.1f);
  mixer1.gain(1, 0.1f);

  
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPress, FALLING);

  
  if (playSdWav1.isPlaying() == false) 
  {
    playSdWav1.play("R2D2.WAV");
    delay(10); // wait for library to parse WAV info
  }
  Serial.println("Setup complete");
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void loop() 
{
  colorChooser();
  if (function == 0) off();
  if (function == 1) saberOn();
  if (function == 2) saberRun();
  if (function == 3) saberOff();

}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void buttonPress()
{
  unsigned long currentMillisInterrupt = millis();
  if (currentMillisInterrupt - previousMillisInterrupt >=1000)
  {
    function++;
    if (function >=4) function = 0;
    previousMillisInterrupt = currentMillisInterrupt;
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void off()
{
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void saberRun()
{
  //Serial.println(analogRead(A13));
  if (playSdWav2.isPlaying() == false) 
  {
    playSdWav2.play("IDLE.WAV");
    delay(10);
  }
  int dimValue = 200;
  r = (r * brightnessCounter) >> 8;
  g = (g * brightnessCounter) >> 8;
  b = (b * brightnessCounter) >> 8;

  for(int i=0;i<NUMPIXELS;i++)
  {
    leds.setPixel(i, Color(g,b,r));
  }
  leds.show();

  if (brightnessFlag == 0)
  {
    brightnessCounter--;
    if (brightnessCounter <=dimValue) brightnessFlag = 1;
  }
  if (brightnessFlag == 1)
  {
    brightnessCounter++;
    if (brightnessCounter >=255) brightnessFlag = 0;
  }

  accelerometerData();
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void colorChooser()
{
  byte colorByte = 0;
  if (digitalRead(COLORBUTTON) == 0)
  {
    delay(250);
    if (digitalRead(COLORBUTTON) == 1)
    {
      colorFlag++;
      if (colorFlag >=5) colorFlag = 0;
      colorByte = 1;
    }
    else if (digitalRead(COLORBUTTON) == 0)
    {
      spark();
    }
    
  }
  switch (colorFlag)
  {
    case 0:
      r = 200;
      g = 0;
      b = 0;
      if (colorByte == 1)
      {
        playSdWav1.play("RED.WAV");
        delay(10);
        colorByte = 0;
      }
      break;
    case 1:
      r = 0;
      g = 200;
      b = 0;
      if (colorByte == 1)
      {
        playSdWav1.play("GREEN.WAV");
        delay(10);
        colorByte = 0;
      }
      break;
    case 2:
      r = 0;
      g = 0;
      b = 200;
      if (colorByte == 1)
      {
        playSdWav1.play("BLUE.WAV");
        delay(10);
        colorByte = 0;
      }
      break;
    case 3:
      r = 100;
      g = 0;
      b = 200;
      if (colorByte == 1)
      {
        playSdWav1.play("PURPLE.WAV");
        delay(10);
        colorByte = 0;
      }
      break;
    case 4:
      r = 0;
      g = 200;
      b = 100;
      if (colorByte == 1)
      {
        playSdWav1.play("ICE.WAV");
        delay(10);
        colorByte = 0;
      }
      break;
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void batteryCheck()
{
  int batteryRed;
  int batteryGreen;
  int batteryBlue;
  int batteryLevel;


  while (digitalRead(COLORBUTTON) == 0)
  { 
    int mv = analogRead(A13);
    int batteryLevel = map(mv, 555, 610, 10, 126);
    batteryLevel = constrain(batteryLevel, 10,126);


    //Serial.println(mv);

    for (int x = 0; x <= batteryLevel; x++)
    {
      if (x <= 30)
      {
        batteryRed = 255;
        batteryGreen = 0;
        batteryBlue = 0;
      }
      if (x > 30 && x <=54)
      {
        batteryRed = 255;
        batteryGreen = 40;
        batteryBlue = 0;
      }
      if (x > 54 && x <=78)
      {
        batteryRed = 255;
        batteryGreen = 255;
        batteryBlue = 0;
      }
      if (x > 78 && x <= 102)
      {
        batteryRed = 153;
        batteryGreen = 255;
        batteryBlue = 0;
      }
      if (x > 102)
      {
        batteryRed = 0;
        batteryGreen = 255;
        batteryBlue = 0;
      }
      leds.setPixel(x, Color(batteryGreen, batteryBlue, batteryRed));
      for (int x = batteryLevel+1; x <= NUMPIXELS; x++)
      {
        leds.setPixel(x, Color(0, 0, 0));
      }
    }
    leds.show();
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/


unsigned int Color(byte r, byte g, byte b)
{
  return (((unsigned int)b << 16) | ((unsigned int)r << 8) | (unsigned int)g);
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void accelerometerData()
{
  currentMillisAccel = millis();
  lsm.read();  /* ask it to read in the data */ 
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 

  int accelerationTotal = (a.acceleration.x + a.acceleration.y + a.acceleration.z);
  Serial.println(accelerationTotal);
  Serial.println();

  if (accelerationTotal >= clashThreshold) clash();
  if (accelerationTotal <= clashThresholdNegative) clash();

  int swingChoosingVariable = random(0,5);
  
  if (g.gyro.x >= xSwingThresholdPositive || g.gyro.x <= xSwingThresholdNegative)
  {
    swingChooser(swingChoosingVariable);
  }
  if (g.gyro.z >= zSwingThresholdPositive || g.gyro.z <= zSwingThresholdNegative)
  {
    swingChooser(swingChoosingVariable);
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void clash()
{
  int sound = random(0,7);

  switch(sound)
  {
    case 0:
      playSdWav1.play("CLASH1.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
    case 1:
      playSdWav1.play("CLASH2.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
    case 2:
      playSdWav1.play("CLASH3.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
    case 3:
      playSdWav1.play("CLASH4.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
    case 4:
      playSdWav1.play("CLASH5.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
    case 5:
      playSdWav1.play("CLASH6.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
    case 6:
      playSdWav1.play("CLASH7.WAV");
      delay(10);
      for(int i=0;i<NUMPIXELS;i++) leds.setPixel(i, Color(0,100,100));
      leds.show();
      delay(flashTime);
      break;
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void swingChooser(int call)
{
  if (call == 0)
  {
    if (playSdWav1.isPlaying() == false) 
        {
          playSdWav1.play("SWING1.WAV");
          delay(10); // wait for library to parse WAV info
        }
  }
  else if (call == 1)
  {
    if (playSdWav1.isPlaying() == false) 
        {
          playSdWav1.play("SWING2.WAV");
          delay(10); // wait for library to parse WAV info
        }
  }
  else if (call == 2)
  {
    if (playSdWav1.isPlaying() == false) 
        {
          playSdWav1.play("SWING3.WAV");
          delay(10); // wait for library to parse WAV info
        }
  }
  else if (call == 3)
  {
    if (playSdWav1.isPlaying() == false) 
        {
          playSdWav1.play("SWING6.WAV");
          delay(10); // wait for library to parse WAV info
        }
  }
  else if (call == 4)
  {
    if (playSdWav1.isPlaying() == false) 
        {
          playSdWav1.play("SWING8.WAV");
          delay(10); // wait for library to parse WAV info
        }
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void saberOn()
{
  if (playSdWav1.isPlaying() == false) 
  {
    playSdWav1.play("SABERON.WAV");
    delay(10); // wait for library to parse WAV info
    playSdWav2.play("IDLE.WAV");
    delay(10);
  }

/*******Semi-Fast Transition*******/  
  leds.setPixel(1, Color(g,b,r));
  leds.setPixel(2, Color(g,b,r));
  leds.show();
  leds.setPixel(3, Color(g,b,r));
  leds.setPixel(4, Color(g,b,r));
  leds.show();
  leds.setPixel(5, Color(g,b,r));
  leds.setPixel(6, Color(g,b,r));
  leds.show();
  leds.setPixel(7, Color(g,b,r));
  leds.setPixel(8, Color(g,b,r));
  leds.show();
  leds.setPixel(9, Color(g,b,r));
  leds.setPixel(10, Color(g,b,r));
  leds.show();
  leds.setPixel(11, Color(g,b,r));
  leds.setPixel(12, Color(g,b,r));
  leds.show();
  leds.setPixel(13, Color(g,b,r));
  leds.setPixel(14, Color(g,b,r));
  leds.show();
  leds.setPixel(15, Color(g,b,r));
  leds.setPixel(16, Color(g,b,r));
  leds.show();
  leds.setPixel(17, Color(g,b,r));
  leds.setPixel(18, Color(g,b,r));
  leds.show();
  leds.setPixel(19, Color(g,b,r));
  leds.setPixel(20, Color(g,b,r));
  leds.show();
  leds.setPixel(21, Color(g,b,r));
  leds.setPixel(22, Color(g,b,r));
  leds.show();
  leds.setPixel(23, Color(g,b,r));
  leds.setPixel(24, Color(g,b,r));
  leds.show();
  leds.setPixel(25, Color(g,b,r));
  leds.setPixel(26, Color(g,b,r));
  leds.show();
  leds.setPixel(27, Color(g,b,r));
  leds.setPixel(28, Color(g,b,r));
  leds.show();
  leds.setPixel(29, Color(g,b,r));
  leds.setPixel(30, Color(g,b,r));
  leds.show();
  leds.setPixel(31, Color(g,b,r));
  leds.setPixel(32, Color(g,b,r));
  leds.show();
  leds.setPixel(33, Color(g,b,r));
  leds.setPixel(34, Color(g,b,r));
  leds.show();
  leds.setPixel(35, Color(g,b,r));
  leds.setPixel(36, Color(g,b,r));
  leds.show();
  leds.setPixel(37, Color(g,b,r));
  leds.setPixel(38, Color(g,b,r));
  leds.show();
  leds.setPixel(39, Color(g,b,r));
  leds.setPixel(40, Color(g,b,r));
  leds.show();
  leds.setPixel(41, Color(g,b,r));
  leds.setPixel(42, Color(g,b,r));
  leds.show();
  leds.setPixel(43, Color(g,b,r));
  leds.setPixel(44, Color(g,b,r));
  leds.show();
  leds.setPixel(45, Color(g,b,r));
  leds.setPixel(46, Color(g,b,r));
  leds.show();
  leds.setPixel(47, Color(g,b,r));
  leds.setPixel(48, Color(g,b,r));
  leds.show();
  leds.setPixel(49, Color(g,b,r));
  leds.setPixel(50, Color(g,b,r));
  leds.show();
  leds.setPixel(51, Color(g,b,r));
  leds.setPixel(52, Color(g,b,r));
  leds.show();
  leds.setPixel(53, Color(g,b,r));
  leds.setPixel(54, Color(g,b,r));
  leds.show();
  leds.setPixel(55, Color(g,b,r));
  leds.setPixel(56, Color(g,b,r));
  leds.show();
  leds.setPixel(57, Color(g,b,r));
  leds.setPixel(58, Color(g,b,r));
  leds.show();
  leds.setPixel(59, Color(g,b,r));
  leds.setPixel(60, Color(g,b,r));
  leds.show();
  leds.setPixel(61, Color(g,b,r));
  leds.setPixel(62, Color(g,b,r));
  leds.show();
  leds.setPixel(63, Color(g,b,r));
  leds.setPixel(64, Color(g,b,r));
  leds.show();
  leds.setPixel(65, Color(g,b,r));
  leds.setPixel(66, Color(g,b,r));
  leds.show();
  leds.setPixel(67, Color(g,b,r));
  leds.setPixel(68, Color(g,b,r));
  leds.show();
  leds.setPixel(69, Color(g,b,r));
  leds.setPixel(70, Color(g,b,r));
  leds.show();
  leds.setPixel(71, Color(g,b,r));
  leds.setPixel(72, Color(g,b,r));
  leds.show();
  leds.setPixel(73, Color(g,b,r));
  leds.setPixel(74, Color(g,b,r));
  leds.show();
  leds.setPixel(75, Color(g,b,r));
  leds.setPixel(76, Color(g,b,r));
  leds.show();
  leds.setPixel(77, Color(g,b,r));
  leds.setPixel(78, Color(g,b,r));
  leds.show();
  leds.setPixel(79, Color(g,b,r));
  leds.setPixel(80, Color(g,b,r));
  leds.show();
  leds.setPixel(81, Color(g,b,r));
  leds.setPixel(82, Color(g,b,r));
  leds.show();
  leds.setPixel(83, Color(g,b,r));
  leds.setPixel(84, Color(g,b,r));
  leds.show();
  leds.setPixel(85, Color(g,b,r));
  leds.setPixel(86, Color(g,b,r));
  leds.show();
  leds.setPixel(87, Color(g,b,r));
  leds.setPixel(88, Color(g,b,r));
  leds.show();
  leds.setPixel(89, Color(g,b,r));
  leds.setPixel(90, Color(g,b,r));
  leds.show();
  leds.setPixel(91, Color(g,b,r));
  leds.setPixel(92, Color(g,b,r));
  leds.show();
  leds.setPixel(93, Color(g,b,r));
  leds.setPixel(94, Color(g,b,r));
  leds.show();
  leds.setPixel(95, Color(g,b,r));
  leds.setPixel(96, Color(g,b,r));
  leds.show();
  leds.setPixel(97, Color(g,b,r));
  leds.setPixel(98, Color(g,b,r));
  leds.show();
  leds.setPixel(99, Color(g,b,r));
  leds.setPixel(100, Color(g,b,r));
  leds.show();
  leds.setPixel(101, Color(g,b,r));
  leds.setPixel(102, Color(g,b,r));
  leds.show();
  leds.setPixel(103, Color(g,b,r));
  leds.setPixel(104, Color(g,b,r));
  leds.show();
  leds.setPixel(105, Color(g,b,r));
  leds.setPixel(106, Color(g,b,r));
  leds.show();
  leds.setPixel(107, Color(g,b,r));
  leds.setPixel(108, Color(g,b,r));
  leds.show();
  leds.setPixel(109, Color(g,b,r));
  leds.setPixel(110, Color(g,b,r));
  leds.show();
  leds.setPixel(111, Color(g,b,r));
  leds.setPixel(112, Color(g,b,r));
  leds.show();
  leds.setPixel(113, Color(g,b,r));
  leds.setPixel(114, Color(g,b,r));
  leds.show();
  leds.setPixel(115, Color(g,b,r));
  leds.setPixel(116, Color(g,b,r));
  leds.show();
  leds.setPixel(117, Color(g,b,r));
  leds.setPixel(118, Color(g,b,r));
  leds.show();
  leds.setPixel(119, Color(g,b,r));
  leds.setPixel(120, Color(g,b,r));
  leds.show();
  leds.setPixel(121, Color(g,b,r));
  leds.setPixel(122, Color(g,b,r));
  leds.show();
  leds.setPixel(123, Color(g,b,r));
  leds.setPixel(124, Color(g,b,r));
  leds.show();
  leds.setPixel(125, Color(g,b,r));
  leds.setPixel(126, Color(g,b,r));
  leds.show();
  leds.setPixel(127, Color(g,b,r));
  leds.setPixel(128, Color(g,b,r));
  leds.show();
  leds.setPixel(129, Color(g,b,r));
  leds.setPixel(130, Color(g,b,r));
  leds.show();
  leds.setPixel(131, Color(g,b,r));
  leds.setPixel(132, Color(g,b,r));
  leds.show();
  leds.setPixel(133, Color(g,b,r));
  leds.setPixel(134, Color(g,b,r));
  leds.show();
  leds.setPixel(135, Color(g,b,r));
  leds.setPixel(136, Color(g,b,r));
  leds.show();
  leds.setPixel(137, Color(g,b,r));
  leds.setPixel(138, Color(g,b,r));
  leds.show();
  leds.setPixel(139, Color(g,b,r));
  leds.setPixel(140, Color(g,b,r));
  leds.show();
  function = 2;
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void saberOff()
{
  playSdWav1.play("SABEROFF.WAV");
  delay(10);
  leds.setPixel(140, Color(0,0,0));
  leds.setPixel(139, Color(0,0,0));
  leds.show();
  leds.setPixel(138, Color(0,0,0));
  leds.setPixel(137, Color(0,0,0));
  leds.show();
  leds.setPixel(136, Color(0,0,0));
  leds.setPixel(135, Color(0,0,0));
  leds.show();
  leds.setPixel(134, Color(0,0,0));
  leds.setPixel(133, Color(0,0,0));
  leds.show();
  leds.setPixel(132, Color(0,0,0));
  leds.setPixel(131, Color(0,0,0));
  leds.show();
  leds.setPixel(130, Color(0,0,0));
  leds.setPixel(129, Color(0,0,0));
  leds.show();
  leds.setPixel(128, Color(0,0,0));
  leds.setPixel(127, Color(0,0,0));
  leds.show();
  leds.setPixel(126, Color(0,0,0));
  leds.setPixel(125, Color(0,0,0));
  leds.show();
  leds.setPixel(124, Color(0,0,0));
  leds.setPixel(123, Color(0,0,0));
  leds.show();
  leds.setPixel(122, Color(0,0,0));
  leds.setPixel(121, Color(0,0,0));
  leds.show();
  leds.setPixel(120, Color(0,0,0));
  leds.setPixel(119, Color(0,0,0));
  leds.show();
  leds.setPixel(118, Color(0,0,0));
  leds.setPixel(117, Color(0,0,0));
  leds.show();
  leds.setPixel(116, Color(0,0,0));
  leds.setPixel(115, Color(0,0,0));
  leds.show();
  leds.setPixel(114, Color(0,0,0));
  leds.setPixel(113, Color(0,0,0));
  leds.show();
  leds.setPixel(112, Color(0,0,0));
  leds.setPixel(111, Color(0,0,0));
  leds.show();
  leds.setPixel(110, Color(0,0,0));
  leds.setPixel(109, Color(0,0,0));
  leds.show();
  leds.setPixel(108, Color(0,0,0));
  leds.setPixel(107, Color(0,0,0));
  leds.show();
  leds.setPixel(106, Color(0,0,0));
  leds.setPixel(105, Color(0,0,0));
  leds.show();
  leds.setPixel(104, Color(0,0,0));
  leds.setPixel(103, Color(0,0,0));
  leds.show();
  leds.setPixel(102, Color(0,0,0));
  leds.setPixel(101, Color(0,0,0));
  leds.show();
  leds.setPixel(100, Color(0,0,0));
  leds.setPixel(99, Color(0,0,0));
  leds.show();
  leds.setPixel(98, Color(0,0,0));
  leds.setPixel(97, Color(0,0,0));
  leds.show();
  leds.setPixel(96, Color(0,0,0));
  leds.setPixel(95, Color(0,0,0));
  leds.show();
  leds.setPixel(94, Color(0,0,0));
  leds.setPixel(93, Color(0,0,0));
  leds.show();
  leds.setPixel(92, Color(0,0,0));
  leds.setPixel(91, Color(0,0,0));
  leds.show();
  leds.setPixel(90, Color(0,0,0));
  leds.setPixel(89, Color(0,0,0));
  leds.show();
  leds.setPixel(88, Color(0,0,0));
  leds.setPixel(87, Color(0,0,0));
  leds.show();
  leds.setPixel(86, Color(0,0,0));
  leds.setPixel(85, Color(0,0,0));
  leds.show();
  leds.setPixel(84, Color(0,0,0));
  leds.setPixel(83, Color(0,0,0));
  leds.show();
  leds.setPixel(82, Color(0,0,0));
  leds.setPixel(81, Color(0,0,0));
  leds.show();
  leds.setPixel(80, Color(0,0,0));
  leds.setPixel(79, Color(0,0,0));
  leds.show();
  leds.setPixel(78, Color(0,0,0));
  leds.setPixel(77, Color(0,0,0));
  leds.show();
  leds.setPixel(76, Color(0,0,0));
  leds.setPixel(75, Color(0,0,0));
  leds.show();
  leds.setPixel(74, Color(0,0,0));
  leds.setPixel(73, Color(0,0,0));
  leds.show();
  leds.setPixel(72, Color(0,0,0));
  leds.setPixel(71, Color(0,0,0));
  leds.show();
  leds.setPixel(70, Color(0,0,0));
  leds.setPixel(69, Color(0,0,0));
  leds.show();
  leds.setPixel(68, Color(0,0,0));
  leds.setPixel(67, Color(0,0,0));
  leds.show();
  leds.setPixel(66, Color(0,0,0));
  leds.setPixel(65, Color(0,0,0));
  leds.show();
  leds.setPixel(64, Color(0,0,0));
  leds.setPixel(63, Color(0,0,0));
  leds.show();
  leds.setPixel(62, Color(0,0,0));
  leds.setPixel(61, Color(0,0,0));
  leds.show();
  leds.setPixel(60, Color(0,0,0));
  leds.setPixel(59, Color(0,0,0));
  leds.show();
  leds.setPixel(58, Color(0,0,0));
  leds.setPixel(57, Color(0,0,0));
  leds.show();
  leds.setPixel(56, Color(0,0,0));
  leds.setPixel(55, Color(0,0,0));
  leds.show();
  leds.setPixel(54, Color(0,0,0));
  leds.setPixel(53, Color(0,0,0));
  leds.show();
  leds.setPixel(52, Color(0,0,0));
  leds.setPixel(51, Color(0,0,0));
  leds.show();
  leds.setPixel(50, Color(0,0,0));
  leds.setPixel(49, Color(0,0,0));
  leds.show();
  leds.setPixel(48, Color(0,0,0));
  leds.setPixel(47, Color(0,0,0));
  leds.show();
  leds.setPixel(46, Color(0,0,0));
  leds.setPixel(45, Color(0,0,0));
  leds.show();
  leds.setPixel(44, Color(0,0,0));
  leds.setPixel(43, Color(0,0,0));
  leds.show();
  leds.setPixel(42, Color(0,0,0));
  leds.setPixel(41, Color(0,0,0));
  leds.show();
  leds.setPixel(40, Color(0,0,0));
  leds.setPixel(39, Color(0,0,0));
  leds.show();
  leds.setPixel(38, Color(0,0,0));
  leds.setPixel(37, Color(0,0,0));
  leds.show();
  leds.setPixel(36, Color(0,0,0));
  leds.setPixel(35, Color(0,0,0));
  leds.show();
  leds.setPixel(34, Color(0,0,0));
  leds.setPixel(33, Color(0,0,0));
  leds.show();
  leds.setPixel(32, Color(0,0,0));
  leds.setPixel(31, Color(0,0,0));
  leds.show();
  leds.setPixel(30, Color(0,0,0));
  leds.setPixel(29, Color(0,0,0));
  leds.show();
  leds.setPixel(28, Color(0,0,0));
  leds.setPixel(27, Color(0,0,0));
  leds.show();
  leds.setPixel(26, Color(0,0,0));
  leds.setPixel(25, Color(0,0,0));
  leds.show();
  leds.setPixel(24, Color(0,0,0));
  leds.setPixel(23, Color(0,0,0));
  leds.show();
  leds.setPixel(22, Color(0,0,0));
  leds.setPixel(21, Color(0,0,0));
  leds.show();
  leds.setPixel(20, Color(0,0,0));
  leds.setPixel(19, Color(0,0,0));
  leds.show();
  leds.setPixel(18, Color(0,0,0));
  leds.setPixel(17, Color(0,0,0));
  leds.show();
  leds.setPixel(16, Color(0,0,0));
  leds.setPixel(15, Color(0,0,0));
  leds.show();
  leds.setPixel(14, Color(0,0,0));
  leds.setPixel(13, Color(0,0,0));
  leds.show();
  leds.setPixel(12, Color(0,0,0));
  leds.setPixel(11, Color(0,0,0));
  leds.show();
  leds.setPixel(10, Color(0,0,0));
  leds.setPixel(9, Color(0,0,0));
  leds.show();
  leds.setPixel(8, Color(0,0,0));
  leds.setPixel(7, Color(0,0,0));
  leds.show();
  leds.setPixel(6, Color(0,0,0));
  leds.setPixel(5, Color(0,0,0));
  leds.show();
  leds.setPixel(4, Color(0,0,0));
  leds.setPixel(3, Color(0,0,0));
  leds.show();
  leds.setPixel(2, Color(0,0,0));
  leds.setPixel(1, Color(0,0,0));
  leds.show();
  leds.setPixel(0, Color(0,0,0));
  leds.setPixel(-1, Color(0,0,0));
  leds.show();
  leds.setPixel(-2, Color(0,0,0));
  leds.setPixel(-3, Color(0,0,0));
  leds.show();
  
  delay(250);
  playSdWav1.stop();
  playSdWav2.stop();
  function = 0;
}


/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/


void batteryTest()
{
  int batteryRed;
  int batteryGreen;
  int batteryBlue;
  int batteryLevel;

  int mv = analogRead(A13);
  batteryLevel = map(mv, 555, 610, 10, 126);
  batteryLevel = constrain(batteryLevel, 10,126);

  //Serial.println(mv);

  for (int x = 0; x <= batteryLevel; x++)
  {
    if (x <= 30)
    {
      batteryRed = 255;
      batteryGreen = 0;
      batteryBlue = 0;
    }
    if (x > 30 && x <=54)
    {
      batteryRed = 255;
      batteryGreen = 40;
      batteryBlue = 0;
    }
    if (x > 54 && x <=78)
    {
      batteryRed = 255;
      batteryGreen = 255;
      batteryBlue = 0;
    }
    if (x > 78 && x <= 102)
    {
      batteryRed = 153;
      batteryGreen = 255;
      batteryBlue = 0;
    }
    if (x > 102)
    {
      batteryRed = 0;
      batteryGreen = 255;
      batteryBlue = 0;
    }
    leds.setPixel(x, Color(batteryGreen, batteryBlue, batteryRed));
    for (int x = batteryLevel+1; x <= NUMPIXELS; x++)
    {
      leds.setPixel(x, Color(0, 0, 0));
    }
  }
  leds.show();
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void spark()
{
  while (digitalRead(COLORBUTTON) == 0)
  {
    lsm.read();  /* ask it to read in the data */ 
    sensors_event_t a, m, g, temp;
    lsm.getEvent(&a, &m, &g, &temp);

    if (a.acceleration.y <= -7)
    {
      tipSpark();
    }

    else sparker();

    int sound = random(0,7);

    switch(sound)
    {
      case 0:
        playSdWav1.play("CLASH1.WAV");
        break;
      case 1:
        playSdWav1.play("CLASH2.WAV");
        break;
      case 2:
        playSdWav1.play("CLASH3.WAV");
        break;
      case 3:
        playSdWav1.play("CLASH4.WAV");
        break;
      case 4:
        playSdWav1.play("CLASH5.WAV");
        break;
      case 5:
        playSdWav1.play("CLASH6.WAV");
        break;
      case 6:
        playSdWav1.play("CLASH7.WAV");
        break;
    }

    
  }
}

/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void tipSpark()
{
  int randomDelay = random(10,40);
  int height = random(120, 125);
  for (int x = height; x <=140; x++)
  {
    leds.setPixel(x, Color(100,100,100));
  }
  leds.show();
  delay(randomDelay);



  leds.setPixel(1, Color(g,b,r));
  leds.setPixel(2, Color(g,b,r));
  leds.setPixel(3, Color(g,b,r));
  leds.setPixel(4, Color(g,b,r));
  leds.setPixel(5, Color(g,b,r));
  leds.setPixel(6, Color(g,b,r));
  leds.setPixel(7, Color(g,b,r));
  leds.setPixel(8, Color(g,b,r));
  leds.setPixel(9, Color(g,b,r));
  leds.setPixel(10, Color(g,b,r));
  leds.setPixel(11, Color(g,b,r));
  leds.setPixel(12, Color(g,b,r));
  leds.setPixel(13, Color(g,b,r));
  leds.setPixel(14, Color(g,b,r));
  leds.setPixel(15, Color(g,b,r));
  leds.setPixel(16, Color(g,b,r));
  leds.setPixel(17, Color(g,b,r));
  leds.setPixel(18, Color(g,b,r));
  leds.setPixel(19, Color(g,b,r));
  leds.setPixel(20, Color(g,b,r));
  leds.setPixel(21, Color(g,b,r));
  leds.setPixel(22, Color(g,b,r));
  leds.setPixel(23, Color(g,b,r));
  leds.setPixel(24, Color(g,b,r));
  leds.setPixel(25, Color(g,b,r));
  leds.setPixel(26, Color(g,b,r));
  leds.setPixel(27, Color(g,b,r));
  leds.setPixel(28, Color(g,b,r));
  leds.setPixel(29, Color(g,b,r));
  leds.setPixel(30, Color(g,b,r));
  leds.setPixel(31, Color(g,b,r));
  leds.setPixel(32, Color(g,b,r));
  leds.setPixel(33, Color(g,b,r));
  leds.setPixel(34, Color(g,b,r));
  leds.setPixel(35, Color(g,b,r));
  leds.setPixel(36, Color(g,b,r));
  leds.setPixel(37, Color(g,b,r));
  leds.setPixel(38, Color(g,b,r));
  leds.setPixel(39, Color(g,b,r));
  leds.setPixel(40, Color(g,b,r));
  leds.setPixel(41, Color(g,b,r));
  leds.setPixel(42, Color(g,b,r));
  leds.setPixel(43, Color(g,b,r));
  leds.setPixel(44, Color(g,b,r));
  leds.setPixel(45, Color(g,b,r));
  leds.setPixel(46, Color(g,b,r));
  leds.setPixel(47, Color(g,b,r));
  leds.setPixel(48, Color(g,b,r));
  leds.setPixel(49, Color(g,b,r));
  leds.setPixel(50, Color(g,b,r));
  leds.setPixel(51, Color(g,b,r));
  leds.setPixel(52, Color(g,b,r));
  leds.setPixel(53, Color(g,b,r));
  leds.setPixel(54, Color(g,b,r));
  leds.setPixel(55, Color(g,b,r));
  leds.setPixel(56, Color(g,b,r));
  leds.setPixel(57, Color(g,b,r));
  leds.setPixel(58, Color(g,b,r));
  leds.setPixel(59, Color(g,b,r));
  leds.setPixel(60, Color(g,b,r));
  leds.setPixel(61, Color(g,b,r));
  leds.setPixel(62, Color(g,b,r));
  leds.setPixel(63, Color(g,b,r));
  leds.setPixel(64, Color(g,b,r));
  leds.setPixel(65, Color(g,b,r));
  leds.setPixel(66, Color(g,b,r));
  leds.setPixel(67, Color(g,b,r));
  leds.setPixel(68, Color(g,b,r));
  leds.setPixel(69, Color(g,b,r));
  leds.setPixel(70, Color(g,b,r));
  leds.setPixel(71, Color(g,b,r));
  leds.setPixel(72, Color(g,b,r));
  leds.setPixel(73, Color(g,b,r));
  leds.setPixel(74, Color(g,b,r));
  leds.setPixel(75, Color(g,b,r));
  leds.setPixel(76, Color(g,b,r));
  leds.setPixel(77, Color(g,b,r));
  leds.setPixel(78, Color(g,b,r));
  leds.setPixel(79, Color(g,b,r));
  leds.setPixel(80, Color(g,b,r));
  leds.setPixel(81, Color(g,b,r));
  leds.setPixel(82, Color(g,b,r));
  leds.setPixel(83, Color(g,b,r));
  leds.setPixel(84, Color(g,b,r));
  leds.setPixel(85, Color(g,b,r));
  leds.setPixel(86, Color(g,b,r));
  leds.setPixel(87, Color(g,b,r));
  leds.setPixel(88, Color(g,b,r));
  leds.setPixel(89, Color(g,b,r));
  leds.setPixel(90, Color(g,b,r));
  leds.setPixel(91, Color(g,b,r));
  leds.setPixel(92, Color(g,b,r));
  leds.setPixel(93, Color(g,b,r));
  leds.setPixel(94, Color(g,b,r));
  leds.setPixel(95, Color(g,b,r));
  leds.setPixel(96, Color(g,b,r));
  leds.setPixel(97, Color(g,b,r));
  leds.setPixel(98, Color(g,b,r));
  leds.setPixel(99, Color(g,b,r));
  leds.setPixel(100, Color(g,b,r));
  leds.setPixel(101, Color(g,b,r));
  leds.setPixel(102, Color(g,b,r));
  leds.setPixel(103, Color(g,b,r));
  leds.setPixel(104, Color(g,b,r));
  leds.setPixel(105, Color(g,b,r));
  leds.setPixel(106, Color(g,b,r));
  leds.setPixel(107, Color(g,b,r));
  leds.setPixel(108, Color(g,b,r));
  leds.setPixel(109, Color(g,b,r));
  leds.setPixel(110, Color(g,b,r));
  leds.setPixel(111, Color(g,b,r));
  leds.setPixel(112, Color(g,b,r));
  leds.setPixel(113, Color(g,b,r));
  leds.setPixel(114, Color(g,b,r));
  leds.setPixel(115, Color(g,b,r));
  leds.setPixel(116, Color(g,b,r));
  leds.setPixel(117, Color(g,b,r));
  leds.setPixel(118, Color(g,b,r));
  leds.setPixel(119, Color(g,b,r));
  leds.setPixel(120, Color(g,b,r));
  leds.setPixel(121, Color(g,b,r));
  leds.setPixel(122, Color(g,b,r));
  leds.setPixel(123, Color(g,b,r));
  leds.setPixel(124, Color(g,b,r));
  leds.setPixel(125, Color(g,b,r));
  leds.setPixel(126, Color(g,b,r));
  leds.setPixel(127, Color(g,b,r));
  leds.setPixel(128, Color(g,b,r));
  leds.setPixel(129, Color(g,b,r));
  leds.setPixel(130, Color(g,b,r));
  leds.setPixel(131, Color(g,b,r));
  leds.setPixel(132, Color(g,b,r));
  leds.setPixel(133, Color(g,b,r));
  leds.setPixel(134, Color(g,b,r));
  leds.setPixel(135, Color(g,b,r));
  leds.setPixel(136, Color(g,b,r));
  leds.setPixel(137, Color(g,b,r));
  leds.setPixel(138, Color(g,b,r));
  leds.setPixel(139, Color(g,b,r));
  leds.setPixel(140, Color(g,b,r));
  leds.show();
}



/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/

void sparker()
{

  int randomDelay = random(10,40);
  leds.setPixel(1, Color(100,100,100));
  leds.setPixel(2, Color(100,100,100));
  leds.setPixel(3, Color(100,100,100));
  leds.setPixel(4, Color(100,100,100));
  leds.setPixel(5, Color(100,100,100));
  leds.setPixel(6, Color(100,100,100));
  leds.setPixel(7, Color(100,100,100));
  leds.setPixel(8, Color(100,100,100));
  leds.setPixel(9, Color(100,100,100));
  leds.setPixel(10, Color(100,100,100));
  leds.setPixel(11, Color(100,100,100));
  leds.setPixel(12, Color(100,100,100));
  leds.setPixel(13, Color(100,100,100));
  leds.setPixel(14, Color(100,100,100));
  leds.setPixel(15, Color(100,100,100));
  leds.setPixel(16, Color(100,100,100));
  leds.setPixel(17, Color(100,100,100));
  leds.setPixel(18, Color(100,100,100));
  leds.setPixel(19, Color(100,100,100));
  leds.setPixel(20, Color(100,100,100));
  leds.setPixel(21, Color(100,100,100));
  leds.setPixel(22, Color(100,100,100));
  leds.setPixel(23, Color(100,100,100));
  leds.setPixel(24, Color(100,100,100));
  leds.setPixel(25, Color(100,100,100));
  leds.setPixel(26, Color(100,100,100));
  leds.setPixel(27, Color(100,100,100));
  leds.setPixel(28, Color(100,100,100));
  leds.setPixel(29, Color(100,100,100));
  leds.setPixel(30, Color(100,100,100));
  leds.setPixel(31, Color(100,100,100));
  leds.setPixel(32, Color(100,100,100));
  leds.setPixel(33, Color(100,100,100));
  leds.setPixel(34, Color(100,100,100));
  leds.setPixel(35, Color(100,100,100));
  leds.setPixel(36, Color(100,100,100));
  leds.setPixel(37, Color(100,100,100));
  leds.setPixel(38, Color(100,100,100));
  leds.setPixel(39, Color(100,100,100));
  leds.setPixel(40, Color(100,100,100));
  leds.setPixel(41, Color(100,100,100));
  leds.setPixel(42, Color(100,100,100));
  leds.setPixel(43, Color(100,100,100));
  leds.setPixel(44, Color(100,100,100));
  leds.setPixel(45, Color(100,100,100));
  leds.setPixel(46, Color(100,100,100));
  leds.setPixel(47, Color(100,100,100));
  leds.setPixel(48, Color(100,100,100));
  leds.setPixel(49, Color(100,100,100));
  leds.setPixel(50, Color(100,100,100));
  leds.setPixel(51, Color(100,100,100));
  leds.setPixel(52, Color(100,100,100));
  leds.setPixel(53, Color(100,100,100));
  leds.setPixel(54, Color(100,100,100));
  leds.setPixel(55, Color(100,100,100));
  leds.setPixel(56, Color(100,100,100));
  leds.setPixel(57, Color(100,100,100));
  leds.setPixel(58, Color(100,100,100));
  leds.setPixel(59, Color(100,100,100));
  leds.setPixel(60, Color(100,100,100));
  leds.setPixel(61, Color(100,100,100));
  leds.setPixel(62, Color(100,100,100));
  leds.setPixel(63, Color(100,100,100));
  leds.setPixel(64, Color(100,100,100));
  leds.setPixel(65, Color(100,100,100));
  leds.setPixel(66, Color(100,100,100));
  leds.setPixel(67, Color(100,100,100));
  leds.setPixel(68, Color(100,100,100));
  leds.setPixel(69, Color(100,100,100));
  leds.setPixel(70, Color(100,100,100));
  leds.setPixel(71, Color(100,100,100));
  leds.setPixel(72, Color(100,100,100));
  leds.setPixel(73, Color(100,100,100));
  leds.setPixel(74, Color(100,100,100));
  leds.setPixel(75, Color(100,100,100));
  leds.setPixel(76, Color(100,100,100));
  leds.setPixel(77, Color(100,100,100));
  leds.setPixel(78, Color(100,100,100));
  leds.setPixel(79, Color(100,100,100));
  leds.setPixel(80, Color(100,100,100));
  leds.setPixel(81, Color(100,100,100));
  leds.setPixel(82, Color(100,100,100));
  leds.setPixel(83, Color(100,100,100));
  leds.setPixel(84, Color(100,100,100));
  leds.setPixel(85, Color(100,100,100));
  leds.setPixel(86, Color(100,100,100));
  leds.setPixel(87, Color(100,100,100));
  leds.setPixel(88, Color(100,100,100));
  leds.setPixel(89, Color(100,100,100));
  leds.setPixel(90, Color(100,100,100));
  leds.setPixel(91, Color(100,100,100));
  leds.setPixel(92, Color(100,100,100));
  leds.setPixel(93, Color(100,100,100));
  leds.setPixel(94, Color(100,100,100));
  leds.setPixel(95, Color(100,100,100));
  leds.setPixel(96, Color(100,100,100));
  leds.setPixel(97, Color(100,100,100));
  leds.setPixel(98, Color(100,100,100));
  leds.setPixel(99, Color(100,100,100));
  leds.setPixel(100, Color(100,100,100));
  leds.setPixel(101, Color(100,100,100));
  leds.setPixel(102, Color(100,100,100));
  leds.setPixel(103, Color(100,100,100));
  leds.setPixel(104, Color(100,100,100));
  leds.setPixel(105, Color(100,100,100));
  leds.setPixel(106, Color(100,100,100));
  leds.setPixel(107, Color(100,100,100));
  leds.setPixel(108, Color(100,100,100));
  leds.setPixel(109, Color(100,100,100));
  leds.setPixel(110, Color(100,100,100));
  leds.setPixel(111, Color(100,100,100));
  leds.setPixel(112, Color(100,100,100));
  leds.setPixel(113, Color(100,100,100));
  leds.setPixel(114, Color(100,100,100));
  leds.setPixel(115, Color(100,100,100));
  leds.setPixel(116, Color(100,100,100));
  leds.setPixel(117, Color(100,100,100));
  leds.setPixel(118, Color(100,100,100));
  leds.setPixel(119, Color(100,100,100));
  leds.setPixel(120, Color(100,100,100));
  leds.setPixel(121, Color(100,100,100));
  leds.setPixel(122, Color(100,100,100));
  leds.setPixel(123, Color(100,100,100));
  leds.setPixel(124, Color(100,100,100));
  leds.setPixel(125, Color(100,100,100));
  leds.setPixel(126, Color(100,100,100));
  leds.setPixel(127, Color(100,100,100));
  leds.setPixel(128, Color(100,100,100));
  leds.setPixel(129, Color(100,100,100));
  leds.setPixel(130, Color(100,100,100));
  leds.setPixel(131, Color(100,100,100));
  leds.setPixel(132, Color(100,100,100));
  leds.setPixel(133, Color(100,100,100));
  leds.setPixel(134, Color(100,100,100));
  leds.setPixel(135, Color(100,100,100));
  leds.setPixel(136, Color(100,100,100));
  leds.setPixel(137, Color(100,100,100));
  leds.setPixel(138, Color(100,100,100));
  leds.setPixel(139, Color(100,100,100));
  leds.setPixel(140, Color(100,100,100));
  leds.show();
  delay(randomDelay);


  leds.setPixel(1, Color(g,b,r));
  leds.setPixel(2, Color(g,b,r));
  leds.setPixel(3, Color(g,b,r));
  leds.setPixel(4, Color(g,b,r));
  leds.setPixel(5, Color(g,b,r));
  leds.setPixel(6, Color(g,b,r));
  leds.setPixel(7, Color(g,b,r));
  leds.setPixel(8, Color(g,b,r));
  leds.setPixel(9, Color(g,b,r));
  leds.setPixel(10, Color(g,b,r));
  leds.setPixel(11, Color(g,b,r));
  leds.setPixel(12, Color(g,b,r));
  leds.setPixel(13, Color(g,b,r));
  leds.setPixel(14, Color(g,b,r));
  leds.setPixel(15, Color(g,b,r));
  leds.setPixel(16, Color(g,b,r));
  leds.setPixel(17, Color(g,b,r));
  leds.setPixel(18, Color(g,b,r));
  leds.setPixel(19, Color(g,b,r));
  leds.setPixel(20, Color(g,b,r));
  leds.setPixel(21, Color(g,b,r));
  leds.setPixel(22, Color(g,b,r));
  leds.setPixel(23, Color(g,b,r));
  leds.setPixel(24, Color(g,b,r));
  leds.setPixel(25, Color(g,b,r));
  leds.setPixel(26, Color(g,b,r));
  leds.setPixel(27, Color(g,b,r));
  leds.setPixel(28, Color(g,b,r));
  leds.setPixel(29, Color(g,b,r));
  leds.setPixel(30, Color(g,b,r));
  leds.setPixel(31, Color(g,b,r));
  leds.setPixel(32, Color(g,b,r));
  leds.setPixel(33, Color(g,b,r));
  leds.setPixel(34, Color(g,b,r));
  leds.setPixel(35, Color(g,b,r));
  leds.setPixel(36, Color(g,b,r));
  leds.setPixel(37, Color(g,b,r));
  leds.setPixel(38, Color(g,b,r));
  leds.setPixel(39, Color(g,b,r));
  leds.setPixel(40, Color(g,b,r));
  leds.setPixel(41, Color(g,b,r));
  leds.setPixel(42, Color(g,b,r));
  leds.setPixel(43, Color(g,b,r));
  leds.setPixel(44, Color(g,b,r));
  leds.setPixel(45, Color(g,b,r));
  leds.setPixel(46, Color(g,b,r));
  leds.setPixel(47, Color(g,b,r));
  leds.setPixel(48, Color(g,b,r));
  leds.setPixel(49, Color(g,b,r));
  leds.setPixel(50, Color(g,b,r));
  leds.setPixel(51, Color(g,b,r));
  leds.setPixel(52, Color(g,b,r));
  leds.setPixel(53, Color(g,b,r));
  leds.setPixel(54, Color(g,b,r));
  leds.setPixel(55, Color(g,b,r));
  leds.setPixel(56, Color(g,b,r));
  leds.setPixel(57, Color(g,b,r));
  leds.setPixel(58, Color(g,b,r));
  leds.setPixel(59, Color(g,b,r));
  leds.setPixel(60, Color(g,b,r));
  leds.setPixel(61, Color(g,b,r));
  leds.setPixel(62, Color(g,b,r));
  leds.setPixel(63, Color(g,b,r));
  leds.setPixel(64, Color(g,b,r));
  leds.setPixel(65, Color(g,b,r));
  leds.setPixel(66, Color(g,b,r));
  leds.setPixel(67, Color(g,b,r));
  leds.setPixel(68, Color(g,b,r));
  leds.setPixel(69, Color(g,b,r));
  leds.setPixel(70, Color(g,b,r));
  leds.setPixel(71, Color(g,b,r));
  leds.setPixel(72, Color(g,b,r));
  leds.setPixel(73, Color(g,b,r));
  leds.setPixel(74, Color(g,b,r));
  leds.setPixel(75, Color(g,b,r));
  leds.setPixel(76, Color(g,b,r));
  leds.setPixel(77, Color(g,b,r));
  leds.setPixel(78, Color(g,b,r));
  leds.setPixel(79, Color(g,b,r));
  leds.setPixel(80, Color(g,b,r));
  leds.setPixel(81, Color(g,b,r));
  leds.setPixel(82, Color(g,b,r));
  leds.setPixel(83, Color(g,b,r));
  leds.setPixel(84, Color(g,b,r));
  leds.setPixel(85, Color(g,b,r));
  leds.setPixel(86, Color(g,b,r));
  leds.setPixel(87, Color(g,b,r));
  leds.setPixel(88, Color(g,b,r));
  leds.setPixel(89, Color(g,b,r));
  leds.setPixel(90, Color(g,b,r));
  leds.setPixel(91, Color(g,b,r));
  leds.setPixel(92, Color(g,b,r));
  leds.setPixel(93, Color(g,b,r));
  leds.setPixel(94, Color(g,b,r));
  leds.setPixel(95, Color(g,b,r));
  leds.setPixel(96, Color(g,b,r));
  leds.setPixel(97, Color(g,b,r));
  leds.setPixel(98, Color(g,b,r));
  leds.setPixel(99, Color(g,b,r));
  leds.setPixel(100, Color(g,b,r));
  leds.setPixel(101, Color(g,b,r));
  leds.setPixel(102, Color(g,b,r));
  leds.setPixel(103, Color(g,b,r));
  leds.setPixel(104, Color(g,b,r));
  leds.setPixel(105, Color(g,b,r));
  leds.setPixel(106, Color(g,b,r));
  leds.setPixel(107, Color(g,b,r));
  leds.setPixel(108, Color(g,b,r));
  leds.setPixel(109, Color(g,b,r));
  leds.setPixel(110, Color(g,b,r));
  leds.setPixel(111, Color(g,b,r));
  leds.setPixel(112, Color(g,b,r));
  leds.setPixel(113, Color(g,b,r));
  leds.setPixel(114, Color(g,b,r));
  leds.setPixel(115, Color(g,b,r));
  leds.setPixel(116, Color(g,b,r));
  leds.setPixel(117, Color(g,b,r));
  leds.setPixel(118, Color(g,b,r));
  leds.setPixel(119, Color(g,b,r));
  leds.setPixel(120, Color(g,b,r));
  leds.setPixel(121, Color(g,b,r));
  leds.setPixel(122, Color(g,b,r));
  leds.setPixel(123, Color(g,b,r));
  leds.setPixel(124, Color(g,b,r));
  leds.setPixel(125, Color(g,b,r));
  leds.setPixel(126, Color(g,b,r));
  leds.setPixel(127, Color(g,b,r));
  leds.setPixel(128, Color(g,b,r));
  leds.setPixel(129, Color(g,b,r));
  leds.setPixel(130, Color(g,b,r));
  leds.setPixel(131, Color(g,b,r));
  leds.setPixel(132, Color(g,b,r));
  leds.setPixel(133, Color(g,b,r));
  leds.setPixel(134, Color(g,b,r));
  leds.setPixel(135, Color(g,b,r));
  leds.setPixel(136, Color(g,b,r));
  leds.setPixel(137, Color(g,b,r));
  leds.setPixel(138, Color(g,b,r));
  leds.setPixel(139, Color(g,b,r));
  leds.setPixel(140, Color(g,b,r));
  leds.show();
  delay(randomDelay);
}

