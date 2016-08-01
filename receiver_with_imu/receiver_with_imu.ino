#include <Thread.h>
#include <ThreadController.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <string.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

void threadSettings(void);
void IMUData(void);
void joystickData(void);
void mainFunction(void);
void showData(void);

int msg[1]; //vector that holds data from joystick
RF24 radio(9,10); //object RF24
const uint64_t pipe = 0xE8E8F0F0E1LL; //comunication pipe

Thread* IMU = new Thread(); //thread for imu
Thread* joystick = new Thread(); //thread for joystick
ThreadController mainThread = ThreadController(); //main thread

//Object MPU6050
MPU6050 accelgyro;

//These variables are used to calculate the values of accelerometer and gyroscope (X,Y,Z)
int16_t ax, ay, az;
int16_t gx, gy, gz;

//These variables are used calculate the accelerometer angle
double accXangle;
double accYangle;
double accZangle;

//These variables are used calculate the gyroscope angle
double gyroXangle = 180;
double gyroYangle = 180;
double gyroZangle = 180;

//This variable stores the execution time
uint32_t timer;

String theMessage = ""; //message from another arduino

//setting up things
void setup() {
  Serial.begin(38400);
  Wire.begin();
  Serial.flush();
  radio.begin(); //starting NRF24L01
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  timer  = micros();
  threadSettings();
}

//call mainFcuntion()
void loop() {
  mainFunction();
}

/**
 * Show the gyro and accel angles and the values 
 * from the potenciometer attached on another arduino
 */
void showData() {
  Serial.print("Potenciometer: ");
  Serial.print(theMessage);
  Serial.print("|");
  Serial.print("Gx: "); Serial.print(gyroXangle); Serial.print("|");
  Serial.print("Gy: "); Serial.print(gyroYangle); Serial.print("|");
  Serial.print("Gz: "); Serial.print(gyroZangle); Serial.print("|");
  Serial.print("Ax: "); Serial.print(accXangle); Serial.print("|");
  Serial.print("Ay: "); Serial.print(accYangle); Serial.print("|");
  Serial.print("Az: "); Serial.print(accZangle); Serial.println("|");
}

/**
 * The main function
 */
void mainFunction() {
  while(true) {
    mainThread.run();
    showData();
  }
}

/**
 * Get values from gyro and accel
 */
void IMUData() {
  // reading MPU6050
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // getting angles with accelerometer
  accXangle = (atan2(ax,az) + PI) * RAD_TO_DEG;
  accYangle = (atan2(ay,az) + PI) * RAD_TO_DEG;
  accZangle = (atan2(ax,ay) + PI) * RAD_TO_DEG;  

  double gyroXrate = ((double)gx / 131.0);
  double gyroYrate = -((double)gy / 131.0);
  double gyroZrate = -((double)gz / 131.0);
  
  //getting the angle of rotation without any filter 
  gyroXangle += gyroXrate * ((double)(micros() - timer)/1000000); 
  gyroYangle += gyroYrate * ((double)(micros() - timer)/1000000);
  gyroZangle += gyroZrate * ((double)(micros() - timer)/1000000);
 
  timer = micros();
  
  //The maximum rate of the accelerometer samples is 1kHz
  delay(1);
}

/**
 * Get the data from the other Arduino board
 */
void joystickData() {
  if(radio.available()) {
    bool done = false;
    done = radio.read(msg, 1);
    char theChar= msg[0];
    if(msg[0] != 2) 
      theMessage.concat(theChar);
    theMessage = "";
  }
}

/**
 * Setting the threads 
 */
void threadSettings() {
  IMU->onRun(IMUData);
  IMU->setInterval(500);
  mainThread.add(IMU);
  
  joystick->onRun(joystickData);
  joystick->setInterval(500);
  mainThread.add(joystick);
}
