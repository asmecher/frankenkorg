# frankenkorg
Frankenkorg RK-100 brain replacement

This Arduino sketch implements a new brain for a Korg RK-100 MIDI keytar.
See http://www.cassettepunk.com for details.

## Circuit
<img alt="Fritzing circiut" src="https://raw.githubusercontent.com/asmecher/frankenkorg/master/frankenkorg.png"/>

WARNING: Klaatu barada necktie -- I made this Fritzing diagram after I built the protoboard, more or less from memory.
One known errata is that it doesn't include pull-up resistors on the data bus (Arduino pins 21-28). You can get by without
these by modifying the sketch to set internal pull-ups on these pins in the `setup` function.

I'd strongly suggest understanding the circuit rather than taking it literally, as I haven't double-checked it.

## Status
Reliable and usable.

# Todo
- Improve behavior of state changes (e.g. octave shifts, program changes) when keys are pressed. Currently they will remain pressed.
