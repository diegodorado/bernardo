#include <MIDI.h>
#include "LedControl.h"

#include <SoftPWM.h>
/* pins_arduino.h defines the pin-port/bit mapping as PROGMEM so
   you have to read them with pgm_read_xxx(). That's generally okay
   for ordinary use, but really bad when you're writing super fast
   codes because the compiler doesn't treat them as constants and
   cannot optimize them away with sbi/cbi instructions.

   Therefore we have to tell the compiler the PORT and BIT here.
   Hope someday we can find a way to workaround this.

   Check the manual of your MCU for port/bit mapping.

   The following example demonstrates setting channels for all pins
   on the ATmega328P or ATmega168 used on Arduino Uno, Pro Mini,
   Nano and other boards. */
SOFTPWM_DEFINE_CHANNEL(2, DDRD, PORTD, PORTD2);  //Arduino pin 2
SOFTPWM_DEFINE_CHANNEL(3, DDRD, PORTD, PORTD3);  //Arduino pin 3
SOFTPWM_DEFINE_CHANNEL(5, DDRD, PORTD, PORTD5);  //Arduino pin 5
SOFTPWM_DEFINE_CHANNEL(6, DDRD, PORTD, PORTD6);  //Arduino pin 6
SOFTPWM_DEFINE_CHANNEL(7, DDRD, PORTD, PORTD7);  //Arduino pin 7
SOFTPWM_DEFINE_CHANNEL(8, DDRB, PORTB, PORTB0);  //Arduino pin 8
SOFTPWM_DEFINE_CHANNEL(9, DDRB, PORTB, PORTB1);  //Arduino pin 9
SOFTPWM_DEFINE_CHANNEL(10, DDRB, PORTB, PORTB2);  //Arduino pin 10
SOFTPWM_DEFINE_CHANNEL(0, DDRB, PORTB, PORTB5);  //Arduino pin 13

// SOFTPWM_DEFINE_EXTERN_OBJECT(8);

SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(5, 10);

/* If you want to use the SoftPWM object outside where it's defined,
   add the following line to the file. */
//SOFTPWM_DEFINE_EXTERN_OBJECT(16);
// SOFTPWM_DEFINE_EXTERN_OBJECT_WITH_PWM_LEVELS(20, 100);


//LedControl lc=LedControl(13,11,12,2);
LedControl lc=LedControl(A0,11,12,2);
// pin 13 is connected to the MAX7219 pin 1 - Data In
// pin 12 is connected to the CLK pin 13
// pin 11 is connected to LOAD pin 12
// we are only using 2 MAX7219
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

void setup() {

  // begin with 60hz pwm frequency
  Palatis::SoftPWM.begin(120);

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



static volatile uint8_t v = 0;
void loop() {
/*
    unsigned long const WAIT = 1000000 / Palatis::SoftPWM.PWMlevels() / 2;
    unsigned long nextMicros = 0;
    for (int v = 0; v < Palatis::SoftPWM.PWMlevels() - 1; ++v) {
      while (micros() < nextMicros);
      nextMicros = micros() + WAIT;
      Palatis::SoftPWM.set(0, v);
    }
    for (int v = Palatis::SoftPWM.PWMlevels() - 1; v >= 0; --v) {
      while (micros() < nextMicros);
      nextMicros = micros() + WAIT;
      Palatis::SoftPWM.set(0, v);
    }
*/

  Palatis::SoftPWM.set(0, 10);
  delay(20);
  Palatis::SoftPWM.set(0, 0);
  delay(1000);
  Palatis::SoftPWM.set(0, 7);
  delay(20);
  Palatis::SoftPWM.set(0, 0);
  delay(1000);
  Palatis::SoftPWM.set(0, 5);
  delay(20);
  Palatis::SoftPWM.set(0, 0);
  delay(1000);




}



long lastLcTick = 0;
long lastTick = 0;

int chan = 0;

void loop2() {

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



