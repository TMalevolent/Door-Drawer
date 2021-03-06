//this version of the code is used for testing the Door with the Arduino Serial Monitor.

// Include the necessary libraries
#include "math.h" //Includes math library
//#include "Adafruit_VL53L0X.h" //library for time of flight sensor (tof)
//#include <Wire.h>
#include <AS5047P.h>

//define pins for motor controller
#define enable_motor_channel 6 //pwm pin
#define motor_channel3  48
#define motor_channel4  42

// define pins for relay/electromagnet
#define relay25 35
#define relay35 33
#define relay45 44

// Initialize FSR and Potentiometer in door frame
#define fsr_13 A0
#define fsr_14 A3
#define fsr_1 A7 
#define fsr_2 A15 
#define fsr_3 A10
#define fsr_4 A4
#define fsr_5 A12
#define fsr_6 A5
#define fsr_7 A6
#define fsr_8 A13
#define fsr_9 A8
#define fsr_10 A14 
#define fsr_11 A9 
#define fsr_12 A11 

//define slave signal pin for as5047p
#define slave_port 9

//defines custom spi bus speed for as5047p (Hz). Max is 10000000Hz (10MHz)
#define bus_speed 100000 //1MHz

//tof sensor object
//Adafruit_VL53L0X tof = Adafruit_VL53L0X();

//AS5047P sensor object
AS5047P as5047p(slave_port, bus_speed);
float door_angle;
float start_pos;

// Initialize user input variables
float magnet_input;
float time_input;
char junk = ' ';
unsigned long time;
unsigned long stoptime;

//declare motor variable for time
const int time_unwind = 2000; // in ms

void setup() {
  // Initialize the Serial monitor
  //Wire.begin();
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  /*//don't run door if TOF is not working
  if (!tof.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }*/
  if (!as5047p.initSPI()){
    Serial.println("as5047p (magnetic potentiometer) failed to connect");
    while (!as5047p.initSPI()){
      ;
    }
  }
  // initialize motor pins as outputs
  pinMode(motor_channel3, OUTPUT);
  pinMode(motor_channel4, OUTPUT);
  pinMode(enable_motor_channel, OUTPUT);
  // Initializes electromagnet relay pins as outputs
  pinMode(relay25, OUTPUT);
  pinMode(relay35, OUTPUT);
  pinMode(relay45, OUTPUT);
  //initialize fsr's
  pinMode(fsr_1, INPUT);
  pinMode(fsr_2, INPUT);
  pinMode(fsr_3, INPUT);
  pinMode(fsr_4, INPUT);
  pinMode(fsr_5, INPUT);
  pinMode(fsr_6, INPUT);
  pinMode(fsr_7, INPUT);
  pinMode(fsr_8, INPUT);
  pinMode(fsr_9, INPUT);
  pinMode(fsr_10, INPUT);
  pinMode(fsr_11, INPUT);
  pinMode(fsr_12, INPUT);
  pinMode(fsr_13, INPUT);
  pinMode(fsr_14, INPUT);
}

void loop() {
  //Temporary Interface for Door testing purposes.
  get_magnet_val();
  get_trial_length();

  Enable_Relays(magnet_input); // changes electromagnets based on input
  bool switched_off = false;
  bool switched_on = true;
  start_pos = as5047p.readAngleDegree();
  time = millis();
  stoptime = time + (time_input * 1000); // converts time_input to seconds
  while (time < stoptime) {
    //read_TOF_val();
    read_angle();
    Serial.print(" -- ");
    read_handle_val();
    Serial.print(" -- ");
    read_pull_force();
    Serial.print(" -- ");
    time = millis();
    Serial.println(time);
  }
  Enable_Relays(0);
  Reset_Door(); // automatically close door
}

void Reset_Door() {
  bool did_move = false;
  int motor_speed = 100; //max speed for the motor
  analogWrite(enable_motor_channel, motor_speed); //turns motor on
  digitalWrite(motor_channel3, LOW);// turns motor
  digitalWrite(motor_channel4, HIGH); // counter clockwise
  while (true) { // door is open more than 5 degrees
    /*VL53L0X_RangingMeasurementData_t measure; //value from tof sensor
    tof.rangingTest(&measure, false);
    door_angle = calc_degree(measure.RangeMilliMeter);*/
    door_angle = start_pos - as5047p.readAngleDegree();
    Serial.print("Angle of the door during closing: "); Serial.println(door_angle);
    if (door_angle < 3) {
      break;
    }
    did_move = true;
    //delay(100);
  }
  if (did_move) {
    digitalWrite(motor_channel3, HIGH); // turns motor
    digitalWrite(motor_channel4, LOW);// clockwise
    delay(time_unwind); //run for certain amount of time
  }
  analogWrite(enable_motor_channel, 0); // turns motor off
}

void Enable_Relays(int user_in) {
  int relay25_val = 0; //for testing purposes
  int delay_time = 1; //1 works
  if (user_in == 0) {
    digitalWrite(relay25, relay25_val);
    delay(delay_time);
    digitalWrite(relay35, LOW);
    delay(delay_time);
    digitalWrite(relay45, LOW);
  }
  else if (user_in == 1) {
    digitalWrite(relay25, (relay25_val + 1));
    delay(delay_time);
    digitalWrite(relay35, LOW);
    delay(delay_time);
    digitalWrite(relay45, LOW);
  }
  else if (user_in == 2) {
    digitalWrite(relay25, relay25_val);
    delay(delay_time);
    digitalWrite(relay35, HIGH);
    delay(delay_time);
    digitalWrite(relay45, LOW);
  }
  else if (user_in == 3) {
    digitalWrite(relay25, relay25_val);
    delay(delay_time);
    digitalWrite(relay35, LOW);
    delay(delay_time);
    digitalWrite(relay45, HIGH);
  }
  else if (user_in == 4) {
    digitalWrite(relay25, (relay25_val + 1));
    delay(delay_time);
    digitalWrite(relay35, HIGH);
    delay(delay_time);
    digitalWrite(relay45, LOW);
  }
  else if (user_in == 5) {
    digitalWrite(relay25, (relay25_val + 1));
    delay(delay_time);
    digitalWrite(relay35, LOW);
    delay(delay_time);
    digitalWrite(relay45, HIGH);
  }
  else if (user_in == 6) {
    digitalWrite(relay25, relay25_val);
    delay(delay_time);
    digitalWrite(relay35, HIGH);
    delay(delay_time);
    digitalWrite(relay45, HIGH);
  }
  else if (user_in == 7) {
    digitalWrite(relay25, (relay25_val + 1));
    delay(delay_time);
    digitalWrite(relay35, HIGH);
    delay(delay_time);
    digitalWrite(relay45, HIGH);
  }
}

float calc_degree(int distance) {
  float D0 = 289; // value of distance from tof when door is closed
  float D1 = 418; //value of distance from tof when door is fully open
  return (distance - D0) / ((D1 - D0) / 90); // 90 is max degree door can open from closed position
}

void get_magnet_val() {
  Serial.print("Enter desired force rating on a scale of 0 to 7: ");
  while (Serial.available() == 0); {
    magnet_input = Serial.parseFloat();
    Serial.print(magnet_input, 0);
    Serial.print("\n");
    while (Serial.available() > 0) {
      junk = Serial.read();
    }
  }
}

void get_trial_length() {
  Serial.print("Enter How long you would like to run each test (in seconds): ");
  while (Serial.available() == 0); {
    time_input = Serial.parseFloat();
    Serial.print(time_input, 0);
    Serial.print("\n");
    while (Serial.available() > 0) {
      junk = Serial.read();
    }
  }
}

void read_handle_val() {
  Serial.print(analogRead(fsr_1));
  Serial.print(analogRead(fsr_2));
  Serial.print(analogRead(fsr_3));
  Serial.print(analogRead(fsr_4));
  Serial.print(analogRead(fsr_5));
  Serial.print(analogRead(fsr_6));
  Serial.print(analogRead(fsr_7));
  Serial.print(analogRead(fsr_8));
  Serial.print(analogRead(fsr_9));
  Serial.print(analogRead(fsr_10));
  Serial.print(analogRead(fsr_11));
  Serial.print(analogRead(fsr_12));
}

void read_pull_force() {
  Serial.print(analogRead(fsr_13));
  Serial.print(analogRead(fsr_14));
}

void read_angle() {
  //0deg is greatest value in our case
  door_angle = start_pos - as5047p.readAngleDegree();
  if (door_angle < 0) {
    door_angle = 0;
  }
  Serial.print(door_angle);
}

/*void read_TOF_val() {
  VL53L0X_RangingMeasurementData_t measure; //value from tof sensor. pointer
  tof.rangingTest(&measure, false);
  door_angle = calc_degree(measure.RangeMilliMeter);
  if (door_angle < 0) {
    door_angle = 0;
  }
  Serial.println(door_angle);
  //Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter); //used for calibrating tof sensor
}*/
