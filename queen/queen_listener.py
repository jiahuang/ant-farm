# listen to an arduino over serial port

import serial
import os
import subprocess

arduino = serial.Serial('/dev/tty.usbmodemfa131', 57600) # serial port# and baud rate

def post_to_server(queen_id, ant_id, ping_id):
  subprocess.call(["curl", "-X", "POST", "-d", "colony=" + colony_id, "-d", "ant=" + ant_id,"-d", "ping=" + ping_id, "api.olinexpo.com/binds"])

prefix = 'Got payload: '
while 1:
  data = arduino.readline().strip()
  pos = data.find(prefix)
  if pos != -1:
    message = data[pos + len(prefix):]
    print "message:", message
    # these might be reversed
    colony_id = message[0:4]
    ant_id = message[4:12]
    ping_id = message[12:16]
    print "colony:", colony_id, "ant:", ant_id, "ping_id:", ping_id
    post_to_server(colony_id, ant_id, ping_id);
    # subprocess.check_output(["echo", "Hello World!"])

