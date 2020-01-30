# HomeTheaterPC Project

HomeTheaterPC Project is a total remake of an old TV decoder box. The goal is to keep the case and making it fully working.
The final project can play videos, movies, music surf on Youtube, everything you'd like from a media box

### *How does it work*

This projects is in 3 main parts :
  * The hardware, which implies a lot of soldering and spare time
  * The hardware control, which is running on an chinese Arduino clone
  * The software, this part is running on a Raspberry Pi 3 with LibreELEC and a few configurations and scripts to work with the Arduino firmware

And that's it.

### *Libraries*

Standard Arduino libraries :
  * [Wire.h](https://www.arduino.cc/en/reference/wire), for the RTC library
  * [Keyboard.h](https://www.arduino.cc/en/Reference/MouseKeyboard), for the front keypad
  
Third party libraries :
  * [RTClib.h](https://github.com/adafruit/RTClib), for the RTC duh
  * [PT6961.h](http://gtbtech.com/?p=528), for the front LED screen
  * [IRremote.h](https://github.com/z3t0/Arduino-IRremote), for the infrared remote control

### *License*

HTPC Project is released under the [MIT](https://opensource.org/licenses/MIT) license
```
Copyright (c) 2017 R. Gaspard

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
