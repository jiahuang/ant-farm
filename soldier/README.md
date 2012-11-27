#Soldier. The verification and reprogramming bench for ants

runs off of an arduino uno

uses the following libraries:

* [RF24](https://github.com/maniacbug/RF24)

Note: set the RF channel to 2 on the RF24 library since that's the frequency the Ant is set at. Line 387 in RF24.cpp should read: 

```
setChannel(2);
```
