#include <SPI.h>


#include <MOS6581.h>
#include <sid_registers.h>

#include "Arduino.h"
#include <SIDFrequency.h>

MOS6581 sid = MOS6581();
SIDFreak lookup = SIDFreak();

//#define debug;

float bends[128] = {
	0.49999999766807524,
	0.5054877463240971,
	0.511035725751023,
	0.5166445970115204,
	0.522315028423748,
	0.528047695640989,
	0.533843281732157,
	0.5397024772631871,
	0.5456259803793186,
	0.5516144968882825,
	0.5576687403444,
	0.5637894321336064,
	0.5699773015594065,
	0.5762330859297735,
	0.5825575306450024,
	0.5889513892865272,
	0.595415423706713,
	0.6019504041196332,
	0.6085571091928443,
	0.6152363261401661,
	0.6219888508154816,
	0.6288154878075662,
	0.6357170505359572,
	0.6426943613478758,
	0.6497482516162131,
	0.656879561838591,
	0.6640891417375111,
	0.6713778503616017,
	0.6787465561879774,
	0.6861961372257208,
	0.6937274811205013,
	0.7013414852603406,
	0.7090390568825401,
	0.7168211131817824,
	0.7246885814194177,
	0.7326423990339505,
	0.740683513752739,
	0.7488128837049212,
	0.7570314775355782,
	0.765340274521153,
	0.7737402646861348,
	0.7822324489210233,
	0.79081783910159,
	0.7994974582094458,
	0.8082723404539337,
	0.8171435313953588,
	0.8261120880695701,
	0.8351790791139112,
	0.8443455848945519,
	0.8536126976352181,
	0.8629815215473342,
	0.8724531729615944,
	0.8820287804609772,
	0.8917094850152208,
	0.9014964401167738,
	0.9113908119182379,
	0.9213937793713197,
	0.9315065343673075,
	0.9417302818790887,
	0.9520662401047277,
	0.9625156406126187,
	0.9730797284882314,
	0.983759762482468,
	0.9945570151616474,
	1.0054727730591373,
	1.0165083368286487,
	1.0276650213992142,
	1.0389441561318664,
	1.0503470849780363,
	1.0618751666396906,
	1.0735297747312254,
	1.0853122979431382,
	1.0972241402074958,
	1.109266720865217,
	1.1214414748351933,
	1.1337498527852652,
	1.146193321305074,
	1.158773363080811,
	1.1714914770718874,
	1.184349178689538,
	1.19734799997739,
	1.2104894897940122,
	1.2237752139974656,
	1.2372067556318829,
	1.2507857151160926,
	1.2645137104343167,
	1.2783923773289583,
	1.2924233694955074,
	1.3066083587795838,
	1.3209490353761453,
	1.335447108030879,
	1.3501043042438072,
	1.3649223704751223,
	1.3799030723532875,
	1.3950481948854159,
	1.4103595426699613,
	1.4258389401117437,
	1.4414882316393327,
	1.457309281924819,
	1.4733039761059976,
	1.4894742200109872,
	1.5058219403853188,
	1.522349085121514,
	1.5390576234911828,
	1.5559495463796715,
	1.5730268665232825,
	1.590291618749101,
	1.6077458602174506,
	1.6253916706670133,
	1.643231152662637,
	1.6612664318458656,
	1.6794996571882146,
	1.6979330012472318,
	1.7165686604253645,
	1.7354088552316687,
	1.7544558305463922,
	1.77371185588846,
	1.7931792256858958,
	1.8128602595492127,
	1.832757302547802,
	1.8528727254893582,
	1.87320892520237,
	1.89376832482171,
	1.9145533740773621,
	1.935566549586314,
	1.9568103551476572,
	1.978287322040924,
	2.000000009327699
};

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

float globalBend = 1;

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
			sid.setFrequency(i, (lookup.lookup(pitch + (12 * voices[i].octaveShift)) * globalBend ) );
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

void handlePitchBend(byte channel, byte pitch, byte velocity){
	globalBend = bends[velocity];	

	for(int i = 0; i < 3; i++){
		if(voices[i].kbd == false){
			sid.setFrequency(i, (lookup.lookup( voices[i].note + (12 * voices[i].octaveShift)) * globalBend) );
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

        voices[0].mode = SID_SQUARE;
        sid.setMode(0,SID_SQUARE);
        voices[0].on = true;
        
        voices[1].mode = SID_SQUARE;
        sid.setMode(1,SID_SQUARE);
        voices[1].on = true;
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

			if(signal.control == 224){
				handlePitchBend(0, 0, signal.value);
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


 
