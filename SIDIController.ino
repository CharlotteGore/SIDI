#include "Arduino.h"
#include <SPI.h>
#include <SIDFrequency.h>
#include <sid_registers.h>
#include <mos6581.h>

MOS6581 sid = MOS6581();
SIDFreak lookup = SIDFreak();

struct Voice {
	word frequency;
	word pw_frequency;
	byte mode;
	boolean on;
	boolean filter;
	byte attack;
	byte decay;
	byte sustain;
	byte release;
	byte id;
	boolean kbd;
	// for midi
	byte note;
	byte octaveShift;
};

struct Filter {
	boolean lp;
	boolean bp;
	boolean hp;
	word frequency;
	byte resonance;
};

struct MIDI {
	byte control;
	byte data;
	byte value;
	boolean waitingForData;
	boolean waitingForValue;
};

struct MIDI signal;



struct Filter filter;

struct Voice voiceOne;
struct Voice voiceTwo;
struct Voice voiceThree;

struct Voice voices[3] = {voiceOne, voiceTwo, voiceThree};

struct Voice* active = voices;

byte modes[7] = {0, SID_SQUARE, SID_RAMP,SID_TRIANGLE,SID_NOISE, SID_RING, SID_SYNC};


word voice_select_input = 0;
word filter_inputs[4]= {0};
word voice_inputs[9] = {0,512,0,0,0,0,0,0,0};

word* inputs[14] = { 
	filter_inputs, 
	filter_inputs + 1, 
	voice_inputs,
	filter_inputs + 2,
	filter_inputs + 3,
	voice_inputs + 1,
	voice_inputs + 2,
	&voice_select_input,
	voice_inputs + 3,
	voice_inputs + 4,
	voice_inputs + 5, 
	voice_inputs + 6, 
	voice_inputs + 7, 
	voice_inputs + 8 
};


void setActiveVoice(word raw){

	byte val = raw/ 342;
	if(voices[val].id != active->id){
		active = &voices[val];
	}

}

void modeSelect(word raw){

	//Serial.println(raw);

	byte val = modes[raw / 147];

	//Serial.print(active->id);
	//Serial.print(" ");
	//Serial.println(val);

	if(active->mode != val){
		active->mode = val;

		//if(val== SID_OFF ){
		//	active->on = false;
		//}else{
		//	active->on = true;
		//}

		sid.setMode(active->id, val);
	}
}

void setAttack(word raw){

	byte val = (byte)raw >> 6; 
	if(active->attack != val){
		active->attack = val;
		sid.setADEnvelope(active->id, active->attack, active->decay);
	}
}

void setDecay(word raw){

	byte val = (byte)raw >> 6; 
	if(active->decay != val){
		active->decay = val;
		sid.setADEnvelope(active->id, active->attack, active->decay);
	}
}

void setSustain(word raw){

	byte val = (byte)raw >> 6; 
	if(active->sustain != val){
		active->sustain = val;
		sid.setSREnvelope(active->id, active->sustain, active->release);
	}

}

void setRelease(word raw){

	byte val = (byte)raw >> 6; 
	if(active->release != val){
		active->release = val;
		sid.setSREnvelope(active->id, active->sustain, active->release);
	}
}

void toggleFilter(word raw){
	if(raw > 512){
		active->filter = true;
	}else{
		active->filter = false;
	}
	sid.setFilter(active->id, active->filter);

}

void toggleLP(word raw){

	if(raw > 512){
		filter.lp = true;
	}else{
		filter.lp = false;
	}

	if(filter.lp && filter.hp){
		sid.setFilterMode( SID_FILT_HP | SID_FILT_LP );
	}else if(filter.lp){
		sid.setFilterMode( SID_FILT_LP );
	}else if(filter.hp){
		sid.setFilterMode( SID_FILT_HP );
	}else{
		sid.setFilterMode( (byte)SID_FILT_OFF );
	}
}

void toggleHP(word raw){
	if(raw > 512){
		filter.hp = true;
	}else{
		filter.hp = false;
	}
	if(filter.lp && filter.hp){
		sid.setFilterMode( SID_FILT_HP | SID_FILT_LP );
	}else if(filter.lp){
		sid.setFilterMode( SID_FILT_LP );
	}else if(filter.hp){
		sid.setFilterMode( SID_FILT_HP );
	}else{
		sid.setFilterMode( SID_FILT_OFF );
	}

}

void toggleKeyboardOn(word raw){
	if(raw > 512){
		active->kbd = true;
	}else{
		active->kbd = false;
	}
	if(active->kbd){
		sid.setVoice(active->id, true);
	}else{
		sid.setVoice(active->id, false);
	}

}

void filterFrequency(word raw){
	filter.frequency = (word)raw << 1;
	sid.filterFrequency(filter.frequency);

}

void filterResonance(word raw){
	filter.resonance = (byte)raw >> 6;
	sid.filterResonance(filter.resonance);

}

void voicePW(word raw){

	active->pw_frequency = (word)raw << 2;
	sid.setPulseWidth(active->id, active->pw_frequency);

}

void voiceFrequency(word raw){
	if(active->kbd){
		active->frequency= (word)raw >> 3;
		sid.setFrequency( active->id, lookup.lookup( active->frequency ) );
	}
}


void (*handlers[14])(word) = { 
	toggleLP,
	toggleHP,
	toggleKeyboardOn,
	filterFrequency,
	filterResonance,
	voicePW,
	voiceFrequency,
	setActiveVoice,
	modeSelect,
	setAttack,
	setDecay,
	setSustain,
	setRelease,
	toggleFilter

};

void handleNoteOn(byte channel, byte pitch, byte velocity){
	
	word freq = lookup.lookup(pitch);
	if(velocity == 0){
		handleNoteOff(channel, pitch, velocity);
	} else {

		for(int i = 0; i < 3; i++){
			if(voices[0].kbd!=true){
				sid.setVoice(i, false);
				voices[i].note = pitch;
				sid.setFrequency(i, freq);
				sid.setVoice(i, true);
			}
		}
		
	}
};

void handleNoteOff(byte channel, byte pitch, byte velocity){

	if(voices[0].kbd!=true){

	for(int i = 0; i < 3; i++){
		if(voices[i].note == pitch){
			sid.setVoice(i, false);
		}
	}

	}
}

void setup()
{
	pinMode(4, OUTPUT);
	

	for(int i = 0;i<3; i++){
		voices[i].id = i + 1;
		voices[i].attack = 0;
		voices[i].decay = 0;
		voices[i].sustain = 15;
		voices[i].release = 0;
		voices[i].on = true;
		voices[i].filter = 0;
		voices[i].mode = SID_SQUARE;
		voices[i].pw_frequency = 2048;
		voices[i].frequency = 0;
		voices[i].kbd = false;
		voices[i].note = 0;
		voices[i].octaveShift = 0;
	}

  
	DDRD = DDRD | B11100000;  
	// get initial state
  
	for(byte i = 0; i < 7; i++){
		PORTD &= B00011111;
		PORTD |= i << 5;
      
		*inputs[i] = analogRead(0);
		*inputs[i + 7] = analogRead(1);

	}
  
  
	cli();
  
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
  
	OCR1A = 15624;
	TCCR1B |=(1 << WGM12);
	TCCR1B |=(1 << CS11) | (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);
  
	sei();

	sid.reset();
	sid.volume(15);

	for(int i = 0; i < 3; i++){
		sid.setPulseWidth(i, 2048);
		sid.setADEnvelope(i, 0,0);
		sid.setSREnvelope(i, 15,0);
		sid.setMode(i, SID_RAMP);
		
	}

	//handleNoteOn(0,40,127);
	Serial.begin(31250);
	//Serial.begin(115200);

  
}

void loop()
{
	byte raw_serial;
 if(Serial.available() > 0){
	//if(Serial.available() > 0){
		raw_serial = Serial.read();
		//incomingByte = Serial.read();

		if((raw_serial & B10000000) == B10000000){

			signal.control = raw_serial;
			signal.waitingForData = true;
			signal.waitingForValue = true;
   
		}else if(signal.waitingForData == true){

			signal.data = raw_serial;
			signal.waitingForData = false;

			if(signal.control==128){
				handleNoteOff(0, signal.data, 0);
			}

		}else if(signal.waitingForValue == true){

			signal.value = raw_serial;
    
			if(signal.control == 144){

				handleNoteOn(0, signal.data, signal.value);

			}
    
			if(signal.control == 128){
				handleNoteOff(0, signal.data, 0);
				
			}
    
		}

	
	}

}



ISR(TIMER1_COMPA_vect){
 
    byte j = 0;
	word incoming;

    for(byte i = 0; i < 7; i++){
      PORTD &= B00011111;
      PORTD |= i << 5;
      
      j = i + 7;
      
     
      //inputsA[i] = incoming;
      
      incoming = analogRead(1);
      //inputsB[i] = incoming;

      if(incoming!=*inputs[j] && (incoming > *inputs[j] + 2 || incoming < *inputs[j] - 2)){
        *inputs[j]= incoming;
		handlers[j](*inputs[j]);
      }

	  incoming = analogRead(0);
       
      if(incoming!=*inputs[i] && (incoming > *inputs[i] + 2 || incoming < *inputs[i] - 2)){
        *inputs[i]= incoming;
		handlers[i](*inputs[i]);

      }

	}

}


 