#include "Arduino.h"
#include <SPI.h>
#include <SIDFrequency.h>
#include <sid_registers.h>
#include <mos6581.h>

MOS6581 sid = MOS6581();
SIDFreak lookup = SIDFreak();

//#define debug;

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
	int8_t octaveShift;
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


word inputs[14];


void setActiveVoice(word raw){

	byte val = raw/ 342;
	if(voices[val].id != active->id){
		active = &voices[val];
	}

}

void modeSelect(word raw){


	byte val = modes[raw / 147];

	if(active->mode != val){
		active->mode = val;

		if(val == 0){
			active->on = 0;
			Serial.print("Voice off");
			sid.setMode(active->id, 0);
		}else{
			active->on = 1;
			sid.setMode(active->id, val);
		}
	
#ifdef debug
		Serial.print("mode ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.print(active->mode, DEC);
		Serial.print(" ");
		Serial.print(active->on, DEC);
#endif
	}
}

void setAttack(word raw){

	byte val = raw >> 6; 
	if(active->attack != val){
		active->attack = val;
		sid.setADEnvelope(active->id, active->attack, active->decay);

#ifdef debug
		Serial.print("decay ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->attack, DEC);
#endif
	}
}

void setDecay(word raw){

	byte val = raw >> 6; 
	if(active->decay != val){
		active->decay = val;
		sid.setADEnvelope(active->id, active->attack, active->decay);

#ifdef debug
		Serial.print("attack ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->decay, DEC);
#endif

	}
}

void setSustain(word raw){

	byte val = raw >> 6; 
	if(active->sustain != val){
		active->sustain = val;
		sid.setSREnvelope(active->id, active->sustain, active->release);

#ifdef debug
				Serial.print("attack ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->sustain, DEC);
#endif

	}

}

void setRelease(word raw){

	byte val = raw >> 6; 


if(active->release != val){
		active->release = val;
		sid.setSREnvelope(active->id, active->sustain, active->release);

#ifdef debug
		Serial.print("release ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->release, DEC);
#endif

	}
}

void toggleFilter(word raw){
	if(raw > 512){
		active->filter = true;
	}else{
		active->filter = false;
	}
	sid.setFilter(active->id, active->filter);

#ifdef debug
		Serial.print("filter ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->filter, DEC);
#endif


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
	#ifdef debug
			Serial.print("lp ");
		Serial.println(filter.lp, DEC);
#endif
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

	#ifdef debug
	Serial.print("hp ");
	Serial.println(filter.hp, DEC);
#endif

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
	#ifdef debug
		Serial.print("kbd ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->kbd, DEC);
#endif
}

void filterFrequency(word raw){
	filter.frequency = raw << 1;
	sid.filterFrequency(filter.frequency);

#ifdef debug
		Serial.print("filter freq ");
		Serial.println(filter.frequency, DEC);
#endif

}

void filterResonance(word raw){
	byte val = raw >> 6;

	if(filter.resonance != val){
		filter.resonance = val;
		sid.filterResonance(filter.resonance);

#ifdef debug
		Serial.print("filter resonance ");
		Serial.println(filter.resonance, DEC);
#endif

	}


}

void voicePW(word raw){

	active->pw_frequency = raw << 2;
	sid.setPulseWidth(active->id, active->pw_frequency);
#ifdef debug
		Serial.print("pwm ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->pw_frequency, DEC);
#endif

}

void voiceFrequency(word raw){

	int8_t val;

	if(active->kbd){
		active->frequency = raw >> 3;
		sid.setFrequency( active->id, lookup.lookup( active->frequency ) );

#ifdef debug
		Serial.print("freq ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
	Serial.println(active->frequency, DEC);
#endif


	}else{
		val = (raw / 205) - 2;
		if(active->octaveShift != val){
		
		}
		active->octaveShift =  val;
		sid.setFrequency( active->id, lookup.lookup( active->note + (12 * active->octaveShift)) );
		
#ifdef debug
		Serial.print("oct shift ");
		Serial.print(active->id, DEC);
		Serial.print(" ");
		Serial.println(active->octaveShift, DEC);
#endif

	}

	
}


void (*handlers[14])(word) = { 
	voiceFrequency,
	voicePW,
	filterResonance,
	filterFrequency,
	toggleKeyboardOn,
	toggleHP,
	toggleLP,
	toggleFilter,
	setRelease,
	setSustain,
	setDecay,
	setAttack,
	modeSelect,
	setActiveVoice
};

void handleNoteOn(byte channel, byte pitch, byte velocity){
	
	for(int i = 0; i < 3; i++){
		if(voices[i].kbd!=true && voices[i].on == 1){
			sid.setVoice(i, false);
			voices[i].note = pitch;
			sid.setFrequency(i, lookup.lookup(pitch + (12 * voices[i].octaveShift))  );
			sid.setVoice(i, true);
		}
	}
		
};

void handleNoteOff(byte channel, byte pitch, byte velocity){


	for(int i = 0; i < 3; i++){
		if(voices[i].kbd!=true && voices[i].on == true){
			if(voices[i].note == pitch){
				sid.setVoice(i, false);
			}
		}
	}
}

byte j;
word incoming;

void setup()
{

	for(int i = 0;i<3; i++){
		voices[i].id = i;
		voices[i].attack = 0;
		voices[i].decay = 0;
		voices[i].sustain = 15;
		voices[i].release = 0;
		voices[i].on = false;
		voices[i].filter = 0;
		voices[i].mode = 0;
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
      
		inputs[i] = analogRead(0);
		inputs[i + 7] = analogRead(1);

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
		sid.setMode(i, 0);
		
	}

	//handleNoteOn(0,40,127);

#ifndef debug
	Serial.begin(31250);
#endif

#ifdef debug
	Serial.begin(115200);
#endif

  
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
 
    for(byte i = 0; i < 7; i++){
      PORTD &= B00011111;
      PORTD |= i << 5;
      
      j = i + 7;
      
      incoming = analogRead(1);


     if(incoming!=inputs[j] && (incoming > inputs[j] + 2 || incoming < inputs[j] - 2)){


		inputs[j]= incoming;
		handlers[j](inputs[j]);

     }

	 incoming = analogRead(0);
       
      if(incoming!=inputs[i] && (incoming > inputs[i] + 2 || incoming < inputs[i] - 2)){
       
	  
	  inputs[i]= incoming;

		handlers[i](inputs[i]);

     }

	}
#ifdef debug
	Serial.print(active->id, DEC);
	Serial.print(" ");
	Serial.print(active->mode, DEC);
	Serial.print(" ");
	Serial.println(active->on,DEC);
#endif
}


 