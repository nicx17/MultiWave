// MultiWave receiver (main unit) - v2
//
// Drives one conductor of the selected port at a time, waits for a fresh
// reading from the transmitter box and shows the result on the OLED.
// Same hardware and same radio message format as Receiver_v1.cpp.

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <ezButton.h>

#if !defined(__AVR_ATmega2560__)
#error "Receiver_v2 targets the Arduino Mega 2560 (encoder uses PCINT4 on pin 10)"
#endif

// ---------------------------------------------------------------------------
// Pins (unchanged from v1)
// ---------------------------------------------------------------------------
const uint8_t PORT_BUTTON_PIN = 22;
const uint8_t ENC_CLK_PIN = 10;  // PB4 / PCINT4
const uint8_t ENC_DT_PIN = 4;
const uint8_t ENC_SW_PIN = 8;
const uint8_t RADIO_CE_PIN = 2;
const uint8_t RADIO_CSN_PIN = 11;

const uint8_t LED_PINS[8] = { 40, 38, 36, 34, 32, 26, 28, 24 };
const uint8_t RJ45_PINS[8] = { 23, 25, 27, 29, 33, 37, 39, 41 };
const uint8_t BNC_PINS[2] = { 47, 45 };
const uint8_t RJ11_PINS[2] = { 44, 46 };

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------
const unsigned long SETTLE_MS = 60;        // after driving a pin, before trusting the radio
const unsigned long RX_TIMEOUT_MS = 400;   // give up waiting for a fresh packet
const unsigned long HOLD_MS = 400;         // keep each pin's result on screen
const unsigned long MANUAL_STALE_MS = 500; // manual mode: no packet for this long = no data
const unsigned long ENC_DEBOUNCE_US = 1500;

// ---------------------------------------------------------------------------
// Radio message: 8 ASCII '0'/'1' chars, one per transmitter input
// ---------------------------------------------------------------------------
const uint8_t NUM_SLOTS = 8;
const byte RADIO_ADDRESS[6] = "00001";

// ---------------------------------------------------------------------------
// Port table
// ---------------------------------------------------------------------------
const char *const T568A_COLOURS[8] = { "White/Green", "Green", "White/Orange", "Blue", "White/Blue", "Orange", "White/Brown", "Brown" };
const char *const T568B_COLOURS[8] = { "White/Orange", "Orange", "White/Green", "Blue", "White/Blue", "Green", "White/Brown", "Brown" };
const char *const BNC_COLOURS[2] = { "BLACK/GND", "RED/SIGNAL" };

// slot  = position in the transmitter's string that this pin should light up
//         (also the index of the LED that represents it)
// label = pin number shown to the user; colours are indexed by label - 1
//
// RJ45 labels keep v1's numbering (RJ45_PINS[0] is shown as pin 8). If the
// numbers or colours on screen are reversed on the real hardware, flip this.
const uint8_t RJ45_SLOTS[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const uint8_t RJ45_LABELS[8] = { 8, 7, 6, 5, 4, 3, 2, 1 };
const uint8_t PAIR_SLOTS[2] = { 3, 4 };  // RJ11 and BNC use the middle two inputs
const uint8_t PAIR_LABELS[2] = { 1, 2 };

struct PortDef {
  const char *name;
  const uint8_t *pins;
  uint8_t count;
  const uint8_t *slots;
  const uint8_t *labels;
  const char *const *colours;  // nullptr if the port has no colour code
};

const PortDef PORTS[] = {
  { "RJ45 T568A", RJ45_PINS, 8, RJ45_SLOTS, RJ45_LABELS, T568A_COLOURS },
  { "RJ45 T568B", RJ45_PINS, 8, RJ45_SLOTS, RJ45_LABELS, T568B_COLOURS },
  { "BNC", BNC_PINS, 2, PAIR_SLOTS, PAIR_LABELS, BNC_COLOURS },
  { "RJ11", RJ11_PINS, 2, PAIR_SLOTS, PAIR_LABELS, nullptr },
};
const uint8_t PORT_COUNT = sizeof(PORTS) / sizeof(PORTS[0]);

// ---------------------------------------------------------------------------
// Modes and results
// ---------------------------------------------------------------------------
enum Mode : uint8_t { MODE_CONTINUITY, MODE_SHORT, MODE_WIRING, MODE_MANUAL, MODE_COUNT };
const char *const MODE_MENU_NAMES[MODE_COUNT] = { "TEST CONTINUITY", "TEST SHORT", "TEST WIRING", "MANUAL" };
const char *const MODE_TITLES[MODE_COUNT] = { "CONTINUITY", "SHORT", "WIRING", "MANUAL" };

enum PinResult : uint8_t { PIN_OK, PIN_OPEN, PIN_SHORTED, PIN_CROSSED, PIN_NO_DATA };
enum SweepResult : uint8_t { SWEEP_NONE, SWEEP_PASS, SWEEP_FAIL };
enum Phase : uint8_t { PHASE_DRIVE, PHASE_SETTLE, PHASE_AWAIT, PHASE_HOLD };

// ---------------------------------------------------------------------------
// Hardware objects
// ---------------------------------------------------------------------------
Adafruit_SH1106 display(-1);  // reset pin not wired (v1 passed 4, which is the encoder's DT pin)
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);
ezButton knobButton(ENC_SW_PIN);
ezButton portButton(PORT_BUTTON_PIN);

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
volatile int8_t encoderDelta = 0;
volatile uint8_t encoderPrevClk = HIGH;
volatile unsigned long encoderLastUs = 0;

uint8_t selectedMode = MODE_CONTINUITY;
uint8_t selectedPort = 0;
bool running = false;
bool menuDirty = true;

// Automatic tests
uint8_t step = 0;
Phase phase = PHASE_DRIVE;
unsigned long phaseStart = 0;
uint8_t sweepFailures = 0;
SweepResult lastSweep = SWEEP_NONE;
char expected[NUM_SLOTS + 1];

// Manual mode
uint8_t manualIndex = 0;
char manualReading[NUM_SLOTS + 1];
unsigned long manualLastRx = 0;
bool manualHasData = false;
bool manualDirty = false;

// ---------------------------------------------------------------------------
// Encoder: CLK rising edge read in a pin-change interrupt so no detent is
// missed while the display or radio is busy.
// ---------------------------------------------------------------------------
ISR(PCINT0_vect) {
  uint8_t clk = digitalRead(ENC_CLK_PIN);
  if (clk != encoderPrevClk && clk == HIGH) {
    unsigned long now = micros();
    if (now - encoderLastUs >= ENC_DEBOUNCE_US) {
      encoderDelta += (digitalRead(ENC_DT_PIN) == HIGH) ? -1 : 1;
      encoderLastUs = now;
    }
  }
  encoderPrevClk = clk;
}

int8_t takeEncoderSteps() {
  noInterrupts();
  int8_t steps = encoderDelta;
  encoderDelta = 0;
  interrupts();
  return steps;
}

uint8_t wrapAdd(uint8_t value, int8_t delta, uint8_t count) {
  int result = (int(value) + delta) % count;
  return result < 0 ? result + count : result;
}

// ---------------------------------------------------------------------------
// Port pins: only the pin under test is driven; every other port pin is left
// floating so a shorted cable can never connect a HIGH output to a LOW one.
// The transmitter's pull-downs pull undriven conductors low.
// ---------------------------------------------------------------------------
void releasePins(const uint8_t *pins, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    pinMode(pins[i], INPUT);
  }
}

void releaseAllPorts() {
  releasePins(RJ45_PINS, 8);
  releasePins(BNC_PINS, 2);
  releasePins(RJ11_PINS, 2);
}

void driveOnly(uint8_t pin) {
  releaseAllPorts();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void showLeds(const char *pattern) {
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    digitalWrite(LED_PINS[i], pattern[i] == '1' ? HIGH : LOW);
  }
}

void ledsOff() {
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

void makePattern(uint8_t slot, char *out) {
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    out[i] = (i == slot) ? '1' : '0';
  }
  out[NUM_SLOTS] = '\0';
}

// ---------------------------------------------------------------------------
// Radio
// ---------------------------------------------------------------------------
bool readPacket(char *out) {
  char buf[32];
  radio.read(buf, sizeof(buf));
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    if (buf[i] != '0' && buf[i] != '1') {
      return false;
    }
    out[i] = buf[i];
  }
  out[NUM_SLOTS] = '\0';
  return true;
}

// First valid packet waiting in the RX FIFO.
bool readNextPacket(char *out) {
  while (radio.available()) {
    if (readPacket(out)) {
      return true;
    }
  }
  return false;
}

// Newest valid packet; drains the FIFO.
bool readLatestPacket(char *out) {
  bool got = false;
  char tmp[NUM_SLOTS + 1];
  while (radio.available()) {
    if (readPacket(tmp)) {
      memcpy(out, tmp, sizeof(tmp));
      got = true;
    }
  }
  return got;
}

// ---------------------------------------------------------------------------
// Evaluation
// ---------------------------------------------------------------------------
uint8_t countOnes(const char *reading) {
  uint8_t ones = 0;
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    if (reading[i] == '1') ones++;
  }
  return ones;
}

PinResult evaluate(uint8_t mode, const char *reading, uint8_t slot) {
  uint8_t ones = countOnes(reading);
  bool hit = reading[slot] == '1';

  switch (mode) {
    case MODE_CONTINUITY:
      return hit ? PIN_OK : PIN_OPEN;
    case MODE_SHORT:
      if (ones > 1) return PIN_SHORTED;
      return ones == 0 ? PIN_OPEN : PIN_OK;
    case MODE_WIRING:
    default:
      if (ones == 0) return PIN_OPEN;
      if (ones > 1) return PIN_SHORTED;
      return hit ? PIN_OK : PIN_CROSSED;
  }
}

// An open conductor is reported in the short test but does not fail it.
bool isFailure(uint8_t mode, PinResult result) {
  if (result == PIN_NO_DATA) return true;
  if (mode == MODE_SHORT) return result == PIN_SHORTED;
  return result != PIN_OK;
}

// Label of the port pin that showed up in the reading (for crossed wires).
int8_t labelForReading(const PortDef &port, const char *reading) {
  for (uint8_t i = 0; i < port.count; i++) {
    if (reading[port.slots[i]] == '1') return port.labels[i];
  }
  return -1;
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
void drawMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("MODE: "));
  display.println(MODE_MENU_NAMES[selectedMode]);
  display.setCursor(0, 20);
  display.print(F("PORT: "));
  display.println(PORTS[selectedPort].name);
  display.setCursor(0, 48);
  display.print(F("PRESS KNOB TO START"));
  display.display();
}

void drawColour(const PortDef &port, uint8_t index) {
  if (port.colours == nullptr) return;
  display.setCursor(0, 48);
  display.print(F("WIRCLR: "));
  display.print(port.colours[port.labels[index] - 1]);
}

void drawTestStep(const PortDef &port, uint8_t index, const char *reading, PinResult result) {
  uint8_t label = port.labels[index];

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(MODE_TITLES[selectedMode]);
  display.print(' ');
  if (lastSweep == SWEEP_PASS) display.print(F("PASS"));
  else if (lastSweep == SWEEP_FAIL) display.print(F("FAIL"));
  else display.print(F("..."));

  display.setCursor(0, 16);
  display.print(F("SENT: "));
  display.println(expected);
  display.print(F("RECV: "));
  display.println(result == PIN_NO_DATA ? "--------" : reading);

  display.setCursor(0, 40);
  if (result == PIN_NO_DATA) {
    display.print(F("NO DATA RECEIVED"));
  } else {
    display.print(F("PIN "));
    display.print(label);
    switch (result) {
      case PIN_OK:
        if (selectedMode == MODE_CONTINUITY) display.print(F(" CONNECTED"));
        else if (selectedMode == MODE_SHORT) display.print(F(" CLEAR"));
        else display.print(F(" OK"));
        break;
      case PIN_OPEN: display.print(F(" NOT CONNECTED")); break;
      case PIN_SHORTED: display.print(F(" SHORTED")); break;
      case PIN_CROSSED: {
        int8_t other = labelForReading(port, reading);
        display.print(F(" -> PIN "));
        if (other < 0) display.print('?');
        else display.print(other);
        break;
      }
      default: break;
    }
  }
  drawColour(port, index);
  display.display();
}

void drawManual() {
  const PortDef &port = PORTS[selectedPort];
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("MANUAL"));
  display.setCursor(0, 16);
  display.print(F("SENT: "));
  display.println(expected);
  display.print(F("RECV: "));
  display.println(manualHasData ? manualReading : "--------");
  display.setCursor(0, 40);
  if (manualHasData) {
    display.print(F("PIN: "));
    display.print(port.labels[manualIndex]);
  } else {
    display.print(F("NO DATA RECEIVED"));
  }
  drawColour(port, manualIndex);
  display.display();
}

void logStep(const PortDef &port, uint8_t index, const char *reading, PinResult result) {
  static const char *const RESULT_NAMES[] = { "OK", "OPEN", "SHORTED", "CROSSED", "NO DATA" };
  Serial.print(port.name);
  Serial.print(F(" pin "));
  Serial.print(port.labels[index]);
  Serial.print(F(" sent "));
  Serial.print(expected);
  Serial.print(F(" recv "));
  Serial.print(result == PIN_NO_DATA ? "--------" : reading);
  Serial.print(' ');
  Serial.println(RESULT_NAMES[result]);
}

// ---------------------------------------------------------------------------
// Test control
// ---------------------------------------------------------------------------
void stopTest() {
  if (!running) return;
  running = false;
  releaseAllPorts();
  ledsOff();
  menuDirty = true;
  Serial.println(F("Test stopped"));
}

void driveManualPin() {
  const PortDef &port = PORTS[selectedPort];
  makePattern(port.slots[manualIndex], expected);
  driveOnly(port.pins[manualIndex]);
  showLeds(expected);
  manualDirty = true;
}

void startTest() {
  running = true;
  Serial.print(MODE_MENU_NAMES[selectedMode]);
  Serial.print(F(" on "));
  Serial.println(PORTS[selectedPort].name);

  if (selectedMode == MODE_MANUAL) {
    manualIndex = 0;
    manualHasData = false;
    manualLastRx = millis();
    driveManualPin();
    return;
  }

  step = 0;
  phase = PHASE_DRIVE;
  sweepFailures = 0;
  lastSweep = SWEEP_NONE;
}

void finishStep(const PortDef &port, const char *reading, PinResult result) {
  if (isFailure(selectedMode, result)) sweepFailures++;
  drawTestStep(port, step, reading, result);
  logStep(port, step, reading, result);
  phase = PHASE_HOLD;
  phaseStart = millis();
}

void runAutomaticTest() {
  const PortDef &port = PORTS[selectedPort];
  unsigned long now = millis();

  switch (phase) {
    case PHASE_DRIVE:
      makePattern(port.slots[step], expected);
      driveOnly(port.pins[step]);
      showLeds(expected);
      phase = PHASE_SETTLE;
      phaseStart = now;
      break;

    case PHASE_SETTLE:
      // Anything already queued was measured before this pin was driven.
      if (now - phaseStart >= SETTLE_MS) {
        radio.flush_rx();
        phase = PHASE_AWAIT;
        phaseStart = now;
      }
      break;

    case PHASE_AWAIT: {
      char reading[NUM_SLOTS + 1];
      if (readNextPacket(reading)) {
        finishStep(port, reading, evaluate(selectedMode, reading, port.slots[step]));
      } else if (now - phaseStart >= RX_TIMEOUT_MS) {
        finishStep(port, "", PIN_NO_DATA);
      }
      break;
    }

    case PHASE_HOLD:
      if (now - phaseStart >= HOLD_MS) {
        step++;
        if (step >= port.count) {
          lastSweep = sweepFailures == 0 ? SWEEP_PASS : SWEEP_FAIL;
          Serial.println(lastSweep == SWEEP_PASS ? F("Sweep PASS") : F("Sweep FAIL"));
          sweepFailures = 0;
          step = 0;
        }
        phase = PHASE_DRIVE;
      }
      break;
  }
}

void runManual(int8_t steps) {
  const PortDef &port = PORTS[selectedPort];
  unsigned long now = millis();

  if (steps != 0) {
    manualIndex = wrapAdd(manualIndex, steps, port.count);
    driveManualPin();
  }

  char reading[NUM_SLOTS + 1];
  if (readLatestPacket(reading)) {
    manualLastRx = now;
    if (!manualHasData || strcmp(reading, manualReading) != 0) {
      memcpy(manualReading, reading, sizeof(reading));
      manualHasData = true;
      manualDirty = true;
    }
  } else if (manualHasData && now - manualLastRx >= MANUAL_STALE_MS) {
    manualHasData = false;
    manualDirty = true;
  }

  if (manualDirty) {
    manualDirty = false;
    drawManual();
  }
}

// ---------------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------------
void haltWithMessage(const __FlashStringHelper *message) {
  Serial.println(message);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(message);
  display.display();
  while (true) {
  }
}

void setup() {
  Serial.begin(9600);

  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  ledsOff();
  releaseAllPorts();

  knobButton.setDebounceTime(50);
  portButton.setDebounceTime(50);

  // reset=false: the SH1106 module's reset is not wired to the Mega
  display.begin(SH1106_SWITCHCAPVCC, 0x3C, false);
  Wire.setClock(400000);  // full-screen redraw ~25 ms instead of ~100 ms
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0, 24);
  display.print(F("   MULTIWAVE v2"));
  display.display();

  if (!radio.begin()) {
    haltWithMessage(F("RADIO NOT FOUND"));
  }
  radio.openReadingPipe(1, RADIO_ADDRESS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  pinMode(ENC_CLK_PIN, INPUT);
  pinMode(ENC_DT_PIN, INPUT);
  encoderPrevClk = digitalRead(ENC_CLK_PIN);
  PCMSK0 |= _BV(PCINT4);
  PCICR |= _BV(PCIE0);

  delay(1000);
  Serial.println(F("MultiWave receiver v2 ready"));
}

void loop() {
  knobButton.loop();
  portButton.loop();
  int8_t steps = takeEncoderSteps();
  bool knobPressed = knobButton.isPressed();
  bool portPressed = portButton.isPressed();

  if (running && selectedMode == MODE_MANUAL) {
    // In manual mode the knob picks the pin; pressing it returns to the menu.
    if (knobPressed) {
      stopTest();
    } else if (portPressed) {
      selectedPort = wrapAdd(selectedPort, 1, PORT_COUNT);
      manualIndex = 0;
      manualHasData = false;
      driveManualPin();
    } else {
      runManual(steps);
      return;
    }
  } else {
    // Turning the knob or changing port stops a running test.
    if (steps != 0) {
      stopTest();
      selectedMode = wrapAdd(selectedMode, steps, MODE_COUNT);
      menuDirty = true;
    }
    if (portPressed) {
      stopTest();
      selectedPort = wrapAdd(selectedPort, 1, PORT_COUNT);
      menuDirty = true;
    }
    if (knobPressed) {
      if (running) stopTest();
      else startTest();
    }
    if (running && selectedMode != MODE_MANUAL) {
      runAutomaticTest();
      return;
    }
  }

  if (running) return;
  if (menuDirty) {
    menuDirty = false;
    drawMenu();
  }
}
