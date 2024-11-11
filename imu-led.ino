#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define LED_COUNT  16      // Number of LEDs in the ring

Adafruit_NeoPixel ring1(LED_COUNT, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring2(LED_COUNT, 10, NEO_GRB + NEO_KHZ800);

float RateRoll, RatePitch, RateYaw;
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch;
float LoopTimer;

void gyro_signals(void) {
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);

  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(0x68);
  Wire.write(0x1B); 
  Wire.write(0x8);
  Wire.endTransmission();                                                   
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);

  int16_t GyroX=Wire.read()<<8 | Wire.read();
  int16_t GyroY=Wire.read()<<8 | Wire.read();
  int16_t GyroZ=Wire.read()<<8 | Wire.read();

  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;

  AccX=(float)AccXLSB/4096;
  AccY=(float)AccYLSB/4096;
  AccZ=(float)AccZLSB/4096;

  AngleRoll=atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
  AnglePitch=-atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
}

void setup() {
  Serial.begin(57600);
  Wire.setClock(400000);
  Wire.begin();
  delay(250);

  Wire.beginTransmission(0x68); 
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  ring1.begin();
  ring2.begin();  
}

void loop() {
  gyro_signals();
  Serial.print("Acceleration X [g]= ");
  Serial.print(AccX);
  Serial.print(" Acceleration Y [g]= ");
  Serial.print(AccY);
  Serial.print(" Acceleration Z [g]= ");
  Serial.println(AccZ);

  if (AccY < 0) {
    ring1.fill(ring1.Color(255, 0, 0), 0, LED_COUNT);
  } else {
    ring1.clear();
  }
  ring1.show();

  delay(50);
}