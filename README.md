ant-farm
========

Fuse the bits to clock the Attiny at 8mHz
```
avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U lfuse:w:0xE2:m;
avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U hfuse:w:0xD7:m;
avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U efuse:w:0xFF:m;
```

```
ls -l /dev/cu.*

make all; avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U flash:w:ant.hex;
```