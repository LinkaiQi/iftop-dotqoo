import paho.mqtt.client as mqtt
import struct
import zlib
import thread
import paho.mqtt.publish as publish
import time
import socket
import argparse
import datetime
import mysql.connector

options = {}
table = {}
stat = {}
# src ip-address, src port-number, dst ip-address, dst port-number,
# protocol, data send/recv, timestamp
STRUCT_TYPE = '=LHLHHQQLL'
STRUCT_SIZE = struct.calcsize(STRUCT_TYPE)
received = 0
# mysql
cnx = None
cur = None



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
    global received

    received += 1
    decompressed_message = zlib.decompress(msg.payload)

    boxid = int(msg.topic.split("/")[-1])
    print len(decompressed_message)
    print "total: ",len(decompressed_message)/STRUCT_SIZE

    save_data(boxid, decompressed_message)
    print "Received reply from %d box"%(received)


# save data to 'table' dictionary
def save_data(boxid, decompressed_message):
    s = struct.Struct(STRUCT_TYPE)
    for i in range(0, len(decompressed_message), STRUCT_SIZE):

        outer_ip, outer_port, inner_ip, inner_port, protocol, recv, send,\
            create, end = s.unpack(decompressed_message[i:i+STRUCT_SIZE])

        # Swipe Endianness
        outer_ip = struct.unpack("<L", struct.pack(">L", outer_ip))[0]
        inner_ip = struct.unpack("<L", struct.pack(">L", inner_ip))[0]

        # filter ip start with "172.16.xxx.xxx"
        if inner_ip & 0xFFFF0000 != 0xAC100000:
            continue

        if (boxid, inner_ip) not in table:
            table[(boxid, inner_ip)] = []

        if options["port"]:
            table[(boxid, inner_ip)].append([outer_ip, outer_port, [protocol], recv, send, create, end])
        else:
            find = False
            for conn in table[(boxid, inner_ip)]:
                if conn[0] == outer_ip:
                    find = True
                    # merge stat info
                    if protocol not in conn[2]:
                        conn[2].append(protocol)
                    conn[3] += recv
                    conn[4] += send
                    if create < conn[5]:
                        conn[5] = create
                    if end > conn[6]:
                        conn[6] = end
                    break
            if not find:
                table[(boxid, inner_ip)].append([outer_ip, 0, [protocol], recv, send, create, end])


# counting total send/recv rate of each IP
def process():
    for inner_ip, info in table.iteritems():
        total_send = total_recv = 0
        for conn in info:
            total_recv += conn[3]
            total_send += conn[4]
        total = total_send + total_recv
        stat[inner_ip] = [total_recv, total_send, total]


# sorting helper function
def key_compare(x, y):
    return ((x[0] - y[0]) << 16) + (x[1] - y[1])


def readable_size(num, suffix='B'):
    for unit in ['','K','M','G','T','P','E','Z']:
        if abs(num) < 1000.0:
            return "%3.1f%s%s" % (num, unit, suffix)
        num /= 1000.0
    return "%.1f%s%s" % (num, 'Y', suffix)


def print_stat():
    for key in sorted(stat.keys(), cmp = key_compare):
        # item : (123456, 2886791075)
        info = stat[key]
        detail = table[key]
        output_str = "\n>>> Boxid: %d\tUserIP: %s\t\tTotal-send: %s\tTotal-recv: %s"\
            %(key[0],socket.inet_ntoa(struct.pack('!L', key[1])), readable_size(info[1]), readable_size(info[0]))
        print output_str
        print ' ' + "-" * (len(output_str) + 16)
        # print table title
        print "\t  ", "Visited-IP".ljust(15),
        if options["port"]:
            print "Port".rjust(5),
        print "Send".rjust(8), "Recv".rjust(8),"\t  Create / Last", "\tProtocols"

        # for conn in detail:
        for conn in sorted(detail, key=lambda connect: connect[3]+connect[4], reverse=True):
            str_ip = socket.inet_ntoa(struct.pack('!L', conn[0]))
            print "\t->", str_ip.ljust(15),

            # lookup hostname from IP
            '''
            try:
                print socket.gethostbyaddr(str_ip)[0],
            except:
                pass
            '''

            if options["port"]:
                if not conn[1]:
                    print "-".rjust(5),
                else:
                    print str(conn[1]).rjust(5),

            print readable_size(conn[4]).rjust(8),
            print readable_size(conn[3]).rjust(8),

            # strftime('%Y-%m-%d %H:%M:%S')
            ctime = datetime.datetime.fromtimestamp(conn[5]).strftime('%H:%M:%S')
            ltime = datetime.datetime.fromtimestamp(conn[6]).strftime('%H:%M:%S')
            print "\t%s / %s"%(ctime, ltime),

            protocols = "\t"
            for protocol in conn[2]:
                if protocol == 17:
                    protocols += "UDP/"
                elif protocol == 6:
                    protocols += "TCP/"
                elif protocol == 1:
                    protocols += "ICMP/"
                elif protocol == 2:
                    protocols += "IGMP/"
            print protocols[:-1]

            print  "  ", conn[0], '  '

            # solve hostname from database
            cur.execute(query_ip_host_pair%(16777216), ())

            for (hostname) in cur:
                print(hostname)




def read_arg():
	global options

	parser = argparse.ArgumentParser(description='Process network traffic stats send by iftop-dotqoo client')

	parser.add_argument('-p', '--port', action="store_true", default=False,
						help ='distinguish port')

	args = parser.parse_args()
	options = vars(args)


def print_options():
    print " ------------------------------------"
    print "| p: print received connection data  |"
    print "| b: broadcast to all remote box     |"
    print "| r boxid: retrieve connection data  |"
    print "|    according to a specified boxid  |"
    print "| q: quit                            |"
    print " ------------------------------------"



read_arg()
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("test.mosquitto.org", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

thread.start_new_thread(loop, ())

# mysql
try:
    # Connect with the MySQL Server
    cnx = mysql.connector.connect(user='root', database='network_traffic', password='QLK8812c')
    # Get two buffered cursors
    cur = cnx.cursor(buffered=True)
except Exception as e:
    print "Cannot connect to local mysql database"

query_ip_host_pair = "SELECT hostname FROM ip_hostname WHERE ip = %d"

time.sleep(3)
print_options()
while True:
    c = raw_input("Type selection: ")

    if c == 'p':
        if table == {}:
            print "Nothing to print"
        process()
        print_stat()

    elif c == 'b':
        # clear history
        received = 0
        table = {}
        stat = {}

        publish.single("NWSTAT/BROADCAST", "broadcast", hostname="test.mosquitto.org")
        print "Publish broadcast msg successful"

    elif len(c)>1 and c[0] == "r":
        try:
            r, boxid = c.split()
        except ValueError:
            print "Invalid input"
            continue

        # clear history
        received = 0
        table = {}
        stat = {}

        publish.single("NWSTAT/RETV/"+boxid, "retrieve", hostname="test.mosquitto.org")
        print "Publish msg successful"

    elif c == "q":
        break

    else:
        print "Invalid input"

# close mysql
if cnx != None:
    cur.close()
    cnx.close()



# end
