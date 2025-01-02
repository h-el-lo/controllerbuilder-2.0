// WORKING SINGLE C0 (midi note 24) NOTEON NOTEOFF, VELOCITY SENSITIVE
// DOUBLE MUX SINGLE KEY

#include <MIDIUSB.h>

int col = 0;
int kps = 0;  // Mux
int kpe = 8;  // Mux

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

int pState[2] = { 0 };
int temp = 0;
bool KPS, KPE = 0;
bool pressed = 0;
bool not_ready = 0;

// TIMER VARIABLES
unsigned long timer[2] = { 0 };
int time;

int vel_min = 0;
int vel_max = 50;
int vel;

//  MIDI VARIABLES
const int channel = 0; // Midi channel 1
int note = 24; // Midi note


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
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
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!not_ready) {

    mux_ch(kps);
    digitalWrite(signal, LOW);
    mux_ch(col);
    temp = !digitalRead(signal2);
    if (temp != pState[0]) {
      if (temp == 1) {
        timer[0] = millis();
        Serial.print("Key 1 pressed: ");
        Serial.println(timer[0]);
        KPS = true;
        pState[0] = temp;
      } else {
        timer[0] = 0;
        KPS = false;
        pState[0] = temp;
      }
    }

    mux_ch(kpe);
    digitalWrite(signal, LOW);
    temp = !digitalRead(signal2);
    if (temp != pState[1]) {
      if (temp == 1) {
        timer[1] = millis();
        Serial.print("Key 2 pressed: ");
        Serial.println(timer[1]);
        KPE = true;
        pState[1] = temp;
      } else {
        timer[1] = 0;
        KPE = false;
        pState[1] = temp;
      }
    }


    if (KPS && KPE) {
      pressed = 1;
      not_ready = 1;
    }
  }


  if (pressed) {
    time = abs(int(timer[1] - timer[0]));
    Serial.print("The result of the calculation is: ");
    Serial.println(time);
    vel = map(constrain(time, vel_min, vel_max), vel_max, vel_min, 10, 127);
    noteOn(channel, note, vel);
    KPS = 0;
    KPE = 0;
    pressed = 0;
  }

  if (not_ready) {
    mux_ch(kps);
    digitalWrite(signal, LOW);
    KPS = !digitalRead(signal2);

    mux_ch(kpe);
    digitalWrite(signal, LOW);
    KPE = !digitalRead(signal2);

    digitalWrite(signal, HIGH);
    if (!KPS && !KPE) {
      noteOff(channel, note, vel);
      not_ready = 0;
      // Serial.println("Ready!");
    }

    // Serial.print("KPS: ");
    // Serial.print(KPS);
    // Serial.print(", KPE: ");
    // Serial.println(KPE);
  }
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
