
//misc//////////////////////////
#define OCTAVE           4  // determines note pitch
#define MIDI_OUT      true  // enables MIDI output & disables synth
#define MIDI_CHANNEL     0  // default channel (0-15) # is 1
#define SYNTH_SCALE      locrian_scale    // see below, dorian, ionian, etc!
//choose a CC number to control with x axis tilting of the board. 1 is mod wheel, for example.
#define MIDI_XCC         1
#define BRIGHTNESS      32  // set Brightness of Neopixels (0-225)

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

//waveform//////////////////////////
int wave = 0;  // 0=sine, 1= square, 2= sawtooth, 3 = triangle


// Musical modes / scales
uint8_t dorian_scale[] = { 0, 2, 3, 5, 7, 9, 10, 12 }; //dorian
uint8_t ionian_scale[] = { 0, 2, 4, 5, 7, 9, 11, 12 };  //ionian
uint8_t phrygian_scale[] = { 0, 1, 2, 3, 5, 7,  8, 10,};  //phrygian
uint8_t lydian_scale[] = { 0, 2, 4, 6, 7, 9, 10, 11 };  //lydian
uint8_t mixolydian_scale[] = { 0, 2, 4, 5, 7, 9, 10, 12 }; //mixolydian
uint8_t aeolian_scale[] = { 0, 2, 3, 5, 7, 8, 10, 12 }; //aeolian
uint8_t locrian_scale[] = { 0, 1, 3, 5, 6, 8, 10, 12 }; //locrian
