#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define LED_COUNT 16 // Number of LEDs in the ring
#define G_MSS 9.81
#define M_PI 3.14159
#define BUFF_SIZE 100

Adafruit_NeoPixel left_ring(LED_COUNT, 37, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel right_ring(LED_COUNT, 39, NEO_GRB + NEO_KHZ800);

double accx_buffer[BUFF_SIZE] = {0.0};
double accy_buffer[BUFF_SIZE] = {0.0};
double accz_buffer[BUFF_SIZE] = {0.0};

int accx_buffer_index = 0;
int accy_buffer_index = 0;
int accz_buffer_index = 0;

float RateRoll, RatePitch, RateYaw;
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch;
float AccXSmoothed, AccYSmoothed, AccZSmoothed;

enum JoyState
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  CENTER
};

enum SystemState
{
  LEFT_BLINK,
  RIGHT_BLINK,
  IDLE
};

SystemState state = IDLE;

int JOY_X_PIN = A0;
int JOY_Y_PIN = A1;

uint32_t no_color = left_ring.Color(0, 0, 0);
uint32_t dim_yellow = left_ring.Color(255, 255, 0);

bool turnStarted = false;
unsigned long turnTime = millis();

int LEFT_BLINK_START = 2;
int LEFT_BLINK_END = 10;


int RIGHT_BLINK_START = 1;
int RIGHT_BLINK_END = 9;

int LEFT_BRAKE_START = 10;
int LEFT_BRAKE_END = 2;
int RIGHT_BRAKE_START = 9;
int RIGHT_BRAKE_END = 1;


int LEFT_STATUS_LED_PIN = 33;
int BRAKE_STATUS_LED_PIN = 31;
int RIGHT_STATUS_LED_PIN = 29;

double get_buffer_avg(double *buffer)
{
  double sum = 0;
  for (int i = 0; i < BUFF_SIZE; i++)
  {
    sum += buffer[i];
  }
  return sum / BUFF_SIZE;
}

void gyro_signals(void)
{
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

  RateRoll = (float)GyroX / 131.0;
  RatePitch = (float)GyroY / 131.0;
  RateYaw = (float)GyroZ / 131.0;

  AccX = (float)AccXLSB * G_MSS / 16384.0;
  AccY = (float)AccYLSB * G_MSS / 16384.0;
  AccZ = (float)AccZLSB * G_MSS / 16384.0;

  AngleRoll = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ));
  AnglePitch = atan(AccZ / sqrt(AccY * AccY + AccX * AccX));

  accx_buffer[accx_buffer_index] = AccX;
  accx_buffer_index = (accx_buffer_index + 1) % BUFF_SIZE;

  accy_buffer[accy_buffer_index] = AccY;
  accy_buffer_index = (accy_buffer_index + 1) % BUFF_SIZE;

  accz_buffer[accz_buffer_index] = AccZ;
  accz_buffer_index = (accz_buffer_index + 1) % BUFF_SIZE;

  AccXSmoothed = get_buffer_avg(accx_buffer);
  AccYSmoothed = get_buffer_avg(accy_buffer);
  AccZSmoothed = get_buffer_avg(accz_buffer);
}

void setup()
{
  Serial.begin(57600);
  while (!Serial)
  {
    continue;
  }
  Wire.setClock(400000);
  Wire.begin();
  delay(250);

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  left_ring.begin();
  right_ring.begin();

  pinMode(LEFT_STATUS_LED_PIN, OUTPUT);
  pinMode(RIGHT_STATUS_LED_PIN, OUTPUT);
  pinMode(BRAKE_STATUS_LED_PIN, OUTPUT);


}

JoyState get_joy_state()
{
  int joy_x = analogRead(JOY_X_PIN);
  int joy_y = analogRead(JOY_Y_PIN);
  int x_diff = abs(joy_x - 524);
  int y_diff = abs(joy_y - 524);

  if (x_diff > y_diff)
  {
    if (joy_x < 100)
    {
      return LEFT;
    }
    else if (joy_x > 900)
    {
      return RIGHT;
    }
  }
  else
  {
    if (joy_y < 100)
    {
      return UP;
    }
    else if (joy_y > 800)
    {
      return DOWN;
    }
  }
  return CENTER;
}

unsigned long previousMillis = 0;
const long blinkIntervalMillis = 500; // in mills
bool isBlinkOn = false;

void loop()
{
  JoyState joy_state = get_joy_state();
  gyro_signals();

  /*
  Transitions:
  -- Current System State, JoyState, Next System State
  - IDLE, LEFT, LEFT_BLINK;
  - IDLE, RIGHT, RIGHT_BLINK;
  - LEFT_BLINK, DOWN, IDLE;
  - LEFT_BLINK, RIGHT, RIGHT_BLINK,
  - RIGHT_BLINK, DOWN, IDLE;
  - RIGHT_BLINK, LEFT, LEFT_BLINK
  */

  switch (state)
  {
  case IDLE:
    if (joy_state == LEFT)
    {
      state = LEFT_BLINK;
    }
    else if (joy_state == RIGHT)
    {
      state = RIGHT_BLINK;
    }
    break;

  case LEFT_BLINK:
    if (joy_state == DOWN)
    {
      state = IDLE;
    }
    else if (joy_state == RIGHT)
    {
      state = RIGHT_BLINK;
    }
    break;

  case RIGHT_BLINK:
    if (joy_state == DOWN)
    {
      state = IDLE;
    }
    else if (joy_state == LEFT)
    {
      state = LEFT_BLINK;
    }
    break;
  }

  if (state == LEFT_BLINK || state == RIGHT_BLINK)
  {
    if (turnStarted)
    {
      if (millis() - turnTime > 250 && abs(RateRoll) < 25)
      {
        turnStarted = false;
        state = IDLE;
      }
    }

    if (RateRoll < -25 && state == LEFT_BLINK)
    {
      turnStarted = true;
      turnTime = millis();
    }

    if (RateRoll > 25 && state == RIGHT_BLINK)
    {
      turnStarted = true;
      turnTime = millis();
    }
  }

  updateTurnSignals();
  
  if (AccYSmoothed * cos(54 * M_PI / 180) + AccXSmoothed * sin(54 * M_PI / 180) < -1)
  {
    updateBrakeLights(1.0);
  }
  else
  {
    updateBrakeLights(0.0);
  }
}

void set_light(Adafruit_NeoPixel &light, int start, int end, uint32_t color)
{
  if (start > end) {

    int count = 16 - start;
    light.fill(color, start, count);
    light.fill(color, 0, end);
  } else {
    int count = end - start;
    light.fill(color, start, count);
  }
  

  light.show();
}

void updateTurnSignals()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkIntervalMillis)
  {
    previousMillis = currentMillis;
    isBlinkOn = !isBlinkOn;

    if (state == LEFT_BLINK)
    {
      set_light(right_ring, RIGHT_BLINK_START, RIGHT_BLINK_END, no_color);
      digitalWrite(RIGHT_STATUS_LED_PIN, 0);

      set_light(left_ring, LEFT_BLINK_START, LEFT_BLINK_END, isBlinkOn ? dim_yellow : no_color);
      digitalWrite(LEFT_STATUS_LED_PIN, isBlinkOn);
    }
    else if (state == RIGHT_BLINK)
    {
      set_light(left_ring, LEFT_BLINK_START, LEFT_BLINK_END, no_color);
      digitalWrite(LEFT_STATUS_LED_PIN, 0);

      set_light(right_ring, RIGHT_BLINK_START, RIGHT_BLINK_END, isBlinkOn ? dim_yellow : no_color);
      digitalWrite(RIGHT_STATUS_LED_PIN, isBlinkOn);

    }
    else
    {
      set_light(right_ring, RIGHT_BLINK_START, RIGHT_BLINK_END, no_color);
      set_light(left_ring, LEFT_BLINK_START, LEFT_BLINK_END, no_color);
      digitalWrite(LEFT_STATUS_LED_PIN, 0);
      digitalWrite(RIGHT_STATUS_LED_PIN, 0);


    }
  }
}

void updateBrakeLights(double light_intensity)
{
  uint32_t color = left_ring.Color(255 * light_intensity, 0, 0);
  set_light(left_ring, LEFT_BRAKE_START, LEFT_BRAKE_END, color);
  set_light(right_ring, RIGHT_BRAKE_START, RIGHT_BRAKE_END, color);
  digitalWrite(BRAKE_STATUS_LED_PIN, light_intensity > 0);
}
