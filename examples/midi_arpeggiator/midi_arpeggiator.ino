/*    Arpeggiator Synth for Adafruit Neotrellis M4
 *  by Collin Cunningham for Adafruit Industries, inspired by Stretta's Polygome
 *  https://www.adafruit.com/product/3938
 *
 *  Change color, scale, pattern, bpm, and waveform variables in settings.h file!
 *
 *  Midi USB Clock Sync by @cbmainz
 *  Sync to Midi clock if there is on
 *
 *  'chroma' coloring by @tapiralec
 *  
 *  removal of synth engine by @johnedgarpark 
 *
 */
#include <Adafruit_ADXL343.h>
#include <Adafruit_NeoTrellisM4.h>

#include "settings.h"

#define WIDTH      8
#define HEIGHT     4

#define N_BUTTONS       WIDTH*HEIGHT
#define ARP_NOTE_COUNT  6
#define NULL_INDEX      255

unsigned long prevReadTime = 0L; // Keypad polling timer
boolean pressed[N_BUTTONS] = {false};        // Pressed state for each button
uint8_t pitchMap[N_BUTTONS];
uint8_t arpSeqIndex[N_BUTTONS] = {NULL_INDEX};   // Current place in button arpeggio sequence
uint8_t arpButtonIndex[N_BUTTONS] = {NULL_INDEX};   // Button index being played for each actual pressed button

unsigned long beatInterval = 60000L / BPM; // ms/beat - should be merged w bpm in a function!
unsigned long prevArpTime  = 0L;
boolean holdActive = false;

//Pulse per quarter note. Each beat has 24 pulses.
int ppqn = 0;

int last_xbend = 0;
int last_ybend = 0;

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
Adafruit_ADXL343 accel = Adafruit_ADXL343(123, &Wire1);

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("Arp MIDI ...");

  trellis.begin();
  trellis.setBrightness(BRIGHTNESS);

  trellis.enableUSBMIDI(true);
  trellis.setUSBMIDIchannel(MIDI_CHANNEL);
  trellis.enableUARTMIDI(true);
  trellis.setUARTMIDIchannel(MIDI_CHANNEL);


  //Set up the notes for grid
  writePitchMap();

  trellis.fill(green);
  delay(500);
  trellis.fill(offColor);

  if (!accel.begin()) {
    Serial.println("No accelerometer found");
    while (1);
  }
}


void loop() {
  trellis.tick();
  midiEventPacket_t rx;

  unsigned long t = millis();
  unsigned long tDiff = t - prevReadTime;

  while (trellis.available()){
    keypadEvent e = trellis.read();
    uint8_t i = e.bit.KEY;
    if (e.bit.EVENT == KEY_JUST_PRESSED) {

      if (!HOLD_ENABLED){         //Normal mode
        pressed[i] = true;
      }
      else {                       //Hold/toggle mode
        if (pressed[i] == true){  //if button is active, deactivate
          pressed[i] = false;
          stopArp(i);
        }
        else {
          pressed[i] = true;   //if button is inactive, activate
        }
      }
    }

    else if (e.bit.EVENT == KEY_JUST_RELEASED) {
        if (!HOLD_ENABLED){       //Normal mode, responds to button release
          pressed[i] = false;
          stopArp(i);
        }
      }
  }

  // CLOCK
  do {
    rx = MidiUSB.read();

    // External CLOCK (via Midi USB)
    if(rx.byte1 == 0xF8){
       ++ppqn;

       if(ppqn == CLOCK_DIVISION){
          respondToPresses();
          MidiUSB.flush();
          ppqn = 0;
          prevArpTime = t;
       };
    }
    // INTERNAL CLOCK
    else {
       if ((t - prevArpTime) >= beatInterval) {
          respondToPresses();
          ppqn = 0;
          prevArpTime = t;
      }
    }
  } while (rx.header != 0);

  // Check for accelerometer
  sensors_event_t event;
  accel.getEvent(&event);
  /* Display the results (acceleration is measured in m/s^2) */
  //Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  //Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  //Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");
  int xbend = 0;
  int ybend = 0;
  bool changed = false;

  if (abs(event.acceleration.y) < 2.0) {  // 2.0 m/s^2
    // don't make any bend unless they've really started moving it
    ybend = 8192; // 8192 means no bend
  } else {
    if (event.acceleration.y > 0) {
      ybend = ofMap(event.acceleration.y, 2.0, 10.0, 8192, 0, true);  // 2 ~ 10 m/s^2 is downward bend
    } else {
      ybend = ofMap(event.acceleration.y, -2.0, -10.0, 8192, 16383, true);  // -2 ~ -10 m/s^2 is upward bend
    }
  }
  if (ybend != last_ybend) {
    Serial.print("Y pitchbend: "); Serial.println(ybend);
    trellis.pitchBend(ybend);
    last_ybend = ybend;
    changed = true;
  }

  if (abs(event.acceleration.x) < 2.0) {  // 2.0 m/s^2
    // don't make any bend unless they've really started moving it
    xbend = 0;
  } else {
    if (event.acceleration.x > 0) {
      xbend = ofMap(event.acceleration.x, 2.0, 10.0, 0, 127, true);  // 2 ~ 10 m/s^2 is upward bend
    } else {
      xbend = ofMap(event.acceleration.x, -2.0, -10.0, 0, 127, true);  // -2 ~ -10 m/s^2 is downward bend
    }
  }
  if (xbend != last_xbend) {
    Serial.print("X mod: "); Serial.println(xbend);
    trellis.controlChange(MIDI_XCC, xbend);
    last_xbend = xbend;
  }


  trellis.sendMIDI();

  prevReadTime = t;
}


void writePitchMap() {
  for (int i = 0; i < N_BUTTONS; i++) {
    int octMod = i/8 + OCTAVE;
    pitchMap[i] = SYNTH_SCALE[i%8] + (octMod*12);
  }
  
}

void respondToPresses() {
  for (uint8_t i = 0; i < N_BUTTONS; i++) {
    if (pressed[i]) {
      playArp(i);
    }
  }
}


void playArp(uint8_t buttonIndex) {
  uint8_t seqIndex, seqButtonIndex, seqNote, x, y;

  //if not starting arp, then increment index
  if (arpSeqIndex[buttonIndex] == NULL_INDEX) seqIndex = 0;
  else seqIndex = arpSeqIndex[buttonIndex] + 1;

  // Loop sequence
  if (seqIndex >= ARP_NOTE_COUNT) {
    seqIndex = 0;
  }

  // Find current button coordinates
  y = buttonIndex / WIDTH;
  x = buttonIndex - (y * WIDTH);

  // Reference to root button for Hold mode
  uint8_t rootY = y;
  uint8_t rootX = x;

  // Add note offsets
  x = x + ARPEGGIATOR_PATTERN[seqIndex][0];
  y = y + ARPEGGIATOR_PATTERN[seqIndex][1];

  // Wrap notes to grid
  if (x >= WIDTH)  x %= WIDTH;
  if (y >= HEIGHT) y %= HEIGHT;

  // Find new note and index
  seqNote = findNoteFromXY(x, y);
  seqButtonIndex = indexFromXY(x, y);

  // Stop prev note in sequence
  stopNoteForButton(arpButtonIndex[buttonIndex]);

  // Store new note
  arpSeqIndex[buttonIndex] = seqIndex;
  arpButtonIndex[buttonIndex] = seqButtonIndex;

  // Play new note
  playNoteForButton(seqButtonIndex);

  // If in Hold mode, light root button
  if (HOLD_ENABLED) trellis.setPixelColor(indexFromXY(rootX, rootY), holdColor);

}

void stopArp(uint8_t button) {
  // Stop playing the note
  stopNoteForButton(arpButtonIndex[button]);

  // Store an invalid button index in its place
  arpSeqIndex[button] = NULL_INDEX;  //check for invalid

  // If in Hold mode, light root button
  if (HOLD_ENABLED) trellis.setPixelColor(button, offColor);
}

uint8_t indexFromXY(uint8_t x, uint8_t y) {
  return (y * WIDTH + x);
}

uint8_t findNoteFromXY(uint8_t x, uint8_t y) {
  return pitchMap[y * WIDTH + x];
}

uint8_t findNoteFromIndex(uint8_t buttonIndex) {
  uint8_t x, y;
  y = buttonIndex / WIDTH;
  x = buttonIndex - (y * WIDTH);

  return findNoteFromXY(x, y);
}

void playNoteForButton(uint8_t buttonIndex) {
  trellis.noteOn(findNoteFromIndex(buttonIndex), 100);
  // trellis.setPixelColor(buttonIndex, onColor);
  trellis.setPixelColor(buttonIndex, Wheel((buttonIndex % 8)*(255/8)));
}

void stopNoteForButton(uint8_t buttonIndex) {
  trellis.noteOff(findNoteFromIndex(buttonIndex), 0);
  trellis.setPixelColor(buttonIndex, offColor);
}

void debugLed(bool light) {
  if (light)
    trellis.setPixelColor(0, blue);
  else
    trellis.setPixelColor(0, 0);
}

// floating point map
float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
  float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);

  if (clamp) {
    if (outputMax < outputMin) {
      if (outVal < outputMax)  outVal = outputMax;
      else if (outVal > outputMin)  outVal = outputMin;
    } else {
      if (outVal > outputMax) outVal = outputMax;
      else if (outVal < outputMin)  outVal = outputMin;
    }
  }
  return outVal;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return trellis.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return trellis.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return trellis.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
