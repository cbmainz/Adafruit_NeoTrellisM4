// Trellis M4 MIDI Keypad CC
// inspired by the Arpeggiator Synth example by Collin Cunningham
// sends 32 notes, pitch bend & a CC from accelerometer tilt over USB MIDI

#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>
#include <Adafruit_NeoTrellisM4.h>

#define NULL_INDEX 255
#define WIDTH      8
#define HEIGHT     4
#define N_BUTTONS  WIDTH*HEIGHT

//misc//////////////////////////
#define MIDI_CHANNEL     0  // default MIDI channel is 1
#define MIDI_OUT      true  //enables MIDI output
#define OCTAVE           2   //determines note pitch
#define SYNTH_SCALE      dorian_scale    // see below, dorian, ionian, etc!
#define MIDI_XCC         1 //choose a CC number to control with x axis tilting of the board. 1 is mod wheel, for example.

// Musical modes / scales
uint8_t dorian_scale[] = { 0, 2, 3, 5, 7, 9, 10, 12 }; //dorian
uint8_t ionian_scale[] = { 0, 2, 4, 5, 7, 9, 11, 12 };  //ionian
uint8_t phrygian_scale[] = { 0, 1, 2, 3, 5, 7,  8, 10,};  //phrygian
uint8_t lydian_scale[] = { 0, 2, 4, 6, 7, 9, 10, 11 };  //lydian
uint8_t mixolydian_scale[] = { 0, 2, 4, 5, 7, 9, 10, 12 }; //mixolydian
uint8_t aeolian_scale[] = { 0, 2, 3, 5, 7, 8, 10, 12 }; //aeolian
uint8_t locrian_scale[] = { 0, 1, 3, 5, 6, 8, 10, 12 }; //locrian

//colors//////////////////////////
uint32_t white =   0xFFFFFF;
uint32_t red =     0xFF0000;
uint32_t blue =    0x0000FF;
uint32_t green =   0x00FF00;
uint32_t teal =    0x00FFFF;
uint32_t magenta = 0xFF00FF;
uint32_t yellow =  0xFFFF00;
uint32_t off =     0x000000;
uint32_t offColor = yellow;   //color of deactivated buttons
uint32_t onColor = red; //color of activated buttons

unsigned long prevReadTime = 0L; // Keypad polling timer
uint8_t       quantDiv = 8;      // Quantization division, 2 = half note
uint8_t       clockPulse = 0;

//#define QUANT_PULSE (96/quantDiv)// Number of pulses per quantization division

boolean pressed[N_BUTTONS] = {false};        // Pressed state for each button
uint8_t pitchMap[N_BUTTONS];
uint8_t arpSeqIndex[N_BUTTONS] = {NULL_INDEX};   // Current place in button arpeggio sequence
uint8_t arpButtonIndex[N_BUTTONS] = {NULL_INDEX};   // Button index being played for each actual pressed button

int last_xbend = 0;
int last_ybend = 0;

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
Adafruit_ADXL343 accel = Adafruit_ADXL343(123, &Wire1);

void setup(){
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("MIDI keypad & pitchbend!");
    
  trellis.begin();
  trellis.setBrightness(32);
  if (MIDI_OUT) {
    trellis.enableUSBMIDI(true);
    trellis.setUSBMIDIchannel(MIDI_CHANNEL);
    trellis.enableUARTMIDI(true);
    trellis.setUARTMIDIchannel(MIDI_CHANNEL);
  }
  
  //Set up the notes for grid
  writePitchMap();

  trellis.fill(offColor);

  if(!accel.begin()) {
    Serial.println("No accelerometer found");
    while(1);
  }
}

  
void loop() {
  trellis.tick();

  unsigned long t = millis();
  unsigned long tDiff = t - prevReadTime;

  while (trellis.available()){
    keypadEvent e = trellis.read();
    uint8_t i = e.bit.KEY;
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      pressed[i] = true;     
    }
    else if (e.bit.EVENT == KEY_JUST_RELEASED) {
        pressed[i] = false;
        stopArp(i);
      }
  }

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
    if (MIDI_OUT) {
      trellis.pitchBend(ybend);
    }
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
    if (MIDI_OUT) {
      trellis.controlChange(MIDI_XCC, xbend);
    }
    last_xbend = xbend;
  }

  if (MIDI_OUT) {
    trellis.sendMIDI();
  }
  
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


  // Find current button coordinates
  y = buttonIndex / WIDTH;
  x = buttonIndex - (y * WIDTH);

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
  if (MIDI_OUT) {
    trellis.noteOn(findNoteFromIndex(buttonIndex), 100);
  }
  else {
    noteOn(findNoteFromIndex(buttonIndex), buttonIndex);
  }
  trellis.setPixelColor(buttonIndex, onColor);
}


void stopNoteForButton(uint8_t buttonIndex) {

  if (MIDI_OUT) {
    trellis.noteOff(findNoteFromIndex(buttonIndex), 0);
  }
  else {
    noteOff(findNoteFromIndex(buttonIndex), buttonIndex);
  }

  trellis.setPixelColor(buttonIndex, offColor);
}

void debugLed(bool light){
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
