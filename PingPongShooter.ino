#include <Servo.h>
#define BLYNK_TEMPLATE_ID "TMPL2ypdmfabu"
#define BLYNK_TEMPLATE_NAME "PingPongMachine"
#define BLYNK_AUTH_TOKEN "WiSbOO7KNTGptcXm_EUKI7Wxb3E4PmMp"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
// #include <FreeRTOSConfig.h>
// #include <Arduino_FreeRTOS.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Aarush iPhone";
char pass[] = "aarush000";
//char ssid[] = "Lightning_2G";
//char pass[] = "QcRwZHcSysLh9V9";


#define BOTTOM_DC 12
#define LEFT_DC 14
#define RIGHT_DC 13
#define IN1_DC_PIN 5
#define IN2_DC_PIN 4
#define SPINNER_DC 16
#define TOP_SERVO 15
#define BOTTOM_SERVO 0

Servo bottom_servo;
Servo top_servo;

int left_speed = 0;
int right_speed = 0;
int bottom_speed = 0;

int left_previous_speed = 0;
int right_previous_speed = 0;
int bottom_previous_speed = 0;

int speed = 0;
int top_angle;
int sweep = 0;
int sweep_type = 0;

int top_spin_speed = 0;
int back_spin_speed = 0;
int side_spin_speed = 0;

BLYNK_WRITE(V0) { //Spinner_Test
  int spinner_motor_speed = param.asInt();
  analogWrite(SPINNER_DC, spinner_motor_speed);
}
BLYNK_WRITE(V1) { //Bottom_Servo_Test
  int bottom_servo_angle = param.asInt();
  bottom_servo.write(bottom_servo_angle);
  Blynk.virtualWrite(V15, bottom_servo_angle);
}
BLYNK_WRITE(V2) { //Top_Servo_Test
  int top_servo_angle = param.asInt();
  top_servo.write(top_servo_angle);
  Blynk.virtualWrite(V14, top_servo_angle);
}
BLYNK_WRITE(V3) { //DC_Test
  int all_motor_speed = param.asInt();
  run_and_slow(BOTTOM_DC, all_motor_speed, bottom_previous_speed);
  run_and_slow(LEFT_DC, all_motor_speed, left_previous_speed);
  run_and_slow(RIGHT_DC, all_motor_speed, right_previous_speed);
  bottom_previous_speed = all_motor_speed;
  left_previous_speed = all_motor_speed;
  right_previous_speed = all_motor_speed;
}
BLYNK_WRITE(V4) { //Left_Motor_Test
  int left_motor_speed = param.asInt();
  run_and_slow(LEFT_DC, left_motor_speed, left_previous_speed);
  left_previous_speed = left_motor_speed;
}
BLYNK_WRITE(V5) { //Bottom_Motor_Test
  int bottom_motor_speed = param.asInt();
  run_and_slow(BOTTOM_DC, bottom_motor_speed, bottom_previous_speed);
  bottom_previous_speed = bottom_motor_speed;
}
BLYNK_WRITE(V6) { //Right_Motor_Test
  int right_motor_speed = param.asInt();
  run_and_slow(RIGHT_DC, right_motor_speed, right_previous_speed);
  right_previous_speed = right_motor_speed;
}
BLYNK_WRITE(V7) { //Speed
  speed = param.asInt();
  left_speed = param.asInt();
  bottom_speed = param.asInt();
  right_speed = param.asInt();
  top_angle = 43.03 + (33981940 - 43.03) / pow((1 + (speed / 0.96)),3.26);
  top_servo.write(top_angle);
  Blynk.virtualWrite(V14, top_angle);
  if (back_spin_speed > 0 && top_spin_speed == 0){
    back_spin(back_spin_speed);
    side_spin(side_spin_speed);
  } else if (back_spin_speed == 0 && top_spin_speed > 0){
    top_spin(top_spin_speed);
    side_spin(side_spin_speed);
  } else {
    forward(left_speed, LEFT_DC);
    forward(bottom_speed, BOTTOM_DC);
    forward(right_speed, RIGHT_DC);
    side_spin(side_spin_speed);
  }
}
BLYNK_WRITE(V8) { //Top_Spin
  top_spin_speed = param.asInt();
  top_spin(top_spin_speed);
}
BLYNK_WRITE(V9) { //Back_Spin
  back_spin_speed = param.asInt();
  back_spin(back_spin_speed);
}
BLYNK_WRITE(V10) { //Side_Spin
  side_spin_speed = param.asInt();
  side_spin(side_spin_speed);
  // resetSpeed();
}
BLYNK_WRITE(V16) { //Location
  int location = param.asInt();
  if (location == 0){
    bottom_servo.write(101);
    Blynk.virtualWrite(V15, 101);
  } else if (location == 1){
    bottom_servo.write(88);
    Blynk.virtualWrite(V15, 88);
  } else if (location == 2){
    bottom_servo.write(75);
    Blynk.virtualWrite(V15, 75);
  } else {}
}
BLYNK_WRITE(V17) { //Sweep_Type
  sweep_type = param.asInt();
  if (sweep_type == 0){
    sweep = 0;
    bottom_servo.write(88);
    Blynk.virtualWrite(V15, 88);
  } else {
    sweep = 1;
  }
}
BLYNK_WRITE(V18) { //Stop
  int stop = param.asInt();
  if (stop == 1){
    forward(0, LEFT_DC);
    forward(0, BOTTOM_DC);
    forward(0, RIGHT_DC);
    analogWrite(SPINNER_DC, 0);
    sweep = 0;
    top_servo.write(0);
    bottom_servo.write(88);
    Blynk.virtualWrite(V14, 0);
    Blynk.virtualWrite(V15, 88);
    Blynk.virtualWrite(V18, 0);
  }
}

void side_spin (int side_spin_speed){
  int new_right_speed = right_speed;
  int new_left_speed = left_speed;
  int new_bottom_speed = bottom_speed;
  if (side_spin_speed < 0){
    new_right_speed = right_speed + side_spin_speed;
    new_left_speed = left_speed - (side_spin_speed / 2);
    new_bottom_speed  = bottom_speed + side_spin_speed;
    new_right_speed = new_right_speed + ((-side_spin_speed / 5) * 2);
    new_left_speed = new_left_speed + ((-side_spin_speed / 5) * 2);
    new_bottom_speed = new_bottom_speed + ((-side_spin_speed / 5) * 2);
  } else if (side_spin_speed > 0){
    new_right_speed = right_speed + (side_spin_speed / 2);
    new_left_speed = left_speed - side_spin_speed;
    new_bottom_speed  = bottom_speed - side_spin_speed;
    new_right_speed = new_right_speed + ((side_spin_speed / 5) * 2);
    new_left_speed = new_left_speed + ((side_spin_speed / 5) * 2);
    new_bottom_speed = new_bottom_speed + ((side_spin_speed / 5) * 2);
  } 
  forward(new_right_speed, RIGHT_DC);
  forward(new_left_speed, LEFT_DC);
  forward(new_bottom_speed, BOTTOM_DC);
  if (speed >= 70 && speed <= 90 && side_spin_speed > 0){
    top_servo.write(top_angle + 5);
    Blynk.virtualWrite(V14, top_angle + 5);
  } else {
    top_servo.write(top_angle);
    Blynk.virtualWrite(V14, top_angle);
  }
}
void top_spin (int top_spin_speed){
  resetSpeed();
  right_speed = right_speed + top_spin_speed * 2;
  left_speed = left_speed + top_spin_speed * 2;
  bottom_speed  = bottom_speed - top_spin_speed;
  forward(right_speed, RIGHT_DC);
  forward(left_speed, LEFT_DC);
  forward(bottom_speed, BOTTOM_DC);
}
void back_spin (int back_spin_speed){
  resetSpeed();
  right_speed = right_speed - back_spin_speed;
  left_speed = left_speed - back_spin_speed;
  bottom_speed  = bottom_speed + (back_spin_speed / 2);
  forward(right_speed, RIGHT_DC);
  forward(left_speed, LEFT_DC);
  forward(bottom_speed, BOTTOM_DC);
  if (speed >= 80 && speed <= 90 && back_spin_speed > 0){
    top_servo.write(top_angle + 5);
    Blynk.virtualWrite(V14, top_angle + 5);
  } else {
    top_servo.write(top_angle);
    Blynk.virtualWrite(V14, top_angle);
  }
}
void run_and_slow (int motor_pin, int value, int previous_speed)
{
  if (value > 10){
    forward (value, motor_pin);
  } else {
    brake(motor_pin, previous_speed);
  }
}
void forward (int pwm_rate, int motor_pin)
{
  digitalWrite(IN1_DC_PIN, LOW);
  digitalWrite(IN2_DC_PIN, HIGH);
  analogWrite(motor_pin, pwm_rate);
  updateShooterGuage(motor_pin, pwm_rate);
}
void reverse (int pwm_rate, int motor_pin)
{
  digitalWrite(IN1_DC_PIN, HIGH);
  digitalWrite(IN2_DC_PIN, LOW);
  analogWrite(motor_pin, pwm_rate);
  updateShooterGuage(motor_pin, pwm_rate);
}
void brake (int motor_pin, int previous_speed)
{
  reverse (255, motor_pin);
  if (previous_speed >= 50){
    int brake_delay = 0.344865 * previous_speed + 194.622;
    delay (brake_delay);
      Serial.println("Slowing Down");
  }
  analogWrite(motor_pin, 0);
  updateShooterGuage(motor_pin, 0);
}
void updateShooterGuage (int motor_pin, int pwm_rate){
  if (motor_pin == 14){
    Blynk.virtualWrite(V11, pwm_rate);
  } else if (motor_pin == 12){
    Blynk.virtualWrite(V12, pwm_rate);
  } else if (motor_pin == 13){
    Blynk.virtualWrite(V13, pwm_rate);
  }
}
void resetSpeed(){
  right_speed = speed;
  left_speed = speed;
  bottom_speed = speed;
}
void randomSweep (){
  // Sweep back from low bound to high bound
    for (int angle = 75; angle <= 101; angle++) {
      bottom_servo.write(angle);
      delay(15);  // Adjust the delay for speed control
      if (random(100) < 5) {  // 5% chance for random events
        int randomEvent = random(3);
        if (randomEvent == 0) {
        delay(random(100, 300)); 
        } else if (randomEvent == 1) {
        delay(random(300, 600));  
        } else {
        delay(random(600, 900));  
        }
      }
    }
    // Sweep back from high bound to low bound
    for (int angle = 101; angle >= 75; angle--) {
      bottom_servo.write(angle);
      delay(15);  // Adjust the delay for speed control
      if (random(100) < 5) {  // 5% chance for random events
        int randomEvent = random(3);
        if (randomEvent == 0) {
          delay(random(100, 300));  
        } else if (randomEvent == 1) {
          delay(random(300, 600));  
        } else {
          delay(random(600, 900));  
        }
      }
    }
}
void normalSweep (){
  // Sweep back from low bound to high bound
    for (int angle = 75; angle <= 101; angle++) {
      bottom_servo.write(angle);
      delay(20);  // Adjust the delay for speed control
    }
    // Sweep back from high bound to low bound
    for (int angle = 101; angle >= 75; angle--) {
      bottom_servo.write(angle);
      delay(20);  // Adjust the delay for speed control
    }
}

void setup() {
  Serial.begin(19200);
  Blynk.begin(auth, ssid, pass);

  pinMode(IN1_DC_PIN, OUTPUT);
  pinMode(IN2_DC_PIN, OUTPUT);
  pinMode(BOTTOM_DC, OUTPUT);
  pinMode(LEFT_DC, OUTPUT);
  pinMode(RIGHT_DC, OUTPUT);
  pinMode(SPINNER_DC, OUTPUT);

  forward (0, BOTTOM_DC); 
  forward (0, LEFT_DC); 
  forward (0, RIGHT_DC); 
  forward (0, SPINNER_DC); 
  analogWriteRange(255);

  pinMode(LED_BUILTIN, OUTPUT);
  analogWrite(LED_BUILTIN, HIGH); 

  bottom_servo.attach(BOTTOM_SERVO, 500, 2400);
  top_servo.attach(TOP_SERVO, 500, 2400);
  
  top_servo.write(0);
  bottom_servo.write(88);
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V1, 88);
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V4, 0);
  Blynk.virtualWrite(V5, 0);
  Blynk.virtualWrite(V6, 0);
  Blynk.virtualWrite(V7, 65);
  Blynk.virtualWrite(V8, 0);
  Blynk.virtualWrite(V9, 0);
  Blynk.virtualWrite(V10, 0);
  Blynk.virtualWrite(V11, 0);
  Blynk.virtualWrite(V12, 0);
  Blynk.virtualWrite(V13, 0);
  Blynk.virtualWrite(V14, 0);
  Blynk.virtualWrite(V15, 88);
  Blynk.virtualWrite(V16, 0);
  Blynk.virtualWrite(V17, 0);
  Blynk.virtualWrite(V18, 0);
}

void loop() {
  if (sweep == 1){
    if (sweep_type == 1){
      randomSweep ();
    } else if (sweep_type == 2){
      normalSweep ();
    }
  } 
  Blynk.run();
}
