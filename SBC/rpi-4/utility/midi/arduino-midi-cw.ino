
/*
 *  MIDI interface for connecting a paddle or straight key to 
 *  a computer running SDR software.
 *  
 *  We monitor three digital inputs (active low), whose state changes
 *  are reported with MIDI NoteOn/NoteOff messages, for
 *  
 *  left CW paddle
 *  right CW paddle
 *  PTT
 *  
 *  All input lines are active-low with an internal pullup, that is, they have
 *  to be shortened to ground.
 *  
 *  Of course, the PTT line can also be used for manually set/clear PTT in SSB QSOs.
 */


  /*
   * defind on of the two following symbols:
   * 
   * ARDUINO_MOCOLUFA:   code for Arduino UNO etc.
   * TEENSY_USBMIDI:     code for TEENSY
   */
#define ARDUINO_PRO_MICRO

#ifdef ARDUINO_PRO_MICRO   
  #include "MIDIUSB.h"
  
  #define PaddleLeft    2   // Digital input for left paddle
  #define PaddleRight   3   // Digital input for right paddle
  #define PTTinput      4   // Digital input for PTT line
#endif

#ifdef ARDUINO_MOCOLUFA   
#define PaddleLeft    2   // Digital input for left paddle
#define PaddleRight   3   // Digital input for right paddle
#define PTTinput      4   // Digital input for PTT line
#endif

#ifdef TEENSY_USBMIDI
#define PaddleLeft     0  // PIN_B0
#define PaddleRight    1  // PIN_B1
#define PTTinput       2  // PIN_B2
#endif

#ifndef PaddleLeft
#error either define ARDUINO_MOCOLUFA or TEENSY_USBMIDI
#endif

/*
 * MIDI notes for CW keys, PTT, and CW speed
 */
#define CWL_KEY 32
#define CWR_KEY 33
#define PTT     34

/*
 * Each input pin is associated with such a struct
 */
struct input {
   int state;               // current state of input line
   unsigned long last;      // time stamp of last change
   int pin;                 // Pin No. for digit
   int key;                 // MIDI key assigned
   int chan;                // MIDI channel assigned. Range 1 ... 16!
};


static struct input cwl   = {1, 0, PaddleLeft,  CWL_KEY,   5};
static struct input cwr   = {1, 0, PaddleRight, CWR_KEY,   5};
static struct input ptt   = {1, 0, PTTinput,    PTT,       5};


static struct input *digital_inputs[]={&cwl, &cwr, &ptt};

#define NUM_DIGITAL sizeof(digital_inputs) / sizeof(struct input *)

void setup() {
  unsigned int i;
#ifdef ARDUINO_PRO_MICRO 
  Serial.begin(31250);
#endif 
#ifdef ARDUINO_MOCOLUFA
  Serial.begin(31250);
#endif  
  for (i=0; i<NUM_DIGITAL; i++) {
    pinMode(digital_inputs[i]->pin, INPUT_PULLUP);
  }
}

#ifdef ARDUINO_MOCOLUFA
static void NoteOn(int note, int chan) {
  Serial.write(0x90 | (--chan & 0x0F));
  Serial.write(note);
  Serial.write(127);
}

static void NoteOff(int note, int chan) {
  Serial.write(0x80 | (--chan & 0x0F));
  Serial.write(note);
  Serial.write((byte)0);
}
#endif

#ifdef ARDUINO_PRO_MICRO 
static void NoteOn(byte note, byte chan) {
  midiEventPacket_t noteOn = {0x09, 0x90 | --chan, note, 64};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

static void NoteOff(byte note, byte chan) {
  midiEventPacket_t noteOff = {0x08, 0x80 | --chan, note, 64};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}
#endif

#ifdef TEENSY_USBMIDI
//
// usbMIDI uses MIDIspeak
//
static void NoteOn(int note, int chan) {
  usbMIDI.sendNoteOn(note, 127, chan);
  usbMIDI.send_now();
}

static void NoteOff(int note, int chan){
  usbMIDI.sendNoteOff(note, 0, chan);
  usbMIDI.send_now();
}
#endif


//
// This checks a digital input. It debounces the DigitalInput pin
// associated with the input, sets the time stamp and sends
// the MIDI message if the state has changed
//

static void CheckDigital(unsigned long now, struct input *inp) {
  int new_state;
  if (now <= inp->last+15) return;        // Debounce with 15 msec
  new_state=digitalRead(inp->pin);        // Read digital input
  if (new_state == inp->state) return;    // Nothing has changed
  inp->state = new_state;                 // record new state
  inp->last = now;                        // set time stamp
  if (inp->state ) {
    NoteOff(inp->key, inp->chan);         // ActiveLow: send NoteOff when input line is HIGH
  } else {
    NoteOn(inp->key, inp->chan);          // ActiveLow: send NoteOn  when input line is LOW
  }
}


void loop() {
  unsigned long now=millis();
  unsigned int i;
  static unsigned long lastread=0;

  for (i=0; i<NUM_DIGITAL; i++) CheckDigital(now, digital_inputs[i]);

//
// Every 20 msec, read and discard any possible incoming message
// This is because we file the loop() at a very high rate and do not want
// to query so often. Note: lastread is updated if there is no input available.
// So the input lines are monitored while draining the input queue.
//
  if (now > lastread+20) {
#ifdef ARDUINO_MOCOLUFA
    if (Serial.available()) {
      Serial.read();
#endif
#ifdef ARDUINO_PRO_MICRO
    midiEventPacket_t rx;
    rx = MidiUSB.read();
    if (rx.header != 0) {
      // nothing to be done
#endif
#ifdef TEENSY_USBMIDI
    if (usbMIDI.read()) {
      // nothing to be done
#endif
    } else {
      lastread=now;
    }
  }  
}
