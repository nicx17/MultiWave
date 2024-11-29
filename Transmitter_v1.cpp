#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

int value = 2;
RF24 radio(35, 33); // CE, CSN
const int NUM_PINS = 8;
const byte address[5] = {'0', '0', '0', '0', '1'};
char pinStatusString[NUM_PINS + 1] = "";
int leds[] = {12, 14, 16, 18, 20, 22, 24, 26};
int leds2[] = {13, 15, 17};
int Value = 0;
int buttonPin = 36;
int lastButtonState = LOW;

void setup() {
  radio.begin();
  Serial.begin(9600);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
pinMode(buttonPin, INPUT);
  // Set pins 2 to 10 (excluding pins 7 and 8) as inputs
  for (int i = 2; i <= 9; i++) {
    pinMode(i, INPUT);
  }

  // Set LED pins as outputs
  for (int i = 0; i < 8; i++) {
    pinMode(leds[i], OUTPUT);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(leds2[i], OUTPUT);
  }
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && lastButtonState == LOW) {
    // Button is pressed
    Value = (Value + 1) % 4; // Cycle value from 0 to 3
    // Perform any action based on the new cycleValue here
  }

  lastButtonState = buttonState;
 Serial.println(Value);
  // Create a string to store the status of pins
 readPins(pinStatusString, NUM_PINS);
if (Value == 0) {
     for (int i = 0; i <= 7; i++) {
    // Check if the character is '1', if so, turn on the corresponding LED
    
      digitalWrite(leds[i], LOW);
    }
    digitalWrite(leds2[0], LOW); // Light corresponding LED
    digitalWrite(leds2[1], LOW);  // Light corresponding LED
    digitalWrite(leds2[2], LOW);  // Light corresponding LED
  }
  if (Value == 1) {
      readPins(pinStatusString, NUM_PINS);
      for (int i = 0; i <= 7; i++) {
    // Check if the character is '1', if so, turn on the corresponding LED
    if (pinStatusString[i] == '1') {
      digitalWrite(leds[i], HIGH);
    }
    // If it's not '1', turn off the corresponding LED
    else {
      digitalWrite(leds[i], LOW);
    }
  }
    digitalWrite(leds2[0], HIGH); // Light corresponding LED
    digitalWrite(leds2[1], LOW);  // Light corresponding LED
    digitalWrite(leds2[2], LOW);  // Light corresponding LED
  } else if (Value == 2) {
    for (int i = 0; i < 3; i++) {
      pinStatusString[i] = '0';
      pinStatusString[NUM_PINS - 1 - i] = '0';
    }
for (int i = 0; i <= 7; i++) {
    // Check if the character is '1', if so, turn on the corresponding LED
    if (pinStatusString[i] == '1') {
      digitalWrite(leds[i], HIGH);
    }
    // If it's not '1', turn off the corresponding LED
    else {
      digitalWrite(leds[i], LOW);
    }
  }
    digitalWrite(leds2[0], LOW);  // Light corresponding LED
    digitalWrite(leds2[1], HIGH); // Light corresponding LED
    digitalWrite(leds2[2], LOW);  // Light corresponding LED
  } else if (Value == 3) {
    for (int i = 0; i < 3; i++) {
      pinStatusString[i] = '0';
      pinStatusString[NUM_PINS - 1 - i] = '0';
    }
    for (int i = 0; i <= 7; i++) {
    // Check if the character is '1', if so, turn on the corresponding LED
    if (pinStatusString[i] == '1') {
      digitalWrite(leds[i], HIGH);
    }
    // If it's not '1', turn off the corresponding LED
    else {
      digitalWrite(leds[i], LOW);
    }
  }
    digitalWrite(leds2[0], LOW);   // Light corresponding LED
    digitalWrite(leds2[1], LOW);   // Light corresponding LED
    digitalWrite(leds2[2], HIGH);  // Light corresponding LED
  }
  Serial.println(pinStatusString);
  sendMessage(pinStatusString);

  delay(90);
}



void readPins(char *pinStatusString, int length) {
  if(Value !=0) {
    for (int i = 2; i <= 9; i++) {
    pinStatusString[i - 2] = digitalRead(i) == HIGH ? '1' : '0';
    
  }
  pinStatusString[length] = '\0'; // Place null terminator after the loop
}
}

void sendMessage(const char *message) {
  uint8_t len = strlen(message);
  char tempMessage[len + 1]; // Temporary buffer to hold message
  strcpy(tempMessage, message); // Copy message to temporary buffer
  radio.write((uint8_t *)tempMessage, len);
}
