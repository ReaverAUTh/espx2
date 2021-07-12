# ESPX - Project #2

An embedded device simulating the collection of nearby MAC addresses via bluetooth, in order to maintain a person's close contacts after a positive COVID-19 test.

## **1. Local Execution**
In order to test the implementation locally on your machine, use the files located in the home directory. Follow the commands in the order given below:

```
make clean
make all
./covid
```

## **2. RPI Execution**
If you wish to test the implementation on your RaspberryPi, you need to have a cross-compiler installed and an active ssh connection. Then execute the following commands (assuming the only user is *root*):

```
make clean
make rpi
scp rpi_covid root@192.168.0.1:~
```

When the program has been executed you need to retrieve the result files (contacts and delays). To do that, execute the following:

```
scp root@192.168.0.1:delays.bin delays.bin
scp root@192.168.0.1:close_contacts_COVID.bin close_contacts_COVID.bin
```

#

Repository for Aristotle Univerisity of Thessaloniki ECE Deparment, Real Time Embedded Systems 2nd project 
