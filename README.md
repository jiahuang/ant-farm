ant-farm
========

Fuse the bits to clock the Attiny at 8mHz

lfuse sets the 1mHz internal clock
hfuse sets brownout detector at 1.8v

```
avrdude -p attiny24 -P /dev/cu.usbmodemfd121 -c avrisp -b 19200 -U lfuse:w:0x62:m -U hfuse:w:0xd8:m -U efuse:w:0xff:m
```

check the port that the attiny is connected to ```ls -l /dev/cu.*```

Increment the UUID and stick on the code
```
python build.py; make all; avrdude -p attiny24 -P /dev/cu.usbmodemfd121 -c avrisp -b 19200 -U flash:w:ant.hex;
```