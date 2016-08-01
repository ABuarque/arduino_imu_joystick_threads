#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

void mainFunction(void);
int getPotenciometerData(void);
String intToString(int);
void sendData(String);

const byte POTENCIOMETER_PIN = A0; //potenciometer pin

int msg[1]; //msg vector
RF24 radio(9,10); //object nrf24l01
const uint64_t pipe = 0xE8E8F0F0E1LL;

//setting up things
void setup() {
  pinMode(POTENCIOMETER_PIN, INPUT_PULLUP);
  Serial.begin(38400);
  Serial.flush();
  
  radio.begin();
  radio.openWritingPipe(pipe);
}

//call the main function
void loop() {
  mainFunction();
}

/**
 * Get the values from potenciometer
 *
 * @return values from 0 to 1024
 */
int getPotenciometerData() {
  return analogRead(POTENCIOMETER_PIN);
}

/**
 * Parse int to String
 * 
 * @param data to parse
 * @return string
 */
String intToString(int data) {
  return String(data, DEC);
}

/**
 * Send the values from potenciometer
 */
void sendData(int data) {
  String dataToSend= intToString(data);
  radio.stopListening();
  int messageSize  = dataToSend.length();
  for(int i = 0; i < messageSize; i++) {
    int charToSend[1];
    charToSend[0] = dataToSend.charAt(i);
    radio.write(charToSend, 1);
  }
  msg[0] = 2;
  radio.write(msg,1);
  radio.powerDown(); 
  delay(1);
  radio.powerUp();
  radio.startListening();
}

/**
 * Send the values from potenciometer to another arduino
 */
void mainFunction() {
  sendData(getPotenciometerData());
}
