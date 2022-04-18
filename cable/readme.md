# Test Protocol for virtual serial port

## If you don't have a serial port you can still implement the protocol and application.
- On Windows (you need cygwin):
http://www.mks.zp.ua/vspdxp.php
http://www.eltima.com/products/vspdxp/ (trial version)
http://www.virtual-serial-port.com/ (trial version)

- On Linux:
 https://tibbo.com/support/downloads/vspdl.html (VSPDL)

## Or
### 1. Install socat utility
```
sudo apt install socat

sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777
```
### 2. Download, compile, and run the virtual cable program "cable.c".
```
gcc cable.c -o cable
sudo ./cable
```

### 3. Connect the transmitter to ```/dev/ttyS10``` and the receiver to ```/dev/ttS11``` (or the other way around).


### 4. Type on and off in the cable program to plug or unplug the virtual cable.

- ```-- on``` &rarr; turns on the cable

- ```-- off``` &rarr; turns off the cable

- ```-- end ``` &rarr;  stops execution of the program

#### BUGFIX FOR Containers or non root users

- Change ```/dev/``` folder to ```/tmp/```

# Test Protocol for Laboratory Serial Port

1. Connect to FEUP's VPN

2. Connect with SSH to the exposed computer on the lab (the ip is X.X.X.X)

3. Tunnel from that computer to root one and setup the link on both pc

4. Test protocol with a actual physical cable inbetween
