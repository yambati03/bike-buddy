#include <Wire.h>

#define G_MSS 9.81
#define M_PI 3.14159
#define BUFF_SIZE 80


float RateRoll, RatePitch, RateYaw;
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch;

void gyro_signals(void) {
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);

  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(0x68);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);

  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();

  RateRoll = (float)GyroX / 131;
  RatePitch = (float)GyroY / 131;
  RateYaw = (float)GyroZ / 131;

  AccX = -(float)AccXLSB / 16384;
  AccY = -(float)AccYLSB / 16384;
  AccZ = -(float)AccZLSB / 16384;

  AngleRoll = -atan(AccY / sqrt(AccX * AccX + AccZ * AccZ));
  AnglePitch = atan(AccZ / sqrt(AccY * AccY + AccX * AccX));
}

void setup() {
  Serial.begin(57600);
  while (!Serial) { ; }
  Wire.setClock(400000);
  Wire.begin();
  delay(250);

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
}

void loop() {
  gyro_signals();

  Serial.print(millis());
  Serial.print(", ");
  Serial.print(RateRoll);
  Serial.print(", ");
  Serial.print(RatePitch);
  Serial.print(", ");
  Serial.print(RateYaw);
  Serial.print(", ");
  Serial.print(AccX);
  Serial.print(", ");
  Serial.print(AccY);
  Serial.print(", ");
  Serial.println(AccZ);
}
