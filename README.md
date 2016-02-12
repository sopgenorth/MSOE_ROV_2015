# Arduino/Processing UDP communciation
This is a basic library that provides a fairly robust way of communicating UDP data between Processing and an Arduino compatible board (this has mostly been tested on a Tiva C connected though). This library was originally intended for use on the MSOE underwater robot( https://www.facebook.com/UnderwaterRobotics), but could likely be useful for other things. 

Since UDP provides no data transfer guarantees, it is expected that all of the data transferred can safely be lost. However, the DataHandler and associated Arduino library do a pretty good job of re-establishing communication once its been lost.

computer_sync - This is a processing example that sets up the DataHandler class for UDP communication. This will attempt to connect to the microcontroller. 

controller_sync - This is an Energia example (probably would work on an Arduino too) that sets up the rovCOM library for connecting to the processing example. The rovCOM class is also setup to allow firmware updates over Ethernet using TI's LM Flash Utility. 
