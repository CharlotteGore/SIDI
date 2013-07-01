SIDI
====

This code is to control a SID using the [MOS6581 SID Control Library](https://github.com/CharlotteGore/MOS6581) library, taking MIDI input for frequency control and, currently, a load of analog inputs for everything else.

It also uses the [SIDFreak - MIDI note to SID Frequency lookup](https://github.com/CharlotteGore/SIDFreak) library, which helps translate MIDI notes into the correct SID phase accumulator values to achieve specific frequencies.

##Update

Have added EAGLE files for the board. This is the other half of the board that's in the MOS6581 library. It holds the 1mhz crystal, an ATMega328P, exposes I2C, ICSP and SID_DATA headers and has a MIDI-In on the UART. This needs programming via ICSP, as the RX pin is being used by the MIDI input.

Note that this board is for the future version of SIDI, which uses I2C peripherals for control data... and it's not back from the PCB printers yet so I don't know whether or not it works*. FUN TIMES.

![Schematic](https://raw.github.com/CharlotteGore/SIDI/master/SIDI_I2C.png)

*Actually the version I sent off to the printers don't work. The RESET pin on the ATMega328P is pulled to ground instead of +5v. Whoops. The schematic and board files have been updated in this repo, but it's Manual Bodge time when the boards arrive..

##Progress

Right now I have a basic but working control surface for controlling the SID.

![Control surface for the SID](https://github.com/CharlotteGore/MOS6581/raw/master/plexidreams.jpg)

It exposes all the registers, plus a little bit extra (octave shifting, switching between keyboard control and a frequency knob etc). Enough to actually make music on SIDs but not enough to truly bring out the full power of it. A couple of 4051 demux chips, a bunch of pots, some toggle switches with pull-up resistors and a timer interrupt vector to do periodic reading. Classic "My First MIDI Controller" stuff except with some nasty PORT manipulation stuff for fun.

Using standard analog potentiometers has proven to be a bit disappointing though, all in all. Keeping a track of what's what when you switch between the voices is incredibly difficult, jarring jumps etc.. total lack of repeatability of sounds. Hmmm.. is that a bad thing though? Probably is. Yes. 

Toggle switches are a terrible idea for these toggleable values too. They need to be buttons which turn LEDs on and off in order to show the current status properly. Having to flip a toggle twice to turn on something that looks like it should be on is just wrong. Bad UX.

The ideal solution - if potentially expensive and-requiring-SMD-soldering solution - is to switch to rotary encoders with LEDs indicating the current value. This means that when the underlying 'real' value changes, the musician can see where the dials are currently set at a glance. Relative controls!

This makes loading and saving patches something very very practical and intuitive, too. The long term plan of being able to load and save patches from an SD card is something that's always in the back of my mind. It's not enough to simply load the sounds - you want to see how those sounds are put together and tweak them in small ways, not instantly break them with no way of getting them back without reloading the patch.

The problem is that there's only a limited amount of rotary encoders you can realistically manage on a single ATMega328 chip. So, my solution (such as it is) is to basically have more. Lots more. Lots of slave ATMegas on an I2C bus. 

Each ATMega328 can handle either 6-7 Rotary Encoders (or 12-14 push buttons, I think) whilst using the SPI bus to controls LED drivers or shift registers and using the I2C bus to send and recieve current values for controls. 

The advantage of shoving all the controls onto an I2C bus is that the Master only needs know about stuff that's changed (and therefore requires some action to be taken). This liberates the master to devote more clock cycles to manipulating the SID. LFOs, amp envelopes, multiple SIDs for polyphony.. all become more viable. 

I've been doing some proof of concept work today, testing using PIN Change Interrupts for buttons and rotary encoders as well as checking how well the I2C bus performs at the basic 100khz speed and frankly it's more than good enough... and it can go four times faster. 

My next step is to implement my current analog control surface as a standalone I2C module and reprogram this app to use it rather than doing its own reading. This way I can basically swap out the control surface for just about anything, and that's a very good thing.
