# Arduino/Processing UDP communciation
This is a basic library that provides a fairly robust way of communicating UDP data between Processing and an Arduino compatible board (this has mostly been tested on a Tiva C connected though). 

Since UDP provides no data transfer guarantees, it is expected that all of the data transferred can safely be lost. However, the DataHandler and associated Arduino library do a pretty good job of re-establishing communication once its been lost.
