SparkFun OpenScale
===========================================================

[![SparkFun OpenScale](https://cdn.sparkfun.com//assets/parts/1/0/4/6/5/13261-01.jpg)](https://www.sparkfun.com/products/13261)

[*SparkFun OpenScale (SEN-13261)*](https://www.sparkfun.com/products/13261)

The SparkFun OpenScale is a simple-to-use, open source solution for measuring weight and temperature. It has the ability to read multiple types of load cells and offers a simple-to-use serial menu to configure calibration value, sample rate, time stamp and units of precision.

Simply attach a four-wire or five-wire load cell of any capacity, plug the OpenScale into a USB port, open a terminal window at 9,600bps, and you’ll immediately see mass readings. The SparkFun OpenScale will enable you to turn a load cell or four load sensors in a Wheatstone bridge configuration into the DIY weigh scale for your application.

The OpenScale was designed for projects and applications where the load was static (like the beehive in front of SparkFun HQ) or where constant readings are needed without user intervention (for example, on a conveyor belt system). A load cell with an equipped OpenScale can remain in place for months without needing user interaction!

On board the SparkFun OpenScale is the ATmega328P microcontroller, for addressing your communications needs and transferring your data to a serial terminal or to a data logger such as the OpenLog, an FT231 with mini USB, for USB to serial connection; the HX711, a 24-bit ADC for weigh scales; and the TMP102, for recording the ambient temperature of your system. The OpenScale communicates at a TTL level of 9,600bps 8-N-1 by default and possesses a baud rate configurable from 1,200bps to 1,000,000bps.

![The electronics monitoring the bee hive](https://raw.githubusercontent.com/sparkfun/OpenScale/master/Hive-Electronics.jpg)

You can read more about the beehive project that OpenScale is used in the [Internet of Bees](http://makezine.com/projects/bees-sensors-monitor-hive-health/) article.

Repository Contents
-------------------

* **/Documentation** - Images used in the hookup guide
* **/Production** - PCB panelized for production
* **/firmware** - Arduino sketch that runs on the ATmega328P
* **/hardware** - Eagle files

Documentation
--------------

* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/openscale-applications-and-hookup-guide)** - Basic hookup guide for the SparkFun OpenScale

License Information
-------------------

This product is _**open source**_! 

Various bits of the code have different licenses applied. Anything SparkFun wrote is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
