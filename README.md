# Sonoff RF Bridge R2 v2.2
This is a I2C capture of the MSM9066 programmer ($30 USD) made by flashing a blink program to the OBS38S003 8051 based micrcocontroller.  
This microcontroller is present in a newer generation of the Sonoff RF Bridge (433 MHz radio receiver used with smart home sensors).  

With these traces, flashing can hopefully being implemented with additional and less expensive devices.  
Ideally flashing would be supported by Tasmota itself once an alternative firmware is developed.  
An alternative firmware would provide control of the bridge radio decoding to support additional protocols and other features.  

The traces can be viewed with PulseView:  
https://sigrok.org/wiki/PulseView

Work on an alternative firmware for the Sonoff RF Bridge is being done here:  
https://github.com/mightymos/RF-Bridge-OB38S003
