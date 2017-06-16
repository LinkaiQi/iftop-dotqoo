# -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
import struct
import zlib
import thread
import paho.mqtt.publish as publish
import time
import socket
import argparse
import datetime

options = {}
table = {}
stat = {}
# src ip-address, src port-number, dst ip-address, dst port-number,
# protocol, data send/recv, timestamp
STRUCT_TYPE = '=LHLHHQQLL'
STRUCT_SIZE = struct.calcsize(STRUCT_TYPE)
received = 0
# MQTT global vars
mqtt_connected = False
client = None
# ip-hostname database
IPL = None



class IPLocator :
	def __init__( self, ipdbFile ):
		self.ipdb = open( ipdbFile, "rb" )
		str = self.ipdb.read( 8 )
		(self.firstIndex,self.lastIndex) = struct.unpack('II',str)
		self.indexCount = (self.lastIndex - self.firstIndex)/7+1
		print self.getVersion()," 纪录总数: %d 条 "%(self.indexCount)

	def getVersion(self):
		s = self.getIpAddr(0xffffff00L)
		return s

	def getAreaAddr(self,offset=0):
		if offset :
			self.ipdb.seek( offset )
		str = self.ipdb.read( 1 )
		(byte,) = struct.unpack('B',str)
		if byte == 0x01 or byte == 0x02:
			p = self.getLong3()
			if p:
				return self.getString( p )
			else:
				return ""
		else:
			self.ipdb.seek(-1,1)
			return self.getString( offset )

	def getAddr(self,offset,ip=0):
		self.ipdb.seek( offset + 4)
		countryAddr = ""
		areaAddr = ""
		str = self.ipdb.read( 1 )
		(byte,) = struct.unpack('B',str)
		if byte == 0x01:
			countryOffset = self.getLong3()
			self.ipdb.seek( countryOffset )
			str = self.ipdb.read( 1 )
			(b,) = struct.unpack('B',str)
			if b == 0x02:
				countryAddr = self.getString( self.getLong3() )
				self.ipdb.seek( countryOffset + 4 )
			else:
				countryAddr = self.getString( countryOffset )
			areaAddr = self.getAreaAddr()
		elif byte == 0x02:
			countryAddr = self.getString( self.getLong3() )
			areaAddr = self.getAreaAddr( offset + 8 )
		else:
			countryAddr = self.getString( offset + 4 )
			areaAddr = self.getAreaAddr()
		return countryAddr + " " + areaAddr

	def setIpRange(self,index):
		offset = self.firstIndex + index * 7
		self.ipdb.seek( offset )
		buf = self.ipdb.read( 7 )
		(self.curStartIp,of1,of2) = struct.unpack("IHB",buf)
		self.curEndIpOffset = of1 + (of2 << 16)
		self.ipdb.seek( self.curEndIpOffset )
		buf = self.ipdb.read( 4 )
		(self.curEndIp,) = struct.unpack("I",buf)

	def getIpAddr(self,ip):
		L = 0
		R = self.indexCount - 1
		while L < R-1:
			M = (L + R) / 2
			self.setIpRange(M)
			if ip == self.curStartIp:
				L = M
				break
			if ip > self.curStartIp:
				L = M
			else:
				R = M
		self.setIpRange( L )
		#version information,255.255.255.X,urgy but useful
		if ip&0xffffff00L == 0xffffff00L:
			self.setIpRange( R )
		if self.curStartIp <= ip <= self.curEndIp:
			address = self.getAddr( self.curEndIpOffset )
			#把GBK转为utf-8
			address = unicode(address,'gbk').encode("utf-8")
		else:
			address = "未找到该IP的地址"
		return address

	def getIpRange(self,ip):
		self.getIpAddr(ip)
		range = self.ip2str(self.curStartIp) + ' - ' \
			+ self.ip2str(self.curEndIp)
		return range

	def getString(self,offset = 0):
		if offset :
			self.ipdb.seek( offset )
		str = ""
		ch = self.ipdb.read( 1 )
		(byte,) = struct.unpack('B',ch)
		while byte != 0:
			str = str + ch
			ch = self.ipdb.read( 1 )
			(byte,) = struct.unpack('B',ch)
		return str

	def ip2str(self,ip):
		return str(ip>>24)+'.'+str((ip>>16)&0xffL)+'.' \
			+str((ip>>8)&0xffL)+'.'+str(ip&0xffL)

	def str2ip(self,s):
		(ip,) = struct.unpack('I',socket.inet_aton(s))
		return ((ip>>24)&0xffL)|((ip&0xffL)<<24) \
			|((ip>>8)&0xff00L)|((ip&0xff00L)<<8)

	def getLong3(self,offset = 0):
		if offset :
			self.ipdb.seek( offset )
		str = self.ipdb.read(3)
		(a,b) = struct.unpack('HB',str)
		return (b << 16) + a





def loop():
    client.loop_forever()


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    global mqtt_connected

    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("NWSTAT/AUTO/#")

    mqtt_connected = True


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global received

    received += 1
    decompressed_message = zlib.decompress(msg.payload)

    boxid = int(msg.topic.split("/")[-1])

    save_data(boxid, decompressed_message)
    print "Received reply from %d boxes"%(received), " boxid:", boxid, " total:", len(decompressed_message)/STRUCT_SIZE


# save data to 'table' dictionary
def save_data(boxid, decompressed_message):
    global received, table, stat

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
    global received, table, stat

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
    global IPL, received, table, stat

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
        counter = 0
        for conn in sorted(detail, key=lambda connect: connect[3]+connect[4], reverse=True):
            # limit output
            if options["limit"] != None and counter >= options["limit"]:
                break
            counter += 1;

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
            print protocols[:-1].ljust(10),

            # print hostname
            if IPL != None:
                print IPL.getIpAddr(conn[0]),

            print ''



def read_arg():
    global options

    parser = argparse.ArgumentParser(description='Process network traffic stats send by iftop-dotqoo client')
    parser.add_argument('-p', '--port', action="store_true", default=False, help ='distinguish port')
    parser.add_argument('-l', dest='limit', action='store', type=int,
                        help ='the limit amount of connection info will be displayed for each client')

    args = parser.parse_args()
    options = vars(args)

    print options


def print_options():
    print " ------------------------------------"
    print "| p: print received connection data  |"
    print "| b: broadcast to all remote box     |"
    print "| r boxid: retrieve connection data  |"
    print "|    according to a specified boxid  |"
    print "| q: quit                            |"
    print " ------------------------------------"

def connect_MQTT():
    global client

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("test.mosquitto.org", 1883, 60)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.

    thread.start_new_thread(loop, ())


def main():
    global IPL, received, table, stat

    read_arg()

    try:
        IPL = IPLocator( "qqwry.dat" )
    except Exception as e:
        print "Cannot open ip-hostname data file"

    connect_MQTT()
    # wait mqtt to connect
    while not mqtt_connected:
        pass

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


if __name__ == "__main__" :
	main()

# end
