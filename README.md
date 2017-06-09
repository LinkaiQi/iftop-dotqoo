## README for iftop-dotqoo

This is a modified version of iftop. This variant aims to add the functionality of caculating the statistic information about network connections and send back the statistic information through the network.

Also, network package filters are added to this version. see help messages (-h option) for more information

In order to optimize the performance, many features of iftop has been disabled. It is only designed for monitoring IP_V4 packages.

### To use iftop-dotqoo, you need libpcap, ncurses, libz libraries:

```
sudo apt-get install automake (optional)
sudo apt-get install libpcap-dev
sudo apt-get install libncurses5-dev libncursesw5-dev
```

### And MQTT client:

    for more information please visit the website: https://eclipse.org/paho/clients/c/

## Compile and Run:

```
    $ ./configure
    open Makefile, find line start with "LIBS = ....", add "-lpaho-mqtt3c -lz" flags
    $ ./make
    $ ./iftop [options]
    
    test MQTT client is located at Python_Client folder, 
    it can be used for retrieve connection data and publish 'BROADCAST' MQTT message
    $ python mqtt_client.py to run
```

### command line input arguments
```
Synopsis: iftop -h | [-q] [-T seconds] [-Y Bytes]
                     [-D seconds] [-M mins]

   iftop:
   -----------------------------------------------------------------------
   -h                  display this message
   -p                  run in promiscuous mode (show traffic between other
                       hosts on the same network segment)
   -i interface        listen on named interface
   -f filter code      use filter code to select packets to count
                       (default: none, but only IP packets are counted)
   -F net/mask         show traffic flows in/out of IPv4 network

   dotqoo:
   -----------------------------------------------------------------------
   -Z port,port...     filter port
   -W                  write info to file(/tmp)
   -Y seconds          delete info interval
   -T threshold        drop info when info_len less than threshold(bytes)
   -S                  send info to socket
   -X seconds          write info to file interval
   -q                  ignore control block pkt
   -M mins             time interval of send connection info over MQTT
   -D seconds          drop the info when no pkt send/recv within -D sec
                       after the connection has been created
   -z ip_p,ip_p...     block the protocol types which follow by '-z'
                       protocol type number can find at
                       wikipedia.org: 'List_of_IP_protocol_numbers'

iftop, modified by dotqoo from version: 1.0pre4
```
