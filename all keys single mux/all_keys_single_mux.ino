// WORKING ALL KEYS NOTEON, NOTE OFF, VELOCITY SENSITIVE
// 64 KEYS, SINGLE MUX

#include <MIDIUSB.h>

// KEYSCAN MATRIX VARIABLES
const int COL_NUM = 8;
const int ROW_NUM = 8;

int cols[COL_NUM] = { 9, 8, 7, 6, 5, 4, 3, 2 };       // Blue cols, input_pullup
int KPS[ROW_NUM] = { 0, 1, 2, 3, 4, 5, 6, 7 };        // Brown rows (mux 0 - 7), output
int KPE[ROW_NUM] = { 8, 9, 10, 11, 12, 13, 14, 15 };  // White rows (mux 8 - 15), output

int S0 = 15;
int S1 = 14;
int S2 = 16;
int S3 = 10;
const int signal_pin = A0;

// Array to keep track of previous states of kps and kpe data for all keys
int pState[2][ROW_NUM][COL_NUM] = { 0 };  // pState[2] for kps[x][y] and kpe[x][y]
int temp;                                 // variable for temporary storage
// Arrays to keep track of present states of kps and kpe data for all keys
bool kps[ROW_NUM][COL_NUM] = { 0 };
int kpe[ROW_NUM][COL_NUM] = { 0 };
bool pressed[ROW_NUM][COL_NUM] = { 0 };

// The "not_ready[x][y]" variable name is used here because using "ready[x][y] = 1" would
// set just ready[0][0] to "1", and all other elements to "0". The logic is then inverted
// in variable naming and assignment to "0" instead. This way, one saves the stress of
// having to hardcode the array, giving flexibility when modifying the program.

// bool ready[ROW_NUM][COL_NUM] = { 1 };
bool not_ready[ROW_NUM][COL_NUM] = { 0 };

// TIMER VARIABLES
unsigned long timer[2][ROW_NUM][COL_NUM] = { 0 };  // timer[2] for kps[x][y] and kpe[x][y]
int time;

// MIDI VARIABLES
const int channel = 0;
int note, vel, velocity;
int vel_min = 0;
int vel_max = 50;


int nums[ROW_NUM][COL_NUM] = {
  // Array  of midi note numbers C1 (24) to D#6 (87), 64 notes in total.
  { 24, 25, 26, 27, 28, 29, 30, 31 },
  { 32, 33, 34, 35, 36, 37, 38, 39 },
  { 40, 41, 42, 43, 44, 45, 46, 47 },
  { 48, 49, 50, 51, 52, 53, 54, 55 },
  { 56, 57, 58, 59, 60, 61, 62, 63 },
  { 64, 65, 66, 67, 68, 69, 70, 71 },
  { 72, 73, 74, 75, 76, 77, 78, 79 },
  { 80, 81, 82, 83, 84, 85, 86, 87 },
};

String noteNames[ROW_NUM][COL_NUM] = {
  // Row 0: MIDI 24 to 31
  { "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1" },

  // Row 1: MIDI 32 to 39
  { "G#1", "A1", "A#1", "B1", "C2", "C#2", "D2", "D#2" },

  // Row 2: MIDI 40 to 47
  { "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2" },

  // Row 3: MIDI 48 to 55
  { "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3" },

  // Row 4: MIDI 56 to 63
  { "G#3", "A3", "A#3", "B3", "C4", "C#4", "D4", "D#4" },

  // Row 5: MIDI 64 to 71
  { "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4" },

  // Row 6: MIDI 72 to 79
  { "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5" },

  // Row 7: MIDI 80 to 87
  { "G#5", "A5", "A#5", "B5", "C6", "C#6", "D6", "D#6" }
};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  for (int j = 0; j < ROW_NUM; j++) {
    pinMode(cols[j], INPUT_PULLUP);
  }

  pinMode(signal_pin, OUTPUT);
  digitalWrite(signal_pin, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int y = 0; y < COL_NUM; y++) {

    for (int x = 0; x < ROW_NUM; x++) {
      // Serial.print("KPS: ");
      // Serial.print(kps[x][y]);
      // Serial.print(", KPE: ");
      // Serial.println(kpe[x][y]);

      note = nums[x][y];

      if (!not_ready[x][y]) {

        // Shift mux to Keypress-start (KPS) channel and read the digital input of note[x][y]
        mux_ch(KPS[x]);
        digitalWrite(signal_pin, LOW);
        temp = !digitalRead(cols[y]);
        digitalWrite(signal_pin, HIGH);

        if (temp != pState[0][x][y]) {
          if (temp == 1) {
            timer[0][x][y] = millis();
            kps[x][y] = 1;
            pState[0][x][y] = temp;
          } else {
            timer[0][x][y] = 0;
            kps[x][y] = 0;
            pState[0][x][y] = temp;
          }
        }

        // Shift mux to Keypress-end (KPE) channel and read the digital input of note[x][y]
        mux_ch(KPE[x]);
        digitalWrite(signal_pin, LOW);
        temp = !digitalRead(cols[y]);
        digitalWrite(signal_pin, HIGH);

        if (temp != pState[1][x][y]) {
          if (temp == 1) {
            timer[1][x][y] = millis();
            kpe[x][y] = 1;
            pState[1][x][y] = temp;
          } else {
            timer[1][x][y] = 0;
            kpe[x][y] = 0;
            pState[1][x][y] = temp;
          }
        }

        if (kps[x][y] && kpe[x][y]) {
          // Declare key[x][y] as "pressed" and not ready to read another keypress
          pressed[x][y] = 1;
          not_ready[x][y] = 1;
        }
      }

      // Sends a noteOn midi message when keypress is complete
      if (pressed[x][y]) {
        time = abs(int(timer[1][x][y] - timer[0][x][y]));
        vel = constrain(time, vel_min, vel_max);
        velocity = map(vel, vel_max, vel_min, 10, 127);
        noteOn(0, note, velocity);
        // serialNoteOn(channel, note, time, velocity);

        pressed[x][y] = 0;
      }

      if (not_ready[x][y]) {

        mux_ch(KPS[x]);
        digitalWrite(signal_pin, LOW);
        kps[x][y] = !digitalRead(cols[y]);

        mux_ch(KPE[x]);
        digitalWrite(signal_pin, LOW);
        kpe[x][y] = !digitalRead(cols[y]);

        digitalWrite(signal_pin, HIGH);
        if (!kps[x][y] && !kpe[x][y]) {
          noteOff(0, note, velocity);
          not_ready[x][y] = 0;
        }
      }
    }
  }
}


void mux_ch(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  digitalWrite(S2, (channel >> 2) & 0x01);
  digitalWrite(S3, (channel >> 3) & 0x01);
}

void noteOn(byte channel, byte note, byte velocity) {
  midiEventPacket_t event = { 0x09, 0x90 | channel, note, velocity };
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void noteOff(byte channel, byte note, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, note, velocity };
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void pitchBend(byte channel, int value) {
  midiEventPacket_t pitchBend = { 0x0E, 0xE0 | channel, value & 0x7F, (value >> 7) & 0x7F };
  MidiUSB.sendMIDI(pitchBend);
  MidiUSB.flush();
}

void serialNoteOn(int channel, int note, int time, int velocity) {
  Serial.print("Channel: ");
  Serial.print(channel + 1);
  Serial.print(", ");
  Serial.print("Note: ");
  Serial.print(note);
  Serial.print(", ");
  Serial.print("Time: ");
  Serial.print(time);
  Serial.print(", ");
  Serial.print("Velocity: ");
  Serial.println(velocity);
}
