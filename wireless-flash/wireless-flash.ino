/*
 * Arduino RF24 Communication
 * This code enables two Arduino boards to communicate using NRF24L01 radio modules.
 * Each board can send and receive button press states to/from the other board.
 * Last updated 12/02/2025, Jazza
 * Licensed under GPLv3
 */

#include <SPI.h>
#include "RF24.h"

// Pin Definitions
const int BUZZER = 2;          // Buzzer for auditory feedback
const int BUTTON_PIN = 3;      // Input button pin
const int CONFIRM_LED_PIN = 4; // Yellow LED for transmission confirmation
const int STATUS_LED_PIN = 5;  // Red LED for received signal indication

// Radio Configuration
const byte RADIO_CE_PIN = 9;  // Radio CE pin
const byte RADIO_CSN_PIN = 8; // Radio CSN pin
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

// Communication Configuration
const byte ADDRESS[][6] = {"pipe1", "pipe2"}; // Radio addresses for two-way communication
const byte RADIO_CHANNEL = 110;               // Radio channel frequency
boolean buttonState = false;                  // Tracks button state for both TX and RX

// Timing Configuration
unsigned long lastStateChange = 0; // Last time the button state changed
boolean lastButtonState = HIGH;    // Previous button state

const int BOARD_NUMBER = 1;

void setup()
{
    // Initialize Pins
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Button with internal pull-up
    pinMode(CONFIRM_LED_PIN, OUTPUT);  // Yellow LED
    pinMode(STATUS_LED_PIN, OUTPUT);   // Red LED

    // Configure Radio
    setupRadio();
}

void loop()
{
    transmitButtonState();
    receiveButtonState();
}

/**
 * Configures the NRF24L01 radio module with required settings
 */
void setupRadio()
{
    radio.begin();

    // Set up communication pipes
    if (BOARD_NUMBER == 1)
    {
        radio.openWritingPipe(ADDRESS[0]); // Board 1 writes to pipe1
        radio.openReadingPipe(1, ADDRESS[1]);
    }
    else
    {
        radio.openReadingPipe(1, ADDRESS[0]);
        radio.openWritingPipe(ADDRESS[1]); // Board 2 writes to pipe2
    }

    // Configure radio parameters
    radio.setPALevel(RF24_PA_MAX);   // Set to maximum power
    radio.setDataRate(RF24_250KBPS); // Set data rate to 250kbps
    radio.setChannel(RADIO_CHANNEL); // Set radio channel
}

/**
 * Transmits the local button state to the other Arduino
 */
void transmitButtonState()
{
    radio.stopListening(); // Switch to transmission mode

    boolean currentButtonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();

    // Check if button state has changed
    if (currentButtonState != lastButtonState)
    {
        // Round to nearest ROUND_TIME
        lastStateChange = currentTime;
        lastButtonState = currentButtonState;
        buttonState = currentButtonState;

        // Transmit the new state
        radio.write(&buttonState, sizeof(buttonState));

        // Update local indicators
        if (buttonState == LOW)
        { // Button is pressed (LOW due to pull-up)
            digitalWrite(CONFIRM_LED_PIN, HIGH);
            tone(BUZZER, BOARD_NUMBER == 1 ? 600 : 1000);
        }
        else
        {
            digitalWrite(CONFIRM_LED_PIN, LOW);
            noTone(BUZZER);
        }
    }

    delay(5); // Small delay for stability
}

/**
 * Receives button state from the other Arduino
 */
void receiveButtonState()
{
    radio.startListening(); // Switch to receiving mode

    if (radio.available())
    {
        radio.read(&buttonState, sizeof(buttonState));

        // Update status LED based on received button state
        if (buttonState == HIGH)
        {
            digitalWrite(STATUS_LED_PIN, LOW);
            noTone(BUZZER);
        }
        else
        {
            digitalWrite(STATUS_LED_PIN, HIGH);
            tone(BUZZER, BOARD_NUMBER == 1 ? 600 : 900);
        }
    }

    delay(5); // Small delay for stability
}