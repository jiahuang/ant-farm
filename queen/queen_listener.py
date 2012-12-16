# listen to an arduino over serial port

import serial, json
import os
import subprocess

arduino = serial.Serial('/dev/cu.usbmodem1411', 57600) # serial port# and baud rate

def chunks(l, n):
  return [l[i:i+n] for i in range(0, len(l), n)]

def post_to_server(queen_id, ant_id):
  subprocess.call(["curl", "-X", "POST", "-d", "colony=" + colony_id, "-d", "ant=" + ant_id, "api.olinexpo.com/binds"])

def append_to_file(filename, data):
  with open(filename, "a") as f: # open file in append mode
    f.write(data+"\n")

def grab_ant_ids():
  old_ant_list_size = len(ant_ids)
  ant_ids.add(ant_id)
  if (len(ant_ids) > old_ant_list_size):
    append_to_file("ant_ids.txt", ant_id)
    
ant_ids = set()

prefix = 'Got payload: '
while 1:
  data = arduino.readline().strip()
  pos = data.find(prefix)
  if pos != -1:
    message = data[pos + len(prefix):]
    print "message:", json.dumps(message)
    # 8 bytes, each 2 characters big, so make minimum 16 to left pad with appropriate leading 0's
    # these might be reversed
    queen_id = message[0:2]
    colony_id = message[2:6]
    ant_id = message[6:14]
    ping_id = message[14:]

    # Revert to small-endian.
    colony_id = chunks(colony_id, 2)
    colony_id.reverse()
    colony_id = ''.join(colony_id)
    ant_id = chunks(ant_id, 2)
    ant_id.reverse()
    ant_id = ''.join(ant_id)

    # ping id is in big endian so dont reverse it
    #ping_id = chunks(ping_id, 2)
    #ping_id.reverse()
    #ping_id = ''.join(ping_id)
    # grab_ant_ids()

    # POST that biznatch
    print "queen:", json.dumps(queen_id), "colony:", json.dumps(colony_id), "ant:", json.dumps(ant_id), "ping:", json.dumps(ping_id)
    post_to_server(colony_id, ant_id);
    # read in the current id
    # subprocess.check_output(["echo", "Hello World!"])