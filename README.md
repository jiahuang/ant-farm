ant-farm
========

```
ls -l /dev/cu.*

make all; avrdude -p attiny24 -P /dev/cu.usbmodem1421 -c avrisp -b 19200 -U flash:w:ant.hex;
```