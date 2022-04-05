# If you don't have a serial port you can still implement the protocol and application.
- On Windows (you need cygwin):
http://www.mks.zp.ua/vspdxp.php
http://www.eltima.com/products/vspdxp/ (trial version)
http://www.virtual-serial-port.com/ (trial version)

- On Linux:
 https://tibbo.com/support/downloads/vspdl.html (VSPDL)

# Or
## 1. Install socat utility
```
sudo apt install socat

sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777
```
## 2. Download, compile, and run the virtual cable program "cable.c".
```
gcc cable.c -o cable
sudo ./cable
```

## 3. Connect the transmitter to ```/dev/ttyS10``` and the receiver to ```/dev/ttS11``` (or the other way around).


## 4. Type on and off in the cable program to plug or unplug the virtual cable.

- ```-- on``` &rarr; turns on the cable

- ```-- off``` &rarr; turns off the cable

- ```-- end ``` &rarr;  stops execution of the program
