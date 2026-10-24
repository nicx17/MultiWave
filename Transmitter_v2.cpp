// MultiWave transmitter (remote unit) - v2
//
// Reads the 8 conductor inputs, shows them on the LED array and sends them to
// the receiver as an 8-character '0'/'1' string. Same hardware and same radio
// message format as Transmitter_v1.cpp.

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ---------------------------------------------------------------------------
// Pins (unchanged from v1)
// ---------------------------------------------------------------------------
const uint8_t RADIO_CE_PIN = 35;
const uint8_t RADIO_CSN_PIN = 33;
const uint8_t BUTTON_PIN = 36;  // external pull-down, HIGH = pressed

const uint8_t NUM_INPUTS = 8;
const uint8_t INPUT_PINS[NUM_INPUTS] = { 2, 3, 4, 5, 6, 7, 8, 9 };  // external pull-downs
const uint8_t LED_PINS[NUM_INPUTS] = { 12, 14, 16, 18, 20, 22, 24, 26 };
const uint8_t PORT_LED_PINS[3] = { 13, 15, 17 };  // one LED above each port

// RJ11 and BNC share the middle two inputs (RJ45 pins 4 and 5).
const uint8_t PAIR_FIRST_INPUT = 3;
const uint8_t PAIR_LAST_INPUT = 4;

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------
const unsigned long SAMPLE_MS = 50;
const unsigned long DEBOUNCE_MS = 50;

const byte RADIO_ADDRESS[6] = "00001";

// Port order follows the selection button: none -> RJ45 -> RJ11 -> BNC -> none
enum Port : uint8_t { PORT_NONE, PORT_RJ45, PORT_RJ11, PORT_BNC, PORT_COUNT };
const char *const PORT_NAMES[PORT_COUNT] = { "NONE", "RJ45", "RJ11", "BNC" };

RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

uint8_t selectedPort = PORT_NONE;
char pinStatus[NUM_INPUTS + 1] = "00000000";
char lastLogged[NUM_INPUTS + 1] = "";
unsigned long lastSample = 0;

// Button debounce
uint8_t buttonStable = LOW;
uint8_t buttonLastRaw = LOW;
unsigned long buttonChangedAt = 0;

bool buttonPressed() {
  uint8_t raw = digitalRead(BUTTON_PIN);
  unsigned long now = millis();
  if (raw != buttonLastRaw) {
    buttonLastRaw = raw;
    buttonChangedAt = now;
  }
  if (now - buttonChangedAt >= DEBOUNCE_MS && raw != buttonStable) {
    buttonStable = raw;
    return buttonStable == HIGH;
  }
  return false;
}

void showPortLeds() {
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(PORT_LED_PINS[i], (selectedPort == i + 1) ? HIGH : LOW);
  }
}

void showPinLeds() {
  for (uint8_t i = 0; i < NUM_INPUTS; i++) {
    digitalWrite(LED_PINS[i], pinStatus[i] == '1' ? HIGH : LOW);
  }
}

void readInputs() {
  for (uint8_t i = 0; i < NUM_INPUTS; i++) {
    bool used = selectedPort == PORT_RJ45 || (i >= PAIR_FIRST_INPUT && i <= PAIR_LAST_INPUT);
    pinStatus[i] = (used && digitalRead(INPUT_PINS[i]) == HIGH) ? '1' : '0';
  }
  pinStatus[NUM_INPUTS] = '\0';
}

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT);
  for (uint8_t i = 0; i < NUM_INPUTS; i++) {
    pinMode(INPUT_PINS[i], INPUT);
    pinMode(LED_PINS[i], OUTPUT);
  }
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(PORT_LED_PINS[i], OUTPUT);
  }
  showPortLeds();
  showPinLeds();

  if (!radio.begin()) {
    Serial.println(F("Radio not found"));
  }
  radio.openWritingPipe(RADIO_ADDRESS);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  Serial.println(F("MultiWave transmitter v2 ready"));
}

void loop() {
  if (buttonPressed()) {
    selectedPort = (selectedPort + 1) % PORT_COUNT;
    showPortLeds();
    Serial.print(F("Port: "));
    Serial.println(PORT_NAMES[selectedPort]);
  }

  unsigned long now = millis();
  if (now - lastSample < SAMPLE_MS) return;
  lastSample = now;

  // With no port selected nothing is sent, so the receiver shows
  // "NO DATA RECEIVED" instead of acting on an old reading.
  if (selectedPort == PORT_NONE) {
    memset(pinStatus, '0', NUM_INPUTS);
    showPinLeds();
    return;
  }

  readInputs();
  showPinLeds();
  radio.write(pinStatus, sizeof(pinStatus));  // includes the terminating '\0'

  if (strcmp(pinStatus, lastLogged) != 0) {
    strcpy(lastLogged, pinStatus);
    Serial.println(pinStatus);
  }
}
