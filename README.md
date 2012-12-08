ant-farm
========

Fuse the bits to clock the Attiny at 8mHz

lfuse sets the 8mHz internal clock
hfuse sets brownout detector at 2.7v

```
avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U lfuse:w:0xE2:m;
avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U hfuse:w:0xDD:m;
avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U efuse:w:0xFF:m;
```

check the port that the attiny is connected to ```ls -l /dev/cu.*```

Increment the UUID and stick on the code
```
python build.py; make all; avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U flash:w:ant.hex;
```