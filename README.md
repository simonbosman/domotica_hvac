# domotica_hvac

I had a very [old thermostat](https://github.com/simonbosman/domotica_hvac/blob/master/old_hvac.jpg)
in my apartment, when I came home from a hard day's work I was welcomed with an artic blizzard.
Not a very welcoming scene, so I got some work to do.

I bought a jeenode master en a jeenode slave, a nice box, a relay and a temperature sensor

http://www.digitalsmarties.net/products/jeenode
http://www.digitalsmarties.net/products/jeelink-classic

I already had an Asus router with a MIPS processor and OpenWrt, hence made the [code in C](https://github.com/simonbosman/domotica_hvac/blob/master/domo.c) for the Asus router and the code for the two jeenode's in Arduino.
[masternode](https://github.com/simonbosman/domotica_hvac/blob/master/JeeMasterNode.ino)
[roomnode](https://github.com/simonbosman/domotica_hvac/blob/master/JeeRoomNode.ino)

This way I could remotely login to my router and set the temperature.
Oh, what I nice feeling getting home when it's all warm and cosy.

See the pics
![Jeenode usbstick plugged in the Asus router](https://github.com/simonbosman/domotica_hvac/blob/master/jeenode.jpg) 
![router](https://github.com/simonbosman/domotica_hvac/blob/master/mips_openwrt.jpg)
![new hvac](https://github.com/simonbosman/domotica_hvac/blob/master/new_hvac.jpg)
![new hvac closeup](https://github.com/simonbosman/domotica_hvac/blob/master/new_hvac2.jpg)
