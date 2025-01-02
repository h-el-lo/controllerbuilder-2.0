// WORKING ALL KEYS NOTEON, NOTE OFF, VELOCITY SENSITIVE
// ALL KEYS, 3 MUXS, PEDAL
#include <MIDIUSB.h>

// =================== MUX VARIABLES  ======================
// Mux 1 (Outputs (keys), KPS AND KPE (rows))
int S10 = 15;
int S11 = 14;
int S12 = 16;
int S13 = 10;
const int signal = A0;

// Mux 2 (Inputs (keys) (columns)) digital
int S20 = 9;
int S21 = 8;
int S22 = 7;
int S23 = 6;
const int signal2 = A1;

// Mux 3 (Analog input for Potentiometers and other control changes like damper pedal)
int S30 = 5;
int S31 = 4;
int S32 = 3;
int S33 = 2;
const int signal3 = A2;
// ===========================================================


// ==============================  KEYS VARIABLES  =====================================
// KEYSCAN MATRIX VARIABLES
const int COL_NUM = 8;
const int ROW_NUM = 8;

int cols[COL_NUM] = { 0, 1, 2, 3, 4, 5, 6, 7 };       // Blue cols (Mux2 0 - 7) input_pullup
int KPS[ROW_NUM] = { 0, 1, 2, 3, 4, 5, 6, 7 };        // Brown rows (Mux1 0 - 7), output
int KPE[ROW_NUM] = { 8, 9, 10, 11, 12, 13, 14, 15 };  // White rows (Mux1 8 - 15), output

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
//  ===========================================================================


// ============================  MIDI VARIABLES  =============================
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
// ==========================================================


// ===================  POTENTIOMETER VARIABLES  =======================

// Global Analog Input Variables
const int N_ANALOGS = 5;
int analogPins[N_ANALOGS] = { 0, 1, 2, 3, 4 }; // (Mux3 0 - 7) input_pullup

// Potentiometer Variables
const int N_POTS = 5;
int potPin[N_POTS] = { 7, 1, 2, 3, 4 };  // (Mux3 0 - 7) input_pullup
int potCC[N_POTS] = { 24, 25, 26, 27, 7 };

int potReading[N_POTS] = { 0 };
int potState[N_POTS] = { 0 };
int potPState[N_POTS] = { 0 };

int midiState[N_POTS] = { 0 };
int midiPState[N_POTS] = { 0 };

byte potThreshold = 15;
const int POT_TIMEOUT = 300;
unsigned long pPotTime[N_POTS] = { 0 };
unsigned long potTimer[N_POTS] = { 0 };

// Modulation Wheel Variables
int modWheel = 5;  // JoyY (Mux3, ch5)
int modReading = 0;
int modMidiState = 0;
int modMidiPState = 0;
int modState = 0;
int modPrevState = 0;
int modTimer = 0;
int pModTime = 0;

// Pitch Wheel Variables
int pitchWheel = 6;  // JoyX (Mux3, ch6)
int pitchReading = 0;
int pitchMidiState = 0;
int pitchMidiPState = 0;
int pitchState = 0;
int pitchPrevState = 0;
byte pitchThreshold = 3;
// =========================================================================

// =====================  SUSTAIN PEDAL VARIABLES  =========================
int sustainPin = 0;  // Mux3, ch7
int susState = 0;
int susPrevState = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(S10, OUTPUT);
  pinMode(S11, OUTPUT);
  pinMode(S12, OUTPUT);
  pinMode(S13, OUTPUT);
  pinMode(signal, OUTPUT);
  digitalWrite(signal, HIGH);

  pinMode(S20, OUTPUT);
  pinMode(S21, OUTPUT);
  pinMode(S22, OUTPUT);
  pinMode(S23, OUTPUT);
  pinMode(signal2, INPUT_PULLUP);

  pinMode(S30, OUTPUT);
  pinMode(S31, OUTPUT);
  pinMode(S32, OUTPUT);
  pinMode(S33, OUTPUT);
  pinMode(signal3, INPUT_PULLUP);
}

void loop() {

  // ==============================  READ THROUGH THE KEYS  ===============================
  for (int y = 0; y < COL_NUM; y++) {

    for (int x = 0; x < ROW_NUM; x++) {

      note = nums[x][y];

      if (!not_ready[x][y]) {

        // Shift mux to Keypress-start (KPS) channel and read the digital input of note[x][y]
        mux_ch(KPS[x]);
        digitalWrite(signal, LOW);
        mux2_ch(cols[y]);
        temp = !digitalRead(signal2);
        digitalWrite(signal, HIGH);

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
        digitalWrite(signal, LOW);
        mux2_ch(cols[y]);
        temp = !digitalRead(signal2);
        digitalWrite(signal, HIGH);

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
        pressed[x][y] = 0;

      }

      if (not_ready[x][y]) {

        mux_ch(KPS[x]);
        digitalWrite(signal, LOW);
        mux2_ch(cols[y]);
        kps[x][y] = !digitalRead(signal2);

        mux_ch(KPE[x]);
        digitalWrite(signal, LOW);
        mux2_ch(cols[y]);
        kpe[x][y] = !digitalRead(signal2);

        digitalWrite(signal, HIGH);
        if (!kps[x][y] && !kpe[x][y]) {
          noteOff(0, note, velocity);
          not_ready[x][y] = 0;
        }
      }
    }
  }
  // =======================================================================================


  // ============  READ THROUGH ALL POTS MINUS PITCH AND MOD WHEELS  =====================
  for (int i = 0; i < N_POTS; i++) {

    mux3_ch(potPin[i]);
    potReading[i] = analogRead(signal3);
    potState[i] = potReading[i];
    midiState[i] = map(potState[i], 0, 1023, 0, 127);

    int potVar = abs(potState[i] - potPState[i]);

    if (potVar > potThreshold) {
      pPotTime[i] = millis();
    }

    potTimer[i] = millis() - pPotTime[i];

    if (potTimer[i] < POT_TIMEOUT) {
      if (midiState[i] != midiPState[i]) {
        controlChange(channel, potCC[i], midiState[i]);
        midiPState[i] = midiState[i];
      }
      potPState[i] = potState[i];
    }
  }
  // ========================================================================================


  // Modulation Wheel (Joystick Y)
  // =========================================================
  // The Modulation Wheel has to be written separately,
  // because of the difference in the range of values
  // - 127 to 127, precisely.

  mux3_ch(modWheel);
  int modReading = analogRead(signal3);
  modState = modReading;
  modMidiState = map(modReading, 0, 1023, -127, 127);
  int modVar = abs(modState - modPrevState);

  if (modVar > potThreshold) {
    pModTime = millis();
  }

  modTimer = millis() - pModTime;

  if (modTimer < POT_TIMEOUT) {
    if (modMidiState != modMidiPState) {
      if (modMidiState >= 0) {
        // Send Modulation coarse (CC 1)
        controlChange(channel, 1, modMidiState);
      } else {
        // Send modulationm LSB fine/smooth (CC 33)
        controlChange(channel, 33, abs(modMidiState));
      }
      modMidiPState = modMidiState;
    }
    modPrevState = modState;
  }
  //==========================================================



  // Pitch Wheel (Joystick X)
  //=========================================================
  mux3_ch(pitchWheel);
  int pitchReading = analogRead(signal3);;
  pitchState = pitchReading;
  pitchMidiState = map(pitchReading, 1023, 0, 0, 16383);

  int pitchVar = abs(pitchState - pitchPrevState);

  if (pitchVar > pitchThreshold) {

    if (pitchMidiState != pitchMidiPState) {
      pitchBend(channel, pitchMidiState);
      pitchPrevState = pitchState;
      // delay(5);
    }
    pitchMidiPState = pitchMidiState;
  }
  //==========================================================


  // Sustain Pedal
  //=========================================================
  mux3_ch(sustainPin);
  int susRead = !digitalRead(signal3);
  int susState = map(susRead, 0, 1, 0, 127);

  if (susState != susPrevState) {
    controlChange(channel, 64, susState);
    susPrevState = susState;
    delay(5);
  }
  //==========================================================
}


void mux_ch(int channel) {
  digitalWrite(S10, channel & 0x01);
  digitalWrite(S11, (channel >> 1) & 0x01);
  digitalWrite(S12, (channel >> 2) & 0x01);
  digitalWrite(S13, (channel >> 3) & 0x01);
}

void mux2_ch(int channel) {
  digitalWrite(S20, channel & 0x01);
  digitalWrite(S21, (channel >> 1) & 0x01);
  digitalWrite(S22, (channel >> 2) & 0x01);
  digitalWrite(S23, (channel >> 3) & 0x01);
}

void mux3_ch(int channel) {
  digitalWrite(S30, channel & 0x01);
  digitalWrite(S31, (channel >> 1) & 0x01);
  digitalWrite(S32, (channel >> 2) & 0x01);
  digitalWrite(S33, (channel >> 3) & 0x01);
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

int findIndex(int arr[], int size, int target) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == target) {
      return i;  // Return the index if a match is found.
    }
  }

  return -1;  // Return =1 if the target is not found in the array.
}


