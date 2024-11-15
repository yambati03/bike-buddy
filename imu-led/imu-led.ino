#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define LED_COUNT  16      // Number of LEDs in the ring
#define G_MSS 9.81
#define M_PI 3.14159

Adafruit_NeoPixel left_ring(LED_COUNT, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel right_ring(LED_COUNT, 10, NEO_GRB + NEO_KHZ800);

double accx_buffer[100] = {0.0};
int accx_buffer_index = 0;

float RateRoll, RatePitch, RateYaw;
float AccX, AccY, AccZ;
float AccX_no_gravity, AccY_no_gravity, AccZ_no_gravity;
float AngleRoll, AnglePitch;
float LoopTimer;

enum JoyState {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  CENTER
};

enum SystemState {
  LEFT_BLINK,
  RIGHT_BLINK,
  IDLE
};

SystemState state = IDLE;

int JOY_X_PIN = A15;
int JOY_Y_PIN = A14;

uint32_t no_color = left_ring.Color(0, 0, 0);
uint32_t dim_yellow = left_ring.Color(10, 10, 0);


int LEFT_BLINK_START = 8;
int LEFT_BLINK_END = 16;
int RIGHT_BLINK_START = 0;
int RIGHT_BLINK_END = 8;

int LEFT_BRAKE_START = 0;
int LEFT_BRAKE_END = 8;
int RIGHT_BRAKE_START = 8;
int RIGHT_BRAKE_END = 16;


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

  AccX=-(float)AccXLSB/4096;
  AccY=-(float)AccYLSB/4096;
  AccZ=(float)AccZLSB/4096;

  AngleRoll=-atan(AccY/sqrt(AccX*AccX+AccZ*AccZ));
  AnglePitch=atan(AccX/sqrt(AccY*AccY+AccZ*AccZ));

  AccX_no_gravity = AccX - sin(AnglePitch); // AccX - sin(AnglePitch);
  AccY_no_gravity = AccY + sin(AngleRoll) * cos(AnglePitch);
  AccZ_no_gravity = AccZ - cos(AngleRoll) * cos(AnglePitch);

  AccX_no_gravity = AccX_no_gravity * G_MSS; // AccX_no_gravity * G_MSS;
  AccY_no_gravity = AccY_no_gravity * G_MSS;
  AccZ_no_gravity = AccZ_no_gravity * G_MSS;
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

  left_ring.begin();
  right_ring.begin();  
}

JoyState get_joy_state() {
  // read joystick values and get next state.
  int joy_x = analogRead(JOY_X_PIN);
  int joy_y = analogRead(JOY_Y_PIN);
  int x_diff = abs(joy_x - 524);
  int y_diff = abs(joy_y - 524);


  // check if there is movement in the X direction.
  if (x_diff > y_diff){
    if (joy_x < 100) {
      return LEFT;
    } else if (joy_x > 900) {
      return RIGHT;
    }
  } else {
    if (joy_y < 100) {
      return UP;
    } else if (joy_y > 800) {
      return DOWN;
    }
  }
  return CENTER;
}



unsigned long previousMillis = 0; 
const long blinkIntervalMillis = 500; // in mills
bool isBlinkOn = false;  
bool turnStarted = false;
unsigned long turnTime = millis();

void loop() {
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

  switch (state) {
    case IDLE:
      if (joy_state == LEFT) {
        state = LEFT_BLINK;
        turnStarted = false;
      } else if (joy_state == RIGHT) {
        state = RIGHT_BLINK;
      }
      break;

    case LEFT_BLINK:
      if (joy_state == DOWN) {
        state = IDLE;
      } else if (joy_state == RIGHT) {
        state = RIGHT_BLINK;
      }
      break;

    case RIGHT_BLINK:
      if (joy_state == DOWN) {
        state = IDLE;
      } else if (joy_state == LEFT) {
        state = LEFT_BLINK;
      }
      break;
  }

  Serial.println(RateYaw);

  if (state == LEFT_BLINK) {

    if (turnStarted) {
      if (millis() - turnTime > 250 && abs(RateYaw) < 100) {
        state = IDLE;
      }
    }

    if (abs(RateYaw) > 100) {
      turnStarted = true;
      turnTime = millis();
    }

    // record gyro values

  
  }

  updateTurnSignals();
  // Serial.println(AccX_no_gravity);

  accx_buffer[accx_buffer_index] = AccX_no_gravity;
  accx_buffer_index = (accx_buffer_index + 1) % 100;

  double sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += accx_buffer[i];
  }
  double avg = sum / 100;

  Serial.println(avg);

  if (avg < -1) {
    updateBrakeLights(abs(avg / 9.81));
  } else {
    updateBrakeLights(0.0);
  }


    // if (AccX_no_gravity < -1) {
  //   updateBrakeLights(abs(AccX_no_gravity / 9.81));
  // } else {
  //   updateBrakeLights(0.0);
// }
}

// pass in ring 

void set_light(Adafruit_NeoPixel& light, int start, int end, uint32_t color) {
  light.fill(color, start, end);
  light.show();
}

void updateTurnSignals() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkIntervalMillis) {
    previousMillis = currentMillis;
    isBlinkOn = !isBlinkOn; 

    if (state == LEFT_BLINK) {
      set_light(right_ring, RIGHT_BLINK_START, RIGHT_BLINK_END, no_color);
      set_light(left_ring, LEFT_BLINK_START, LEFT_BLINK_END, isBlinkOn ? dim_yellow : no_color);
    } else if (state == RIGHT_BLINK) {
      // turn off left turn signal
      set_light(left_ring, LEFT_BLINK_START, LEFT_BLINK_END, no_color);
      set_light(right_ring, RIGHT_BLINK_START, RIGHT_BLINK_END, isBlinkOn ? dim_yellow : no_color);
    } else {
      set_light(right_ring, RIGHT_BLINK_START, RIGHT_BLINK_END, no_color);
      set_light(left_ring, LEFT_BLINK_START, LEFT_BLINK_END, no_color);
    }
  }

}

void updateBrakeLights(double light_intensity) {
  uint32_t color = left_ring.Color(255 * light_intensity, 0, 0);
  set_light(left_ring, LEFT_BRAKE_START, LEFT_BRAKE_END, color);
  set_light(right_ring, RIGHT_BRAKE_START, RIGHT_BRAKE_END, color);
}

