/**
 * Frankenkorg
 * Copyright (c) 2017 by Alec Smecher
 * See LICENSE for license details.
 */

// MIDI commands; see e.g. http://computermusicresource.com/MIDI.Commands.html
const byte MIDI_NOTE_OFF = 0x80;
const byte MIDI_NOTE_ON = 0x90;
const byte MIDI_MODULATION = 0xb0;
const byte MIDI_BEND = 0xe0;
const byte MIDI_RESET = 0x99;

// Midi key number offsets for octave settings; see e.g. http://computermusicresource.com/midikeys.html
const byte MIDI_OCTAVE_OFFSET_L = 0;
const byte MIDI_OCTAVE_OFFSET_M = 36;
const byte MIDI_OCTAVE_OFFSET_H = 72;

// Internal constants (code-only)
const byte LED_BANK_RED = 0;
const byte LED_BANK_GREEN = 1;

/**
 * Pin assignments
 */
// Output pins driving 3-to-8 demux for the green LED bank
const byte LED_BANK_GREEN_PIN_0 = 2;
const byte LED_BANK_GREEN_PIN_1 = 1;
const byte LED_BANK_GREEN_PIN_2 = 0;

// Output pins driving the 3-to-8 demux for the red LED bank
const byte LED_BANK_RED_PIN_0 = 12;
const byte LED_BANK_RED_PIN_1 = 11;
const byte LED_BANK_RED_PIN_2 = 10;

// Output pins driving the 3-to-8 demux for keypad scanning
const byte KEYPAD_OUT_PIN_0 = 14;
const byte KEYPAD_OUT_PIN_1 = 13;
const byte KEYPAD_OUT_PIN_2 = 23;

// Input bus for reading keypads
const byte KEYPAD_IN_PIN_0 = 15;
const byte KEYPAD_IN_PIN_1 = 16;
const byte KEYPAD_IN_PIN_2 = 17;
const byte KEYPAD_IN_PIN_3 = 18;
const byte KEYPAD_IN_PIN_4 = 19;
const byte KEYPAD_IN_PIN_5 = 20;
const byte KEYPAD_IN_PIN_6 = 21;
const byte KEYPAD_IN_PIN_7 = 22;

// Values to feed to the keypad demux to test each bank
const byte KEYPAD_BANK_PROGRAM = 7;
const byte KEYPAD_BANK_NECK = 6;
const byte KEYPAD_KEYBOARD_PINS[] = {5, 4, 3, 2, 1, 0};

// Bits to test for individual switches/buttons
// - When reading KEYPAD_BANK_NECK
const byte KEYPAD_BANK_NECK_DOWN = 1;
const byte KEYPAD_BANK_NECK_UP = 2;
const byte KEYPAD_BANK_NECK_OCTAVE_L = 4;
const byte KEYPAD_BANK_NECK_OCTAVE_M = 8;
const byte KEYPAD_BANK_NECK_OCTAVE_H = 16;
const byte KEYPAD_BANK_NECK_DESTINATION_2 = 64;
const byte KEYPAD_BANK_NECK_DESTINATION_1 = 32;
const byte KEYPAD_BANK_NECK_SHIFT = 128;
// - When reading KEYPAD_BANK_PROGRAM
const byte KEYPAD_BANK_PROGRAM_1 = 128;
const byte KEYPAD_BANK_PROGRAM_2 = 64;
const byte KEYPAD_BANK_PROGRAM_3 = 32;
const byte KEYPAD_BANK_PROGRAM_4 = 16;
const byte KEYPAD_BANK_PROGRAM_5 = 8;
const byte KEYPAD_BANK_PROGRAM_6 = 4;
const byte KEYPAD_BANK_PROGRAM_7 = 2;
const byte KEYPAD_BANK_PROGRAM_8 = 1;

// Pin numbers for analog inputs
const byte NECK_BEND_PIN = 25;
const byte NECK_VOLUME = 27;
const byte NECK_MG_PIN = 26;

// Number of loops before a switch press can be considered debounced.
const byte DEBOUNCE_TIME = 200;

/**
 * Set which LED is lit for a particular bank.
 * - bank: LED_BANK_RED or LED_BANK_GREEN
 * - value: Which LED to light, 0-7
 */
void setLeds(byte bank, byte value) {
  switch (bank) {
    case LED_BANK_RED:
      value = 7-value; // This bank is wired in reverse for some reason
      digitalWrite(LED_BANK_RED_PIN_0, value & 1);
      digitalWrite(LED_BANK_RED_PIN_1, value & 2);
      digitalWrite(LED_BANK_RED_PIN_2, value & 4);
      break;
    case LED_BANK_GREEN:
      digitalWrite(LED_BANK_GREEN_PIN_0, value & 1);
      digitalWrite(LED_BANK_GREEN_PIN_1, value & 2);
      digitalWrite(LED_BANK_GREEN_PIN_2, value & 4);
      break;
  }
}

/**
 * Read a bank of keys.
 * - bank: Which bank to read.
 *   Use KEYPAD_BANK_PROGRAM, KEYPAD_BANK_NECK, or KEYPAD_KEYBOARD_PINS[x]
 * Returns a byte-sized bitfield indicating which keys are pressed.
 */
unsigned char getKeys(byte bank) {
  digitalWrite(KEYPAD_OUT_PIN_0, bank & 1);
  digitalWrite(KEYPAD_OUT_PIN_1, bank & 2);
  digitalWrite(KEYPAD_OUT_PIN_2, bank & 4);
  return
    (digitalRead(KEYPAD_IN_PIN_0)?0:1) +
    (digitalRead(KEYPAD_IN_PIN_1)?0:2) +
    (digitalRead(KEYPAD_IN_PIN_2)?0:4) +
    (digitalRead(KEYPAD_IN_PIN_3)?0:8) +
    (digitalRead(KEYPAD_IN_PIN_4)?0:16) +
    (digitalRead(KEYPAD_IN_PIN_5)?0:32) +
    (digitalRead(KEYPAD_IN_PIN_6)?0:64) +
    (digitalRead(KEYPAD_IN_PIN_7)?0:128);
}

void setup() {
  // Configure I/O pins
  pinMode(LED_BANK_GREEN_PIN_0, OUTPUT);
  pinMode(LED_BANK_GREEN_PIN_1, OUTPUT);
  pinMode(LED_BANK_GREEN_PIN_2, OUTPUT);

  pinMode(LED_BANK_RED_PIN_0, OUTPUT);
  pinMode(LED_BANK_RED_PIN_1, OUTPUT);
  pinMode(LED_BANK_RED_PIN_2, OUTPUT);

  pinMode(KEYPAD_OUT_PIN_0, OUTPUT);
  pinMode(KEYPAD_OUT_PIN_1, OUTPUT);
  pinMode(KEYPAD_OUT_PIN_2, OUTPUT);

  pinMode(KEYPAD_IN_PIN_0, INPUT);
  pinMode(KEYPAD_IN_PIN_1, INPUT);
  pinMode(KEYPAD_IN_PIN_2, INPUT);
  pinMode(KEYPAD_IN_PIN_3, INPUT);
  pinMode(KEYPAD_IN_PIN_4, INPUT);
  pinMode(KEYPAD_IN_PIN_5, INPUT);
  pinMode(KEYPAD_IN_PIN_6, INPUT);
  pinMode(KEYPAD_IN_PIN_7, INPUT);

  // MIDI configuration
  Serial.begin(31250);
  Serial.write(MIDI_RESET);
}

// Control states from the last loop iteration
byte lastKeys[sizeof(KEYPAD_KEYBOARD_PINS)];
byte lastNeckKeys = 0;
byte lastProgramKeys = 0;
int lastBend = 511; // Middle (i.e. no bend)
byte lastModulation = 0;

// Current states
byte bank = 0; // 0 to 7
byte program = 0; // 0 to 7

// Current debounce counter. Nonzero indicates debounce state.
byte debounce=0;


void loop() {
  /**
   * Neck control interaction handling
   */
  byte neckKeys = getKeys(KEYPAD_BANK_NECK);
  byte programKeys = getKeys(KEYPAD_BANK_PROGRAM);
  if (debounce==0) {
    // Program up/down keys
    if (neckKeys & KEYPAD_BANK_NECK_UP && !(lastNeckKeys & KEYPAD_BANK_NECK_UP)) {
        program = (program + 1) % 8;
        setLeds(LED_BANK_RED, program);
        debounce=DEBOUNCE_TIME;
    }
    if (neckKeys & KEYPAD_BANK_NECK_DOWN && !(lastNeckKeys & KEYPAD_BANK_NECK_DOWN)) {
        program = (program - 1) % 8;
        setLeds(LED_BANK_RED, program);
        debounce=DEBOUNCE_TIME;
    }
    if (neckKeys & KEYPAD_BANK_NECK_SHIFT && !(lastNeckKeys & KEYPAD_BANK_NECK_SHIFT)) {
        bank = (bank + 1) % 8;
        setLeds(LED_BANK_GREEN, bank);
        debounce=DEBOUNCE_TIME;
    }

    // Program direct access keys
    byte p = 128;
    for (byte i=0; i<8; i++) {
      if (programKeys & p && !(lastNeckKeys & p)) {
        program = i;
        setLeds(LED_BANK_RED, program);
        debounce=DEBOUNCE_TIME;
      }
      p /= 2; // Shift right
    }
    lastNeckKeys = neckKeys;
    lastProgramKeys = programKeys;
  }

  // Finished handling debounced controls. Decrement debounce counter if necessary.
  if (debounce>0) debounce--;

  /**
   * Bend handling
   */
  int bend = (analogRead(NECK_BEND_PIN)+6) * 16;
  if (lastBend != bend) {
    byte hi = bend / 128;
    byte lo = bend % 128;
    Serial.write(MIDI_BEND + program);
    Serial.write(lo);
    Serial.write(hi);
  }
  lastBend = bend;

  /**
   * Modulation handling
   */
  int modulation = analogRead(NECK_MG_PIN) / 8;
  if (lastModulation != modulation) {
    Serial.write(MIDI_MODULATION + program);
    Serial.write(neckKeys & KEYPAD_BANK_NECK_DESTINATION_1?1:2);
    Serial.write(modulation);
  }
  lastModulation = modulation;

  /**
   * Keyboard interaction handling
   */
  byte keys[sizeof(KEYPAD_KEYBOARD_PINS)];
  byte volume = analogRead(NECK_VOLUME_PIN) / 8;

  // Read the key states
  for (byte i=0; i<sizeof(KEYPAD_KEYBOARD_PINS); i++) {
    keys[i] = getKeys(KEYPAD_KEYBOARD_PINS[i]);
  }

  // Compare the key states with the last iteration
  byte keyCounter = 0;
  for (byte i=0; i<sizeof(KEYPAD_KEYBOARD_PINS); i++) {
    byte p = 1;
    for (byte j=0; j<8; j++) {
      bool currentState = keys[i] & p;
      bool lastState = lastKeys[i] & p;
      if (lastState != currentState) { // The state of this key has changed
        byte key = keyCounter;
        if (neckKeys & KEYPAD_BANK_NECK_OCTAVE_L) key += MIDI_OCTAVE_OFFSET_L;
        else if (neckKeys & KEYPAD_BANK_NECK_OCTAVE_M) key += MIDI_OCTAVE_OFFSET_M;
        else /* KEYPAD_BANK_NECK_OCTAVE_H */ key += MIDI_OCTAVE_OFFSET_H;
        Serial.write(MIDI_NOTE_ON + program);
        Serial.write(key);
        Serial.write(currentState?volume:0);
      }
      p*=2; // Shift left
      keyCounter++;
    }
  }

  // Save the state for the next iteration
  for (byte i=0; i<sizeof(KEYPAD_KEYBOARD_PINS); i++) {
    lastKeys[i] = keys[i];
  }
}
