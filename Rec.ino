/*
  Arduino NRF24L01 simple transmit and receive - Created October 2017
    by DonX Developer
  *** This code was created for an Arduino UNO using the Arduino IDE v1.8.4. ***

  Parts:
  2 X Arduinos
  2 X Breadboards
  2 X NRF24L01
  [optional] 2 X YL-105 breakout boards for the NRF24L01. This allows for 5v connection and easier wiring
  2 X Momentary switches
  2 X Red LEDs
  2 X Yellow LEDs
  4 X 220 ohm resistors
  Jumper wires


  NRF24L01  Arduino pin
  VCC       3.3 V
  GND       GND
  CS        8
  CE        7
  MOSI      11 or ICSP-4
  MISO      12 or ICSP-1
  SCK       13 or ICSP-3

  If you use the YL-105 breakout board, the Vcc lead can go to the 5v Arduino pin

  Arduino pin 2 to Yellow LED long lead - anode
  Yellow short lead - cathode to 220ohm resistor, then second resistor lead to GND

  Arduino pin 3 to Red LED long lead - anode
  Red short lead - cathode to 220ohm resistor, then second resistor lead to GND

  Arduino pin 4 to switch, other side of switch to GND

  The physical build of the two boards are identical
  There are minor software differences for the reading and writing pipes for the respective boards.

*/

#include <SPI.h>
#include "RF24.h"

#define button 3
#define confirmLed 4
#define led 5

RF24 NRF24L01 (8, 9);//create object called NRF24L01. specifying the CE and CSN pins to be used on the Arduino

byte address[] [6] = {"pipe1", "pipe2"};//set addresses of the 2 pipes for read and write
boolean buttonState = false;//used for both transmission and receive

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(confirmLed, OUTPUT);//yellow LED
  pinMode(led, OUTPUT);//red LED

  NRF24L01.begin();

  //open the pipes to read and write from board 1
  NRF24L01.openWritingPipe(address[0]);//open writing pipe to address pipe 1
  NRF24L01.openReadingPipe(1, address[1]);//open reading pipe from address pipe 2

  NRF24L01.setPALevel(RF24_PA_MAX);//set RF power output to minimum, RF24_PA_MIN (change to RF24_PA_MAX if required)
  NRF24L01.setDataRate(RF24_250KBPS);//set data rate to 250kbps
  NRF24L01.setChannel(110);//set frequency to channel 110
}

void loop() {
  //Transmit button change to the other Arduino
  delay(10);
  NRF24L01.stopListening();
  buttonState = digitalRead(button);//test for button press on this board
  if (buttonState == LOW)//button is pulled up so test for LOW
  {
    NRF24L01.write(&buttonState, sizeof(buttonState));//send LOW state to other Arduino board
    //flash the yellow LED to show progress
    digitalWrite(confirmLed, HIGH);
    delay(100);
    digitalWrite(confirmLed, LOW);
  }

  buttonState = HIGH;//reset the button state variable

  //Receive button change from the other Arduino
  delay(10);
  NRF24L01.startListening();
  if (NRF24L01.available())//do we have transmission from other Arduino board
  {
    NRF24L01.read(&buttonState, sizeof(buttonState));//update the variable with new state
    NRF24L01.stopListening();
  }
    else {
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
  }
  if (buttonState == HIGH)//test the other Arduino's button state
  {
    digitalWrite(led, LOW);
  }
  else
  {
    flashLed();//indicate that the button was pressed on the other board
  }
  buttonState = HIGH;//reset the button state variable
}

//flash the red LED five times
void flashLed()
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
  }

}

