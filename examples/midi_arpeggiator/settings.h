
//misc//////////////////////////
#define OCTAVE           2  // determines note pitch
#define BPM              120  // set tempo if no midi sync possible 
#define MIDI_CHANNEL     0  // default channel (0-15) # is 0
#define SYNTH_SCALE      mixolydian_scale    // see below, dorian, ionian, etc!
#define CLOCK_DIVISION   12  // 24 = quarter note, 12 = eighth notes, 6 = sixteenth notes, 8 = eight note triplets
//choose a CC number to control with x axis tilting of the board. 1 is mod wheel, for example.
#define MIDI_XCC         1
#define ARPEGGIATOR_PATTERN jump_arp // see below, many shapes to choose from!
#define HOLD_ENABLED     1  // Buttons are toggled on & off instead of only on while pressed, O = off, 1 = on
#define BRIGHTNESS      16  // set Brightness of Neopixels (0-225)

//colors//////////////////////////
uint32_t white =   0xFFFFFF;
uint32_t red =     0xFF0000;
uint32_t blue =    0x0000FF;
uint32_t green =   0x00FF00;
uint32_t teal =    0x00FFFF;
uint32_t magenta = 0xFF00FF;
uint32_t yellow =  0xFFFF00;
uint32_t off =     0x000000;
uint32_t offColor = off;   //color of deactivated buttons
uint32_t onColor = magenta; //color of activated buttons
uint32_t holdColor = white; //color for toggled buttons in Hold mode

// Musical modes / scales
uint8_t dorian_scale[] = { 0, 2, 3, 5, 7, 9, 10, 12 }; //dorian
uint8_t ionian_scale[] = { 0, 2, 4, 5, 7, 9, 11, 12 };  //ionian
uint8_t phrygian_scale[] = { 0, 1, 2, 3, 5, 7,  8, 10,};  //phrygian
uint8_t lydian_scale[] = { 0, 2, 4, 6, 7, 9, 10, 11 };  //lydian
uint8_t mixolydian_scale[] = { 0, 2, 4, 5, 7, 9, 10, 12 }; //mixolydian
uint8_t aeolian_scale[] = { 0, 2, 3, 5, 7, 8, 10, 12 }; //aeolian
uint8_t locrian_scale[] = { 0, 1, 3, 5, 6, 8, 10, 12 }; //locrian

//pattern//////////////////////////

int8_t dipper_arp[6][2] = { //lil dipper
  {  0,  0 },
  {  1, 0 },
  {  2, 0 },
  {  2,  1 },
  {  1,  1 },
  {  1,  0 },
};

int8_t circle_arp[6][2] = { //circle
  {  0,  0 },
  {  0, -1 },
  {  1,  0 },
  {  0,  1 },
  { -1,  0 },
  {  0, -1 },
};

int8_t onenote_arp[6][2] = { //one note
  {  0,  0 },
  {  0,  0 },
  {  0,  0 },
  {  0,  0 },
  {  0,  0 },
  {  0,  0 },
};


int8_t square_arp[6][2] = { //square
  {  0,  0 },
  { -1, -1 },
  {  1, -1 },
  {  1,  1 },
  { -1,  1 },
  { -1, -1 },
};


int8_t sshape_arp[6][2] = { //S-shape
  {  0,  0 },
  {  0, -1 },
  {  1, -1 },
  {  0,  0 },
  {  0,  1 },
  { -1,  1 },
};

int8_t tshape_arp[6][2] = { //T-shape
  {  0,  0 },
  {  1, 0 },
  {  2, 0 },
  {  2,  -1 },
  {  2,  0 },
  {  2,  1 },
};


int8_t effiks_arp[6][2] = { // effiks
  {  0,  0 },
  {  1,  1 },
  {  2,  2 },
  {  0,  0 },
  {  1,  -1 },
  {  2,  -2 },
};

int8_t backandforth_arp[6][2] = { // back and forth
  {  0,  0 },
  {  1,  0 },
  {  2,  0 },
  {  3,  0 },
  {  2,  0 },
  {  1,  0 },
};

int8_t double_arp[6][2] = { // double
  {  0,  0 },
  {  0,  0 },
  {  1,  0 },
  {  1,  0 },
  {  -1,  0 },
  {  -1,  0 },
};

int8_t jump_arp[6][2] = { // jump
  {  0,  0 },
  {  2,  1 },
  {  4,  0 },
  {  6,  1 },
  {  8,  0 },
  {  2,  0 },
};

int8_t kit_arp[6][2] = { // kit
  {  0,  0 },
  {  2,  0 },
  {  4,  0 },
  {  6,  0 },
  {  8,  0 },
  {  10,  0 },
};
