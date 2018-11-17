#include "Adafruit_NeoTrellisM4.h"


static const byte NEO_PIN = 10;

static const byte ROWS = 4; // four rows
static const byte COLS = 8; // eight columns

//define the symbols on the buttons of the keypads
static byte trellisKeys[ROWS][COLS] = {
  {0, 1,  2,  3,  4,  5,  6,  7},
  {8, 9,  10, 11, 12, 13, 14, 15},
  {16, 17, 18, 19, 20, 21, 22, 23},
  {24, 25, 26, 27, 28, 29, 30, 31}
};
static byte rowPins[ROWS] = {14, 15, 16, 17}; //connect to the row pinouts of the keypad
static byte colPins[COLS] = {2, 3, 4, 5, 6, 7, 8, 9}; //connect to the column pinouts of the keypad

Adafruit_NeoTrellisM4::Adafruit_NeoTrellisM4(void) :
  Adafruit_Keypad(makeKeymap(trellisKeys), rowPins, colPins, ROWS, COLS),
  Adafruit_NeoPixel_ZeroDMA(ROWS*COLS, NEO_PIN, NEO_GRB)
{
  _num_keys = ROWS * COLS;
  _rows = ROWS;
  _cols = COLS;
  _midi_channel_usb = 0;
  _midi_channel_usb = 0;
  _pending_midi = false;
}

void Adafruit_NeoTrellisM4::begin(void) {
  Adafruit_Keypad::begin();

  // Initialize all pixels to 'off'
  Adafruit_NeoPixel::begin();
  fill(0x0);
  Adafruit_NeoPixel::show();
  Adafruit_NeoPixel::setBrightness(255);
}

void Adafruit_NeoTrellisM4::setPixelColor(uint32_t pixel, uint32_t color) {
  Adafruit_NeoPixel::setPixelColor(pixel, color);
  Adafruit_NeoPixel::show();
}

void Adafruit_NeoTrellisM4::fill(uint32_t color) {
  for (int i=0; i<ROWS*COLS; i++) {
    Adafruit_NeoPixel::setPixelColor(i, color);
  }
  Adafruit_NeoPixel::show();
}

void Adafruit_NeoTrellisM4::tick(void)
{
  Adafruit_Keypad::tick();
  // look for an entire column being pressed at once and if it was, clear the whole buffer
  uint8_t rcount[] = {0, 0, 0, 0, 0, 0, 0, 0};
  for(int i=0; i<(COLS*ROWS)-1; i++){
    if (Adafruit_Keypad::justPressed(i+1, false))
      rcount[i%COLS]++;
  }
  for (int i=0; i<COLS; i++){
    if (rcount[i] >= ROWS){
      Adafruit_Keypad::clear();
      break;
    }
  }
}


void Adafruit_NeoTrellisM4::setUSBMIDIchannel(uint8_t c) {
  _midi_channel_usb = min(15, c);  // channel can only be between 0-15;
}

void Adafruit_NeoTrellisM4::setUARTMIDIchannel(uint8_t c) {
  _midi_channel_uart = min(15, c);  // channel can only be between 0-15;
}


void Adafruit_NeoTrellisM4::noteOn(byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | _midi_channel_usb, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  _pending_midi = true;
}

void Adafruit_NeoTrellisM4::noteOff(byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | _midi_channel_usb, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  _pending_midi = true;
}

void Adafruit_NeoTrellisM4::pitchBend(int value) {
  byte lowValue =  min(value & 0x7F, 127);
  byte highValue = min(value >> 7, 127);
  midiEventPacket_t pitchBend = {0x0E, 0xE0 | _midi_channel_usb, lowValue, highValue};
  MidiUSB.sendMIDI(pitchBend);
  _pending_midi = true;
}

void Adafruit_NeoTrellisM4::controlChange(byte control, byte value) {
  value = min(0x7F, value);
  control = min(0x7F, control);
  midiEventPacket_t event = {0x0B, 0xB0 | _midi_channel_usb, control, value};
  MidiUSB.sendMIDI(event);
  _pending_midi = true;
}

void Adafruit_NeoTrellisM4::sendMIDI(void) {
  if (_pending_midi) {
    MidiUSB.flush();
    _pending_midi = false;
  }
}
