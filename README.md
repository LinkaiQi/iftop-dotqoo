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
    ./configure
    open Makefile, find this line "LIBS = ....", add "-lpaho-mqtt3c -lz" flags
    ./make
    ./iftop [options]
```
