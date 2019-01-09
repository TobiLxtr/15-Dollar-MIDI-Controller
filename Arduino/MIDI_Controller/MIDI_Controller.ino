/*
 * Very basic, little MIDI controller with just four potentiomters for simple mixing and
 * better control to record automations for velocities, filters etc.
 * 
 * I used an Arduino (Pro) Micro (Not Mini!!!). It's very cheap, small and works perfectly!
 * But all Arduino/Genuino Atmega32u4 or ARM based board will work. e.g.: Zero, DUE, 101, Micro, Leonardo...
 * 
 *    Contact: 
 *    Twitter - @Tobi_Lxtr
 *    Instagram - @Tobi_Lxtr
 */

#include <MIDIUSB.h> //import library to use Arduino (only Atmega32u4 or ARM based boards) as a USB-MIDI device 

//<------------------SETTINGS------------------>

#define LED_ON true //set LED on/off
#define BRIGHTNESS 248  // set brightness of the leds: "255" is off / "0" is full brightness

#define MIDI_CHANNEL 0 //set global midi channel (0-15)

#define CC_TL 20  //set "midi contol change" control number for top left poti
#define CC_TR 21  //... for top right poti
#define CC_BL 22  //... for bottom left poti
#define CC_BR 23  //... for bottom right poti

//<-------------------------------------------->

#define LED_PIN 3       //pin the LEDs cathode is attached to, anode connect to 5V
#define POTI_TL_PIN A1  //analog input pin of the top left poti
#define POTI_TR_PIN A2  //.. of the top right poti
#define POTI_BL_PIN A0  //...
#define POTI_BR_PIN A3

#define NUM_POTIS 4 //just increase this value and the arrays below to use more pots

byte brightness = BRIGHTNESS; 

byte potiPins[] = {POTI_TL_PIN, POTI_TR_PIN, POTI_BL_PIN, POTI_BR_PIN}; 
boolean potiInversed[] = {false, false, true, true};  //array to store whether the potis are connected "backwards" (= right turn equals lower values)
byte potiCC[] = {CC_TL, CC_TR, CC_BL, CC_BR}; 
unsigned int potiValues[NUM_POTIS]; //array to store poti values of the adc
byte CCValues[NUM_POTIS], CCValuesOld[NUM_POTIS]; //values to store the calculated control change data values of each poti and their old values to see if they changed


void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, brightness);

  //<-----------INIT----------->
  readPotis();  
  convertData();
  
  for(int i = 0; i < NUM_POTIS; i++) {
    CCValuesOld[i] = CCValues[i];
  }
  //<-------------------------->
}

void loop() {
  readPotis();  
  convertData();  
  handleMidi();
}


//<----------------ROUTINES---------------->

//read poti values
void readPotis() {
  for(int i = 0; i < NUM_POTIS; i++) {
    potiValues[i] = (6 * potiValues[i] + 4 * analogRead(potiPins[i]))/10; //smooth poti values to avoid jittering
  }
}

//convert adc values to midi data
void convertData() {
  for(int i = 0; i < NUM_POTIS; i++) {
    
    //if it's an inversed poti
    if(potiInversed[i]) { 
      CCValues[i] = map(potiValues[i], 0, 1023, 127, 0); //map values from 1023-0 (adc) to 0 - 127 (max midi data range)
      CCValues[i]--; //decrease by one cause they didn't go all the way to 0
    }
    //if it's not an inversed poti
    else CCValues[i] = map(potiValues[i], 0, 1023, 0, 127); //map values from 0-1023 (adc) to 0 - 127 (max midi data range)
  }
}

//send midi data
void handleMidi() {
  for(int i = 0; i < NUM_POTIS; i++) {
    if(CCValues[i] != CCValuesOld[i]) { //if the midi value changed, send it
      controlChange(MIDI_CHANNEL, potiCC[i], CCValues[i]);  //send midi message, with midi channel number, control number and value
      CCValuesOld[i] = CCValues[i]; //old value ist now current value, to start a new reading
    }
  }
}

//routine to send midi cc messages
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

//routine to fade leds - not used since it's written using "blocking" code.
//just for display
void eyesAnimation() {
  for(brightness = 253; brightness > 233; brightness--) {
    analogWrite(LED_PIN, brightness);
    delay(50);
  }
  for(brightness = 233; brightness < 253; brightness++) {
    analogWrite(LED_PIN, brightness);
    delay(50);
  }
}

