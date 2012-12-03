# listen to an arduino over serial port

import serial
import os
import subprocess

arduino = serial.Serial('/dev/tty.usbmodem1411', 57600) # serial port# and baud rate

def post_to_server(queen_id, ant_id):
  subprocess.check_output(["curl", "localhost:3000/hardware/" + queen_id + "/" + ant_id])

prefix = 'Got payload: '
while 1:
  data = arduino.readline().strip()
  pos = data.find(prefix)
  if pos != -1:
    message = data[pos + len(prefix):]
    print "message:", message
    # 8 bytes, each 2 characters big, so make minimum 16 to left pad with appropriate leading 0's
    # these might be reversed
    queen_id = message[0:8]
    ant_id = message[8:]
    print "queen:", queen_id, "ant:", ant_id
    # post_to_server(queen_id, ant_id);
    # subprocess.check_output(["echo", "Hello World!"])

