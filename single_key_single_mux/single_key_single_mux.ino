// WORKING SINGLE C0 (midi note 24) NOTEON NOTEOFF, VELOCITY SENSITIVE
// SINGLE KEY, SINGLE MUX

#include <Adafruit_TinyUSB_MIDI.h>

Adafruit_TinyUSB_MIDI MIDI;

int col = 9;
int kps = 0;  // Mux
int kpe = 8;  // Mux

int S0 = 15;
int S1 = 14;
int S2 = 16;
int S3 = 10;
const int signal_pin = A0;

int pState[2] = { 0 };
int temp = 0;
bool KPS, KPE = 0;
bool pressed = 0;
bool not_ready = 0;

// TIMER VARIABLES
unsigned long timer[2] = { 0 };
int time_taken;

int vel_min = 0;
int vel_max = 50;
int vel;

//  MIDI VARIABLES
const int channel = 0; // Midi channel 1
int note = 24; // Midi note



// ================================================
void mux_ch(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  digitalWrite(S2, (channel >> 2) & 0x01);
  digitalWrite(S3, (channel >> 3) & 0x01);
}
// =================================================


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  MIDI.begin();

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(col, INPUT_PULLUP);

  pinMode(signal_pin, OUTPUT);
  digitalWrite(signal_pin, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!not_ready) {

    mux_ch(kps);
    digitalWrite(signal_pin, LOW);
    temp = !digitalRead(col);
    if (temp != pState[0]) {
      if (temp == 1) {
        timer[0] = millis();
        // Serial.print("Key 1 pressed: ");
        // Serial.println(timer[0]);
        KPS = true;
        pState[0] = temp;
      } else {
        timer[0] = 0;
        KPS = false;
        pState[0] = temp;
      }
    }

    mux_ch(kpe);
    digitalWrite(signal_pin, LOW);
    temp = !digitalRead(col);
    if (temp != pState[1]) {
      if (temp == 1) {
        timer[1] = millis();
        // Serial.print("Key 2 pressed: ");
        // Serial.println(timer[1]);
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
    time_taken = abs(int(timer[1] - timer[0]));
    // Serial.print("The result of the calculation is: ");
    // Serial.println(time);
    vel = map(constrain(time_taken, vel_min, vel_max), vel_max, vel_min, 10, 127);
    MIDI.sendNoteOn(note, vel, channel);
    KPS = 0;
    KPE = 0;
    pressed = 0;
  }

  if (not_ready) {
    mux_ch(kps);
    digitalWrite(signal_pin, LOW);
    KPS = !digitalRead(col);

    mux_ch(kpe);
    digitalWrite(signal_pin, LOW);
    KPE = !digitalRead(col);

    digitalWrite(signal_pin, HIGH);
    if (!KPS && !KPE) {
      MIDI.sendNoteOff(note, vel, channel);
      not_ready = 0;
      // Serial.println("Ready!");
    }

    // Serial.print("KPS: ");
    // Serial.print(KPS);
    // Serial.print(", KPE: ");
    // Serial.println(KPE);
  }
}

