#include <Wire.h>
#include <MPU6050_tockn.h>
#include <BleMouse.h>

// MPU6050 GPIO pins
#define SDA_PIN 14
#define SCL_PIN 13

// Button pins
#define RESET_BUTTON 27
#define UP_BUTTON 26
#define LEFT_BUTTON 25
#define RIGHT_BUTTON 33
#define DOWN_BUTTON 32

// init MPU
MPU6050 mpu6050(Wire);
float gx, gy, gz; // gyroscope for x, y, z

// how far to turn in order to register movement
// ignores small movement/noise
const int16_t deadZone = 10; 
const float sensitivity = 0.5; // mouse sensitivity

// init blemouse
BleMouse bleMouse("Gyro Mouse", "Gorav", 100);

// vars
const int debounce = 20;
bool left_hold = false; // for drag / drag select
const int CenterX = 960;
const int CenterY = 540;

// check if between +/-127 and return num inrange of +/-127
int betweenNums(int num){
  if (num >= 127){
    num = 127;
  } else if(num <= -127){
    num = -127;
  } else{
    num = num;
  }
  return num;
}

// bleMouse.move() only moves within +/-127 pixels
// move cursor by a specific amount of pixels
void cursorMove(int dx, int dy){
  while (dx != 0 || dy != 0){
    int stepx = betweenNums(dx);
    int stepy = betweenNums(dy);
    bleMouse.move(stepx, stepy);
    dx -= stepx;
    dy -= stepy;
    delay(debounce/4);
  }
}

// get values from mp6050
void getMotion(void){
  gx=mpu6050.getGyroX(); // get gyror x-axis
  gy=mpu6050.getGyroY(); // get gyroscoposcope fe for y-axis
  gz=mpu6050.getGyroZ(); // get gyroscope for z-axis
}

void setup() {
  Serial.begin(115200);

  // Button setups
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);

  // MPU setups
  Wire.begin(SDA_PIN, SCL_PIN);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  
  // blemouse setup
  bleMouse.begin();
}

void loop() {

  if (bleMouse.isConnected()){
    mpu6050.update();
    getMotion();
    // if movement is less than deadzone ingore it
    if(fabs(gx) < deadZone){gx = 0;}
    if(fabs(gy) < deadZone){gy = 0;}
    if(fabs(gz) < deadZone){gz = 0;}
    int x_movement = (int)(sensitivity * gx);
    int y_movement = (int)(sensitivity * gy);
    int z_movement = (int)(sensitivity * gz);
    bleMouse.move(betweenNums(-z_movement), betweenNums(-x_movement)); // yaw(-gz)-> x movement pitch(-gx)-> y movement
    
    // up button
    if (digitalRead(UP_BUTTON) == LOW){
      Serial.printf("Scroll Up\n");
      bleMouse.move(0, 0, 1, 0);
      delay(debounce*2);
    }

    // down button
    if (digitalRead(DOWN_BUTTON) == LOW){
      Serial.printf("Scroll Down\n");
      bleMouse.move(0, 0, -1, 0);
      delay(debounce*2);
    }

    // left click button
    if (digitalRead(LEFT_BUTTON) == LOW && !left_hold){
      delay(debounce);
      if (digitalRead(LEFT_BUTTON) == LOW){
        Serial.printf("LEFT CLICK\n");
        bleMouse.press(MOUSE_LEFT);
        left_hold = true;
        delay(debounce);
      }
    }

    // left hold
    if (digitalRead(LEFT_BUTTON) == HIGH && left_hold) {
      if (digitalRead(LEFT_BUTTON) == HIGH) {
        Serial.printf("LEFT HOLD\n");
        bleMouse.release(MOUSE_LEFT);
        left_hold = false;
        delay(debounce);
      }
    }


    // right button
    if (digitalRead(RIGHT_BUTTON) == LOW){
      delay(debounce);
      if (digitalRead(RIGHT_BUTTON) == LOW){
        Serial.printf("RIGHT CLICK\n");
        bleMouse.click(MOUSE_RIGHT);
        while (digitalRead(RIGHT_BUTTON) == LOW);
          delay(debounce);
      }
    }

    // reset button
    if (digitalRead(RESET_BUTTON) == LOW){
      delay(debounce);
      if (digitalRead(RESET_BUTTON) == LOW){
        Serial.printf("RESET BUTTON\n");
        // move to bottom right
        cursorMove(10000, 10000);
        cursorMove(-CenterX, -CenterY);
        while (digitalRead(RESET_BUTTON) == LOW);
          delay(debounce);
      }
    }

    delay(debounce/2);
  }
}
