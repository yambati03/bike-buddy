#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define LED_COUNT  16      // Number of LEDs in the ring

Adafruit_NeoPixel ring1(LED_COUNT, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring2(LED_COUNT, 10, NEO_GRB + NEO_KHZ800);

float RateRoll, RatePitch, RateYaw;
float AccX, AccY, AccZ;
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

int JOY_X_PIN = A0;
int JOY_Y_PIN = A1;


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
  while (!Serial) { ; }
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

void loop() {
  JoyState joy_state = get_joy_state();


  /*
  State Machine


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

  updateBlinking();
}

void updateBlinking() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkIntervalMillis) {
    previousMillis = currentMillis;
    isBlinkOn = !isBlinkOn; 

    if (state == LEFT_BLINK) {
      // make sure to turn off right LED;
      ring2.clear();
      ring2.show();
      if (isBlinkOn) {
        ring1.fill(ring1.Color(1, 0, 0), 0, LED_COUNT); 
      } else {
        ring1.clear(); 
      }
      ring1.show();
    } else if (state == RIGHT_BLINK) {
      // make sure to turn off left LED;
      ring1.clear();
      ring1.show();
      if (isBlinkOn) {
        ring2.fill(ring2.Color(1, 0, 0), 0, LED_COUNT);
      } else {
        ring2.clear();
      }
      ring2.show();
    } else {
      // turn off both rings if not in a blinking state
      ring1.clear();
      ring2.clear();
      ring1.show();
      ring2.show();
    }
  }


  // gyro_signals();
  // Serial.print("Acceleration X [g]= ");
  // Serial.print(AccX);
  // Serial.print(" Acceleration Y [g]= ");
  // Serial.print(AccY);
  // Serial.print(" Acceleration Z [g]= ");
  // Serial.println(AccZ);

  

  

  // if (AccY < 0) {
  //   ring1.fill(ring1.Color(255, 0, 0), 0, LED_COUNT);
  // } else {
  //   ring1.clear();
  // // }
  // ring1.show();

  // delay(50);
}
