#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Nano w/ ATmega328
#define __AVR_ATmega328P__
#define ARDUINO 105
#define __AVR__
#define F_CPU 16000000L
#define __cplusplus
#define __attribute__(x)
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__
#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define prog_void
#define PGM_VOID_P int
#define NOINLINE __attribute__((noinline))

typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

void setActiveVoice(word raw);
void modeSelect(word raw);
void setAttack(word raw);
void setDecay(word raw);
void setSustain(word raw);
void setRelease(word raw);
void toggleFilter(word raw);
void toggleLP(word raw);
void toggleHP(word raw);
void toggleKeyboardOn(word raw);
void filterFrequency(word raw);
void filterResonance(word raw);
void voicePW(word raw);
void voiceFrequency(word raw);
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handlePitchBend(byte channel, byte pitch, byte velocity);
//
//

#include "C:\Program Files (x86)\Arduino\hardware\arduino\variants\eightanaloginputs\pins_arduino.h" 
#include "C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\Charlotte\Documents\Arduino\SIDIController\SIDIController.ino"
#endif
