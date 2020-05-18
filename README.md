TeensyStripController 1.04
==========================

Firmware for a Teensy 3.1/3.2 or Teensy 4.0 to control WS2811/WS2812 based ledstrips. Fully compatible with the DirectOutput Framework.

![Teensy 3.1 with OctoWS2811 adaptor](http://www.pjrc.com/store/octo28_adaptor_6.jpg)


Hardware
--------
This code has been designed for Teensy 3.1 or Teensy 3.2. For easy installation of the ledstrips is is highly recommended to get a OctoWS2811 adaptor board as well. 
Both boards are available at http://pjrc.com/store/ 
For the Teensy be sure to get a Teensy 3.2, preferably the version which has the pins already soldered in.

Ledstrips using the WS2812 are widely available on Ebay, AliExpress, Adafruit (Neopixels) and many other sources. Be careful when ordering ledstrips. After all these are low cost china products with all the pros and cons these products have.

Make sure your power supply is strong enough. Every led needs up to 60ma and having strips with a lot of leds can easily drive you into high power requirements (e.g. 1m of ledstrip with 60 leds needs a PSU delivering at least 3.6A). Use thick wires for the power connections to the ledstrips and be sure to inject power after every 100-200 leds at least. Be careful to do the wiring correctly, otherwise the ledstrip, the wiring or even the PSU can get pretty hot which can be a fire hazard and which is bad for the lifetime of the whole setup. If using a lot of leds, be even more careful! A lot of leds means a lot of amperes and a lot of amperes with low voltage is also used for welding!

Software
--------
The code of this project can be compiled and installed on the Teensy with the Arduino IDE with installed Teensyduino extensions.

Currently the firmware supports up to 1100 leds on each of the  output 8 channels. Keep in mind that every led on a strip takes a little bit of time to update. To get some etimate on the max framerate you can achieve take the number of leds on your longest connected strip and multiply by 30microseconds. This will give you the approx time technically required for a single update of the strip. If you divide 1000000 by the result of the first calculation, you get the max update frequency (real update frequency will be a bit lower). Example: 500 (leds) * 30 microsconds= 15000 microseconds. 1000000 / 15000 = 66 Hz max update frequency (try to stay well above 30hz with your setup).

To drive the controller at least DirectOutput Framework R3 is required. Check out the DOF docu page on _BuiltIn Output controllers_ for details on the configuration of DOF for the TeensyStrip controller.

Integrated Product
------------------

The Oak Micros Pinball Addressable LEDs (PAL) board is a pre-built integrated product that can be used to control up to 8 addressable LEDs strips. It uses the Teensy 3.2 and the latest software from this site. It includes a pushbutton which can be used to initiate a LED test when powered up and has a pluggable screw connector for power and 6 LED connections. The remaining two other LED connections are available on an optional 0.1" header.

![Oak Micros PAL board](http://vpforums.org/imghost/24/pal_board.jpg)

For further information read the [user guide](https://drive.google.com/open?id=1Zk_5RxsWX4VIPhT4XtGlj1rNlBTug39a) and see [this thread](https://www.vpforums.org/index.php?showtopic=43482) on VPForums.org.

Documentation
-------------
More information can be found in the wiki for this project: https://github.com/DirectOutput/TeensyStripController/wiki


Firmware Downloads
------------------
Compiled firmware files can be downloaded from: https://github.com/DirectOutput/TeensyStripController/releases

License
-------
See License file in the repo.
