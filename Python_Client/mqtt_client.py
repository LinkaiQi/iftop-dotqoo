import paho.mqtt.client as mqtt
import struct
import zlib
import thread
import paho.mqtt.publish as publish
import time

def loop():
    client.loop_forever()

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("NWSTAT/AUTO/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    
    decompressed_message = zlib.decompress(msg.payload)
    
    #print(msg.topic+" "+str(msg.payload))
    struct_size = struct.calcsize('=LHLHHQQ')
    print len(decompressed_message)
    print "total: ",len(decompressed_message)/struct_size

    # ------------ experiment -------------
    
    # for i in range(0, len(data), 4):
    #s = struct.Struct('=ilqL')
    s = struct.Struct('=LHLHHQQ')
    
    for i in range(0, len(decompressed_message), struct_size):
    #pos = struct.unpack('i', data[i:i+4])
        unpacked_data = s.unpack(decompressed_message[i:i+struct_size])
        print 'Unpacked Values:', unpacked_data
    
    #unpacked_data = s.unpack(msg.payload[:20])
    #print 'Unpacked Values:', unpacked_data
    #unpacked_data = s.unpack(msg.payload[20:])
    #print 'Unpacked Values:', unpacked_data

    # --------- end of experiment ---------

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("test.mosquitto.org", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

thread.start_new_thread( loop, () )

time.sleep(2)
print "'Enter' to publish BROADCAST MSG (repeatable!)",
while True:
    raw_input()
    publish.single("NWSTAT/BROADCAST", "broadcast", hostname="test.mosquitto.org")
    print "Publish successful",

