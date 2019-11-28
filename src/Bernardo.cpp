#include <Arduino.h>
#include <MIDI.h>
#include "LedControl.h"

LedControl lc=LedControl(13,11,12,2);

#define LC_TICK_INTERVAL 25

#define OUTS 9
/*
   0  2  36 Bombo       36 Bass Drum 1
   1  3  38 Tambor      38 Acoustic Snare
   -- 2 4   Platillo --- mosfet muerto
   2  5  41 Conga1      41 Low Floor Tom
   3  5  45 Conga1      45 Low Tom
   4  6  43 Conga2      43 High Floor Tom
   5  7  42 Shaker      42 Closed Hi-hat
   6  8  46 Placa       46 Open Hi-hat
   7  9  51 Rueda       51 Ride Cymbal 1
   8 10  49 Gong        49 Crash Cymbal 1

notes i will receive * and what they mean
midiMapping = [36, 38, 42, 46, 41, 43, 45, 49, 51]
*/

int channels_pin[OUTS]           = {  2,  3,  5,  5,  6,  7,  8,  9, 10};
int channels_note[OUTS]          = { 36, 38, 41, 45, 43, 42, 46, 51, 49};
int channels_state[OUTS]         = {  0,  0,  0,  0,  0,  0,  0,  0,  0};
long channels_at[OUTS]           = {  0,  0,  0,  0,  0,  0,  0,  0,  0};
long channels_pulse_length[OUTS] = { 20, 20, 20, 20, 20, 20, 20, 20, 20};


struct CustomMidiSettings : public midi::DefaultSettings {
  static const unsigned BaudRate = 31250;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, CustomMidiSettings);

void handleNoteOn(byte channel, byte pitch, byte velocity) {

  for (int i = 0; i < OUTS; i++) {
    if (channels_note[i] == pitch) {
      digitalWrite(channels_pin[i], 1);
      channels_state[i] = 1;
      channels_at[i] = millis();
    }
  }
}





#define PIXELS_COUNT (2*8*8)
uint8_t pixels[PIXELS_COUNT] = {0};
void tick_lc()
{
  int i = random(PIXELS_COUNT);
  // invert a random pixel
  pixels[i] = !pixels[i];
  int dev = i/(8*8);
  int col = (i/8)%8;
  int row = i%8;
  lc.setLed(dev,col,row,pixels[i]);
}



void setup() {

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0,false);// turn off power saving, enables display
  lc.setIntensity(0,15);// sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  lc.shutdown(1,false);// turn off power saving, enables display
  lc.setIntensity(1,15);// sets brightness (0~15 possible values)
  lc.clearDisplay(1);// clear screen
  randomSeed(analogRead(0));
}


long lastLcTick = 0;
long lastTick = 0;
int chan = 0;

void loop() {

  long curMillis = millis();

  for (int i = 0; i < OUTS; i++) {
    // only turn off channels with pulse length setting
    if (channels_pulse_length[i] != 0 && channels_state[i] && (curMillis - channels_at[i]) > channels_pulse_length[i]) {
      digitalWrite(channels_pin[i], 0);
      channels_state[i] = 0;
    }
  }
  MIDI.read();



  if(curMillis-lastLcTick>LC_TICK_INTERVAL){
    lastLcTick = curMillis;
    tick_lc();
  }


/*
  if(curMillis-lastTick>1000){
    lastTick = curMillis;
    digitalWrite(channels_pin[chan], 1);
    channels_state[chan] = 1;
    channels_at[chan] = curMillis;
    chan++;
    chan %= OUTS;
  }
  */

}

