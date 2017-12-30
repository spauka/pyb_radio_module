/*
The MIT License (MIT)

Copyright (c) 2017 Sebastian Pauka

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "NCSSPybRadio.h"

NCSSPybRadio module;

int main()
{
    // Initialise the module and radio
    module.init();
    module.radio.enable();

    while (true) {
        //module.led_io.setDigitalValue(1);
        module.led_io.setAnalogValue(12);
        module.radio.datagram.send("ON");
        module.sleep(5000);
        module.led_io.setAnalogValue(0);
        module.radio.datagram.send("OFF");
        module.sleep(5000);
    }

    // We will never get here, but this would put us in a waiting loop.
    // Use if we don't use main.
    release_fiber();
}

