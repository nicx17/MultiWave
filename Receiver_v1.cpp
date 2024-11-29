#include <Adafruit_SH1106.h>  // Library for OLED display
#include <SPI.h>              // SPI library for Arduino Ethernet
#include <Ethernet.h>         // Ethernet library
#include <Arduino.h>
#include <nRF24L01.h>      // Library for nRF24L01 module
#include <RF24.h>          // RF24 library for nRF24L01 module
#include <Wire.h>          // I2C library
#include <Adafruit_GFX.h>  // Graphics library for OLED
#include <ezButton.h>      // Button library for debouncing
#define OLED_RESET 4  // OLED reset pin
Adafruit_SH1106 display(OLED_RESET);
#define SCREEN_WIDTH 128  // OLED screen width
#define SCREEN_HEIGHT 64  // OLED screen height
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)

// Button and state variables
const int buttonPin = 22;
int buttonState = 0;      // Current button state
int lastButtonState = 0;  // Previous button state
int variableValue = 0;    // Variable for current value (1 to 3)
int previousValue = 0;    // Store previous value

// LED pin definitions
const int ledPins[] = { 40, 38, 36, 34, 32, 26, 28, 24 };

// RJ45, RJ11 and BNC pin definitions
const int portPins[] = { 23, 25, 27, 29, 33, 37, 39, 41 };
const int numPins = sizeof(portPins) / sizeof(portPins[0]);
const int rj11Pins[] = { 44, 46 };
const int num2Pins = sizeof(rj11Pins) / sizeof(rj11Pins[0]);
const int bncPins[] = { 47, 45 };
const int num3Pins = sizeof(bncPins) / sizeof(bncPins[0]);

// Various string configurations for ports, options, and wiring
String receivedMessage = "";
String binaryValue = "00001000";
String binaryValue2 = "00001000";
String binaryString3 = "00010000";
String binaryString4 = "10000000";
String options[] = { "TEST CONTINUITY", "TEST SHORT", "TEST WIRING", "MANUAL" };
int numOpt = sizeof(options) / sizeof(options[0]);
String ports[] = { "RJ45 T568A", "RJ45 T568B", "BNC", "RJ11" };
int numPorts = sizeof(ports) / sizeof(ports[0]);
String wireColour1[] = { "White/Green", "Green", "White/Orange", "Blue", "White/Blue", "Orange", "White/Brown", "Brown" };
int num568a = sizeof(wireColour1) / sizeof(wireColour1[0]);
String wireColour2[] = { "White/Orange", "Orange", "White/Green", "Blue", "White/Blue", "Green", "White/Brown", "Brown" };
int num568b = sizeof(wireColour2) / sizeof(wireColour2[0]);
String wireColour3[] = { "BLACK/GND", "RED/SIGNAL" };

// Rotary encoder pins
#define CLK_PIN 10
#define DT_PIN 4
#define SW_PIN 8

// Encoder and button variables
int counter = 0;
int CLK_state;
int prev_CLK_state;
volatile unsigned int encoderPos = 0;
int currentValue = encoderPos;
boolean buttonPressed = false;
ezButton button(SW_PIN);  // Using the ezButton library for debouncing

// nRF24L01 radio setup
const byte address[6] = "00001";
RF24 radio(2, 11);  // CE, CSN pins

// Function declarations
void function1();
void function2();
void function3();
void function4();
void (*functions[])() = { function1, function2, function3, function4 };

void setup() {
  // Button pin configuration
  pinMode(buttonPin, INPUT_PULLUP);

  // Configure LED pins as outputs
  for (int i = 0; i < 8; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Configure rotary encoder pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  button.setDebounceTime(50);
  prev_CLK_state = digitalRead(CLK_PIN);

  // Initialize RF module
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  // Initialize OLED display
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  Serial.begin(9600);

  // Configure RJ45 pins
  for (int i = 0; i < numPins; ++i) {
    pinMode(portPins[i], OUTPUT);
  }
  for (int i = 0; i < num2Pins; ++i) {
    pinMode(rj11Pins[i], OUTPUT);
    pinMode(bncPins[i], OUTPUT);
  }
}

void loop() {
  // Reset LED states
  for (int i = 0; i < 8; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  button.loop();
  CLK_state = digitalRead(CLK_PIN);

  // Handle rotary encoder movement
  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_PIN) == HIGH) {
      encoderPos = (encoderPos - 1 + 4) % 4;
      Serial.print("encoderPos: ");
      Serial.println(encoderPos);
    } else {
      encoderPos = (encoderPos + 1) % 4;
      Serial.print("encoderPos: ");
      Serial.println(encoderPos);
    }
  }
  prev_CLK_state = CLK_state;

  // Check button press
  if (button.isPressed()) {
    buttonPressed = true;
  }

  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    variableValue++;
    delay(500);
    if (variableValue > 3) {
      variableValue = 0;
    }
    Serial.println(variableValue);
  }

  // Call selected function when encoder button is pressed
  int index = encoderPos;
  if (buttonPressed) {
    buttonPressed = false;
    functions[encoderPos]();
  }

  // Update OLED display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("MODE: ");
  display.println(options[index]);
  display.setCursor(0, 20);
  display.print("PORT: ");
  display.println(ports[variableValue]);
  display.display();
}


void function1() {
  Serial.println("TEST CONTINUITY SELECTED");
  String binaryValue = "00000001";
  String binaryValue2 = "00001000";
  String binaryString3 = "00010000";
  String receivedMessage = "";  // String object to store received message
  char text[32] = "";
  int d = 0;
  int p;
  while (encoderPos == 0) {
    String binaryString3 = "00010000";
    int k = 0;
    int t = 5;

    if (variableValue == 0) {
      for (int i = 0; i < numPins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {

            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;

        char lastBit = binaryValue2[binaryValue2.length() - 1];                         // Store the last bit
        binaryValue2 = lastBit + binaryValue2.substring(0, binaryValue2.length() - 1);  // Shift all bits to the right
        Serial.print("BIN: ");
        Serial.println(binaryValue2);                                                 // Print the binary representation
        char lastBit2 = binaryValue[binaryValue.length() - 1];                        // Store the last bit
        binaryValue = lastBit2 + binaryValue.substring(0, binaryValue.length() - 1);  // Shift all bits to the right
        // Serial.print("BIN: ");
        // Serial.println(binaryValue);  // Print the binary representation

        digitalWrite(portPins[i], HIGH);
        for (int w = 0; w <= 7; w++) {
          digitalWrite(ledPins[w], binaryValue.charAt(w) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed
        if (radio.available()) {

          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryValue2[t] == receivedMessage[t]) {

            if (binaryValue2 == "10000000") {
              p = 8;
            } else if (binaryValue2 == "01000000") {
              p = 7;
            }
            if (binaryValue2 == "00100000") {
              p = 6;
            } else if (binaryValue2 == "00010000") {
              p = 5;
            }
            if (binaryValue2 == "00001000") {
              p = 4;
            } else if (binaryValue2 == "00000100") {
              p = 3;
            }
            if (binaryValue2 == "00000010") {
              p = 2;
            } else if (binaryValue2 == "00000001") {
              p = 1;
            }
            k++;
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue2);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" CONNECTED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour1[p - 1]);
            display.display();
          } else {
            if (binaryValue2 == "10000000") {
              p = 8;
            } else if (binaryValue2 == "01000000") {
              p = 7;
            }
            if (binaryValue2 == "00100000") {
              p = 6;
            } else if (binaryValue2 == "00010000") {
              p = 5;
            }
            if (binaryValue2 == "00001000") {
              p = 4;
            } else if (binaryValue2 == "00000100") {
              p = 3;
            }
            if (binaryValue2 == "00000010") {
              p = 2;
            } else if (binaryValue2 == "00000001") {
              p = 1;
            }
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST CONTINUITY FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue2);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("NOT CONNECTED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour1[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }
        t++;
        if (t > 7) {
          t = 0;
        }
        if (k == 8) {

          display.setCursor(0, 0);
          display.println("TEST CONTINUITY PASS");

          // Display buffer
          display.display();
        }

        digitalWrite(portPins[i], LOW);
      }
    }

    if (variableValue == 1) {

      for (int i = 0; i < numPins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {

            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        // Set the current pin high and others low
        int p;

        char lastBit = binaryValue2[binaryValue2.length() - 1];                         // Store the last bit
        binaryValue2 = lastBit + binaryValue2.substring(0, binaryValue2.length() - 1);  // Shift all bits to the right
        Serial.print("BIN: ");
        Serial.println(binaryValue2);                                                 // Print the binary representation
        char lastBit2 = binaryValue[binaryValue.length() - 1];                        // Store the last bit
        binaryValue = lastBit2 + binaryValue.substring(0, binaryValue.length() - 1);  // Shift all bits to the right
        // Serial.print("BIN: ");
        // Serial.println(binaryValue);  // Print the binary representation

        digitalWrite(portPins[i], HIGH);
        for (int w = 0; w <= 7; w++) {
          digitalWrite(ledPins[w], binaryValue.charAt(w) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed
        if (radio.available()) {

          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryValue2[t] == receivedMessage[t]) {
            if (binaryValue2 == "10000000") {
              p = 8;
            } else if (binaryValue2 == "01000000") {
              p = 7;
            }
            if (binaryValue2 == "00100000") {
              p = 6;
            } else if (binaryValue2 == "00010000") {
              p = 5;
            }
            if (binaryValue2 == "00001000") {
              p = 4;
            } else if (binaryValue2 == "00000100") {
              p = 3;
            }
            if (binaryValue2 == "00000010") {
              p = 2;
            } else if (binaryValue2 == "00000001") {
              p = 1;
            }
            k++;
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue2);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" CONNECTED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour2[p - 1]);
            display.display();
          } else {
            if (binaryValue2 == "10000000") {
              p = 8;
            } else if (binaryValue2 == "01000000") {
              p = 7;
            }
            if (binaryValue2 == "00100000") {
              p = 6;
            } else if (binaryValue2 == "00010000") {
              p = 5;
            }
            if (binaryValue2 == "00001000") {
              p = 4;
            } else if (binaryValue2 == "00000100") {
              p = 3;
            }
            if (binaryValue2 == "00000010") {
              p = 2;
            } else if (binaryValue2 == "00000001") {
              p = 1;
            }
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST CONTINUITY FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue2);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("NOT CONNECTED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour2[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }
        t++;
        if (t > 7) {
          t = 0;
        }
        if (k == 8) {

          display.setCursor(0, 0);
          display.println("TEST CONTINUITY PASS");

          // Display buffer
          display.display();
        }

        digitalWrite(portPins[i], LOW);
      }
    }

    if (variableValue == 2) {
      digitalWrite(portPins[1], LOW);
      String binaryString3 = "00010000";
      int p;
      t = 4;
      for (int i = 0; i < num2Pins; ++i) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {

          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        String center = binaryString3.substring(3, 5);
        // Rotate the center characters
        char temp = center[0];
        center[0] = center[1];
        center[1] = temp;
        // Replace the rotated center back into the string
        binaryString3.setCharAt(3, center[0]);
        binaryString3.setCharAt(4, center[1]);
        Serial.print("BIN: ");
        Serial.println(binaryString3);  // Print the binary representation

        digitalWrite(bncPins[i], HIGH);
        for (int w = 7; w >= 0; w--) {
          int inverseIndex = binaryString3.length() - 1 - w;
          digitalWrite(ledPins[w], binaryString3.charAt(inverseIndex) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed

        if (radio.available()) {

          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryString3[t] == receivedMessage[t]) {
            int p;
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            k++;
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" CONNECTED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour3[p - 1]);
            display.display();
          } else {
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST CONTINUITY FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("NOT CONNECTED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour3[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }
        t++;
        if (t > 4) {
          t = 3;
        }
        if (k == 2) {

          display.setCursor(0, 0);
          display.println("TEST CONTINUITY PASS");
          display.display();
        }
        digitalWrite(bncPins[i], LOW);
      }
    }



    if (variableValue == 3) {
      digitalWrite(portPins[1], LOW);
      String binaryString3 = "00010000";
      t = 4;
      for (int i = 0; i < num3Pins; ++i) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        String center = binaryString3.substring(3, 5);
        // Rotate the center characters
        char temp = center[0];
        center[0] = center[1];
        center[1] = temp;
        // Replace the rotated center back into the string
        binaryString3.setCharAt(3, center[0]);
        binaryString3.setCharAt(4, center[1]);
        Serial.print("BIN: ");
        Serial.println(binaryString3);  // Print the binary representation

        digitalWrite(rj11Pins[i], HIGH);
        for (int w = 7; w >= 0; w--) {
          int inverseIndex = binaryString3.length() - 1 - w;
          digitalWrite(ledPins[w], binaryString3.charAt(inverseIndex) == '1' ? HIGH : LOW);
        }
        delay(300);
        if (radio.available()) {
          radio.read(&text, sizeof(text));

          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryString3[t] == receivedMessage[t]) {
            int p;
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            k++;
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" CONNECTED");
            display.display();
          } else {
            int p;
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST CONTINUITY FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("NOT CONNECTED");
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }
        t++;
        if (t > 4) {
          t = 3;
        }
        if (k == 2) {

          display.setCursor(0, 0);
          display.println("TEST CONTINUITY PASS");
          d++;
          // Display buffer
          display.display();
        }

        digitalWrite(rj11Pins[i], LOW);
      }
    }
  }
}

void function2() {
  String binaryString3 = "00010000";
  String binaryValue = "00001000";
  String binaryValue2 = "00000001";
  int z = 0;
  int p;
  int sum = 0;
  int k = 0;
  char text[32] = "";
  Serial.println("TEST SHORT SELECTED");

  while (encoderPos == 1) {
    display.clearDisplay();

    if (variableValue == 0) {
      for (int i = 0; i < numPins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        char lastBit = binaryValue[binaryValue.length() - 1];                        // Store the last bit
        binaryValue = lastBit + binaryValue.substring(0, binaryValue.length() - 1);  // Shift all bits to the right
        Serial.print("BIN: ");
        char lastBit2 = binaryValue2[binaryValue2.length() - 1];                         // Store the last bit
        binaryValue2 = lastBit2 + binaryValue2.substring(0, binaryValue2.length() - 1);  // Shift all bits to the right
        Serial.println(binaryValue);                                                     // Print the binary representation

        digitalWrite(portPins[i], HIGH);
        for (int w = 0; w < 8; w++) {
          digitalWrite(ledPins[w], binaryValue2.charAt(w) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed


        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message
          char text[32] = "";
          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          for (int i = 0; i < receivedMessage.length(); i++) {
            // Convert the character to an integer and add it to the sum
            sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
          }
          Serial.println(sum);
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryValue == receivedMessage) {

            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }


            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" Clear");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" Clear");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour1[p - 1]);
            display.display();
          } else {
            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }


            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" Clear");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" SHORT");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour1[p - 1]);
            display.display();
          }


          if (sum == 1) {
            z++;
            sum = 0;
          } else if (sum >= 1) {
            k++;
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }



        if (z >= 8) {

          display.setCursor(0, 0);
          display.println("TEST SHORT: PASS");
          display.display();
          z = 1;
        } else if (k > 1) {
          Serial.println(k);
          display.setCursor(0, 0);
          display.println("TEST SHORT: FAIL");
          display.display();
          k = 0;
        }


        digitalWrite(portPins[i], LOW);
      }
    }


    if (variableValue == 1) {
      for (int i = 0; i < numPins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        char lastBit = binaryValue[binaryValue.length() - 1];                        // Store the last bit
        binaryValue = lastBit + binaryValue.substring(0, binaryValue.length() - 1);  // Shift all bits to the right
        Serial.print("BIN: ");
        char lastBit2 = binaryValue2[binaryValue2.length() - 1];                         // Store the last bit
        binaryValue2 = lastBit2 + binaryValue2.substring(0, binaryValue2.length() - 1);  // Shift all bits to the right
        Serial.println(binaryValue);                                                     // Print the binary representation

        digitalWrite(portPins[i], HIGH);
        for (int w = 0; w < 8; w++) {
          digitalWrite(ledPins[w], binaryValue2.charAt(w) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed


        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message
          char text[32] = "";
          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          for (int i = 0; i < receivedMessage.length(); i++) {
            // Convert the character to an integer and add it to the sum
            sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
          }
          Serial.println(sum);
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryValue == receivedMessage) {

            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }


            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" Clear");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" Clear");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour2[p - 1]);
            display.display();
          } else {
            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }


            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" Clear");
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" SHORT");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour2[p - 1]);
            display.display();
          }
          if (sum == 1) {
            z++;
            sum = 0;
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");
          display.display();
        }

        if (z >= 8) {

          display.setCursor(0, 0);
          display.println("TEST SHORT: PASS");
          display.display();
          z = 1;
        } else if (k > 1) {
          Serial.println(k);
          display.setCursor(0, 0);
          display.println("TEST SHORT: FAIL");
          display.display();
          k = 0;
        }
        digitalWrite(portPins[i], LOW);
      }
    }

    if (variableValue == 2) {
      digitalWrite(portPins[1], LOW);
      String binaryString3 = "00010000";
      z = 0;
      int p;
      for (int i = 0; i < num2Pins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;

        int k = 0;
        String center = binaryString3.substring(3, 5);
        // Rotate the center characters
        char temp = center[0];
        center[0] = center[1];
        center[1] = temp;
        // Replace the rotated center back into the string
        binaryString3.setCharAt(3, center[0]);
        binaryString3.setCharAt(4, center[1]);
        Serial.print("BIN: ");
        Serial.println(binaryString3);  // Print the binary representation
        digitalWrite(bncPins[i], HIGH);
        for (int w = 7; w >= 0; w--) {
          int inverseIndex = binaryString3.length() - 1 - w;
          digitalWrite(ledPins[w], binaryString3.charAt(inverseIndex) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed

        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message

          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryString3 == receivedMessage) {
            // Iterate through each character in the string
            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }
            display.clearDisplay();
            display.setCursor(0, 16);
            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" Clear");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour3[p - 1]);
            display.display();
          } else {
            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }
            display.clearDisplay();
            display.setCursor(0, 16);
            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" SHORT");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour3[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }

        if (sum == 1) {
          z++;
          sum = 0;
        } else if (sum > 1) {
          k++;
          sum = 0;
        }

        if (z >= 2) {

          display.setCursor(0, 0);
          display.println("TEST SHORT: PASS");
          display.display();
          z = 0;
          k = 0;
        } else if (k > 0) {
          k = 0;
          display.setCursor(0, 0);
          display.println("TEST SHORT: FAIL");
          display.display();
        }


        digitalWrite(bncPins[i], LOW);
      }
    }


    if (variableValue == 3) {
      digitalWrite(portPins[1], LOW);
      String binaryString3 = "00010000";
      for (int i = 0; i < num3Pins; ++i) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        int p;

        int k = 0;

        String center = binaryString3.substring(3, 5);
        // Rotate the center characters
        char temp = center[0];
        center[0] = center[1];
        center[1] = temp;
        // Replace the rotated center back into the string
        binaryString3.setCharAt(3, center[0]);
        binaryString3.setCharAt(4, center[1]);
        Serial.print("BIN: ");
        Serial.println(binaryString3);  // Print the binary representation


        digitalWrite(rj11Pins[i], HIGH);
        for (int w = 7; w >= 0; w--) {
          int inverseIndex = binaryString3.length() - 1 - w;
          digitalWrite(ledPins[w], binaryString3.charAt(inverseIndex) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed

        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message

          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryString3 == receivedMessage) {
            // Iterate through each character in the string
            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            int p;
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            k++;
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" Clear");
            display.clearDisplay();
            display.setCursor(0, 16);
            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" Clear");
            display.display();
          } else {
            for (int i = 0; i < receivedMessage.length(); i++) {
              // Convert the character to an integer and add it to the sum
              sum += receivedMessage[i] - '0';  // Subtract '0' to convert from ASCII to integer
            }
            Serial.println(sum);
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }
            display.clearDisplay();
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" Clear");
            display.setCursor(0, 16);
            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" SHORT");
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");
          display.display();
        }

        if (sum == 1) {
          z++;
          sum = 0;
        } else if (sum > 1) {
          k++;
          sum = 0;
        }

        if (z >= 2) {

          display.setCursor(0, 0);
          display.println("TEST SHORT: PASS");
          display.display();
          z = 0;
          k = 0;
        } else if (k > 0) {
          k = 0;
          display.setCursor(0, 0);
          display.println("TEST SHORT: FAIL");
          display.display();
        }
        digitalWrite(rj11Pins[i], LOW);
      }
    }
  }
}
void function3() {
  digitalWrite(portPins[1], LOW);
  String binaryValue = "00001000";
  int t = 0;
  String binaryValue2 = "00000001";
  int p;
  char text[32] = "";
  Serial.println("testWiring selected!");
  while (encoderPos == 2) {
    int k = 0;
    if (variableValue == 0) {
      int t = 5;
      for (int i = 0; i < numPins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        char lastBit = binaryValue[binaryValue.length() - 1];                            // Store the last bit
        binaryValue = lastBit + binaryValue.substring(0, binaryValue.length() - 1);      // Shift all bits to the right
        char lastBit2 = binaryValue2[binaryValue2.length() - 1];                         // Store the last bit
        binaryValue2 = lastBit2 + binaryValue2.substring(0, binaryValue2.length() - 1);  // Shift all bits to the right
        Serial.print("BIN: ");
        Serial.println(binaryValue);  // Print the binary representation
        digitalWrite(portPins[i], HIGH);
        for (int l = 0; l < 8; l++) {
          digitalWrite(ledPins[l], binaryValue2.charAt(l) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed
        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message
          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryValue[t] == receivedMessage[t]) {
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }
            k++;
            display.clearDisplay();
            display.setCursor(0, 16);
            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour1[p - 1]);
            display.display();
          } else {
            int p;
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST WIRING FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("MISMATCHED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour1[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }


        t++;
        if (t > 7) {
          t = 0;
        }
        if (k == 8) {
          display.setCursor(0, 0);
          display.println("TEST WIRING PASS");

          // Display buffer
          display.display();
        }




        digitalWrite(portPins[i], LOW);
      }
    }
    if (variableValue == 1) {
      int t = 5;
      for (int i = 0; i < numPins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        char lastBit = binaryValue[binaryValue.length() - 1];                            // Store the last bit
        binaryValue = lastBit + binaryValue.substring(0, binaryValue.length() - 1);      // Shift all bits to the right
        char lastBit2 = binaryValue2[binaryValue2.length() - 1];                         // Store the last bit
        binaryValue2 = lastBit2 + binaryValue2.substring(0, binaryValue2.length() - 1);  // Shift all bits to the right
        Serial.print("BIN: ");
        Serial.println(binaryValue);  // Print the binary representation

        digitalWrite(portPins[i], HIGH);
        for (int l = 0; l < 8; l++) {
          digitalWrite(ledPins[l], binaryValue2.charAt(l) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed


        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message
          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryValue[t] == receivedMessage[t]) {
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }
            k++;
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour2[p - 1]);
            display.display();
          } else {
            int p;
            if (binaryValue == "10000000") {
              p = 8;
            } else if (binaryValue == "01000000") {
              p = 7;
            }
            if (binaryValue == "00100000") {
              p = 6;
            } else if (binaryValue == "00010000") {
              p = 5;
            }
            if (binaryValue == "00001000") {
              p = 4;
            } else if (binaryValue == "00000100") {
              p = 3;
            }
            if (binaryValue == "00000010") {
              p = 2;
            } else if (binaryValue == "00000001") {
              p = 1;
            }

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST WIRING FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryValue);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("MISMATCHED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour2[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }

        t++;
        if (t > 7) {
          t = 0;
        }
        if (k == 8) {
          display.setCursor(0, 0);
          display.println("TEST WIRING PASS");

          // Display buffer
          display.display();
        }




        digitalWrite(portPins[i], LOW);
      }
    }

    if (variableValue == 2) {

      String binaryString3 = "00010000";
      int t = 4;
      for (int i = 0; i < num2Pins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        // Set the current pin high and others low
        int k = 0;
        String center = binaryString3.substring(3, 5);
        // Rotate the center characters
        char temp = center[0];
        center[0] = center[1];
        center[1] = temp;
        // Replace the rotated center back into the string
        binaryString3.setCharAt(3, center[0]);
        binaryString3.setCharAt(4, center[1]);
        Serial.print("BIN: ");
        Serial.println(binaryString3);  // Print the binary representation


        digitalWrite(bncPins[i], HIGH);
        for (int w = 7; w >= 0; w--) {
          int inverseIndex = binaryString3.length() - 1 - w;
          digitalWrite(ledPins[w], binaryString3.charAt(inverseIndex) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed
        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message
          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryString3[t] == receivedMessage[t]) {
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }
            k++;
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" Clear");
            display.display();
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour3[p - 1]);
            display.display();
          } else {
            int p;
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST WIRING FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("MISMATCHED");
            display.setCursor(0, 48);
            display.print("WIRCLR: ");
            display.print(wireColour3[p - 1]);
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }
        t++;
        if (t > 4) {
          t = 3;
        }
        if (k >= 2) {
          display.setCursor(0, 0);
          display.println("TEST WIRING PASS");
          k = 0;
          // Display buffer
          display.display();
        }
        digitalWrite(bncPins[i], LOW);
      }
    }

    if (variableValue == 3) {
      digitalWrite(portPins[1], LOW);
      String binaryString3 = "00010000";
      int t = 4;
      for (int i = 0; i < num2Pins; i++) {
        button.loop();
        CLK_state = digitalRead(CLK_PIN);
        if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
          if (digitalRead(DT_PIN) == HIGH) {

            encoderPos = (encoderPos + 1) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          } else {
            encoderPos = (encoderPos - 1 + 3) % 3;
            Serial.print("encoderPos: ");
            Serial.println(encoderPos);
            break;
          }
        }
        prev_CLK_state = CLK_state;
        // Set the current pin high and others low
        int k = 0;
        String center = binaryString3.substring(3, 5);
        // Rotate the center characters
        char temp = center[0];
        center[0] = center[1];
        center[1] = temp;
        // Replace the rotated center back into the string
        binaryString3.setCharAt(3, center[0]);
        binaryString3.setCharAt(4, center[1]);
        Serial.print("BIN: ");
        Serial.println(binaryString3);  // Print the binary representation


        digitalWrite(rj11Pins[i], HIGH);
        for (int w = 7; w >= 0; w--) {
          int inverseIndex = binaryString3.length() - 1 - w;
          digitalWrite(ledPins[w], binaryString3.charAt(inverseIndex) == '1' ? HIGH : LOW);
        }
        delay(300);  // Adjust delay as needed
        if (radio.available()) {
          String receivedMessage = "";  // String object to store received message
          radio.read(&text, sizeof(text));
          receivedMessage = text;  // Copy char array to String object
          Serial.print("RX: ");
          Serial.println(receivedMessage);

          delay(500);
          if (binaryString3[t] == receivedMessage[t]) {
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }
            k++;
            display.clearDisplay();
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print(" Clear");
            display.display();
            // Binary representation of matched pin
            Serial.print("Pin ");
            Serial.print(p);
            Serial.println(" matched!");
          } else {
            int p;
            if (binaryString3 == "00010000") {
              p = 1;
            } else if (binaryString3 == "00001000") {
              p = 2;
            }

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("TEST WIRING FAIL");
            display.setCursor(0, 16);

            display.print("SENT: ");
            display.println(binaryString3);
            display.print("RECV: ");
            display.println(receivedMessage);
            display.setCursor(0, 40);
            display.print("Pin:");
            display.print(p);
            display.print("MISMATCHED");
            display.display();
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 16);
          display.println("NO DATA RECIEVED");

          display.display();
        }
        t++;
        if (t > 4) {
          t = 3;
        }
        if (k >= 2) {
          display.setCursor(0, 0);
          display.println("TEST WIRING PASS");
          k = 0;
          // Display buffer
          display.display();
        }
        digitalWrite(rj11Pins[i], LOW);
      }
    }
  }
}



void function4()

{
  String binaryString4 = "10000000";
  char text[32] = "";
  int x = 1;
  while (x == 1) {
    int p;



    if (variableValue == 0) {

      button.loop();
      if (button.isPressed()) {
        x = 2;
        break;
      }
      CLK_state = digitalRead(CLK_PIN);

      if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
        if (digitalRead(DT_PIN) == HIGH) {
          encoderPos = encoderPos - 1;
          currentValue = encoderPos;
          Serial.print("encoderPos: ");
          Serial.println(encoderPos);
        } else {
          encoderPos = encoderPos + 1;
          currentValue = encoderPos;
          Serial.print("encoderPos: ");
          Serial.println(encoderPos);
        }
      }
      prev_CLK_state = CLK_state;
      if (currentValue > previousValue) {
        Serial.println("Value has increased.");
        char lastBit = binaryString4[binaryString4.length() - 1];                          // Store the last bit
        binaryString4 = lastBit + binaryString4.substring(0, binaryString4.length() - 1);  // Shift all bits to the right

        for (int w = 0; w <= 7; w++) {
          digitalWrite(ledPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
          digitalWrite(portPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
        }
        Serial.println(binaryString4);
      } else if (currentValue < previousValue) {
        Serial.println("Value has decreased.");
        char firstBit = binaryString4[0];                       // Store the first bit
        binaryString4 = binaryString4.substring(1) + firstBit;  // Shift all bits to the left
        for (int w = 0; w <= 7; w++) {
          digitalWrite(ledPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
          digitalWrite(portPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
        }
        Serial.println(binaryString4);
      }
      previousValue = currentValue;
      if (radio.available()) {

        radio.read(&text, sizeof(text));
        receivedMessage = text;  // Copy char array to String object
        Serial.print("RX: ");
        Serial.println(receivedMessage);
        if (binaryString4 == "10000000") {
          p = 8;
        } else if (binaryString4 == "01000000") {
          p = 7;
        }
        if (binaryString4 == "00100000") {
          p = 6;
        } else if (binaryString4 == "00010000") {
          p = 5;
        }
        if (binaryString4 == "00001000") {
          p = 4;
        } else if (binaryString4 == "00000100") {
          p = 3;
        }
        if (binaryString4 == "00000010") {
          p = 2;
        } else if (binaryString4 == "00000001") {
          p = 1;
        }
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MANUAL");
        display.setCursor(0, 16);
        display.print("SENT: ");
        display.println(binaryString4);
        display.print("RECV: ");
        display.println(receivedMessage);
        display.setCursor(0, 40);
        display.print("PIN: ");
        display.print(p);
        display.setCursor(0, 50);
        display.print("WIRCLR: ");
        display.print(wireColour1[p - 1]);
        display.display();
      }

      // Update the previous value to the current value for the next iteration

      // delay(500);
    }
    if (variableValue == 1) {

      button.loop();
      if (button.isPressed()) {
        x = 2;
        break;
      }
      CLK_state = digitalRead(CLK_PIN);

      if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
        if (digitalRead(DT_PIN) == HIGH) {
          encoderPos = encoderPos - 1;
          currentValue = encoderPos;
          Serial.print("encoderPos: ");
          Serial.println(encoderPos);
        } else {
          encoderPos = encoderPos + 1;
          currentValue = encoderPos;
          Serial.print("encoderPos: ");
          Serial.println(encoderPos);
        }
      }
      prev_CLK_state = CLK_state;
      if (currentValue > previousValue) {
        Serial.println("Value has increased.");
        char lastBit = binaryString4[binaryString4.length() - 1];                          // Store the last bit
        binaryString4 = lastBit + binaryString4.substring(0, binaryString4.length() - 1);  // Shift all bits to the right

        for (int w = 0; w <= 7; w++) {
          digitalWrite(ledPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
          digitalWrite(portPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
        }
        Serial.println(binaryString4);
      } else if (currentValue < previousValue) {
        Serial.println("Value has decreased.");
        char firstBit = binaryString4[0];                       // Store the first bit
        binaryString4 = binaryString4.substring(1) + firstBit;  // Shift all bits to the left
        for (int w = 0; w <= 7; w++) {
          digitalWrite(ledPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
          digitalWrite(portPins[w], binaryString4.charAt(w) == '1' ? HIGH : LOW);
        }
        Serial.println(binaryString4);
      }
      previousValue = currentValue;
      if (radio.available()) {

        radio.read(&text, sizeof(text));
        receivedMessage = text;  // Copy char array to String object
        Serial.print("RX: ");
        Serial.println(receivedMessage);
        if (binaryString4 == "10000000") {
          p = 8;
        } else if (binaryString4 == "01000000") {
          p = 7;
        }
        if (binaryString4 == "00100000") {
          p = 6;
        } else if (binaryString4 == "00010000") {
          p = 5;
        }
        if (binaryString4 == "00001000") {
          p = 4;
        } else if (binaryString4 == "00000100") {
          p = 3;
        }
        if (binaryString4 == "00000010") {
          p = 2;
        } else if (binaryString4 == "00000001") {
          p = 1;
        }
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MANUAL");
        display.setCursor(0, 16);
        display.print("SENT: ");
        display.println(binaryString4);
        display.print("RECV: ");
        display.println(receivedMessage);
        display.setCursor(0, 40);
        display.print("PIN: ");
        display.print(p);
        display.setCursor(0, 50);
        display.print("WIRCLR: ");
        display.print(wireColour2[p - 1]);
        display.display();
      }

      // Update the previous value to the current value for the next iteration

      // delay(500);
    } else {
      display.clearDisplay();
      display.setCursor(0, 16);
      display.println("NO DATA RECIEVED");

      display.display();
    }

    if (variableValue == 2 || variableValue == 3) {
      display.clearDisplay();
      while (variableValue == 2 || variableValue == 3) {
        button.loop();
        if (button.isPressed()) {
          x = 2;
          break;
        }
        display.setCursor(0, 0);
        display.println("MANUAL MODE NOT AVAILABLE FOR THIS PORT");
        display.display();
      }
    }
  }
}