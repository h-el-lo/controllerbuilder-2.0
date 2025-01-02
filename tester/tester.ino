// Was written initailly to test polarity of rows and columns.
// was modified to include a multiplexer and print output to
// the serial monitor without using the MIDIUSB library.

// WORKING (OUTPUT MUX (ROWS), INPUTPULLUP (COLS)).

int S0 = 15;
int S1 = 14;
int S2 = 16;
int S3 = 10;
const int signal_pin = A2;

void setup() {
  // put your setup code here, to run once:
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(signal_pin, OUTPUT);

  pinMode(9, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(signal_pin, LOW);


  ch_read(0);
  Serial.print("Value1: ");
  Serial.print(!digitalRead(9));
  
  ch_read(8);
  Serial.print(", Value2: ");
  Serial.println(!digitalRead(9));
}

void ch_read(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  digitalWrite(S2, (channel >> 2) & 0x01);
  digitalWrite(S3, (channel >> 3) & 0x01);

  // Serial.print((channel >> 3) & 0x01);
  // Serial.print((channel >> 2) & 0x01);
  // Serial.print((channel >> 1) & 0x01);
  // Serial.println(channel & 0x01);

  // delay(1000);
}