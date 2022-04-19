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


# Setup
For virtual Serial Ports


## 1. Copy your linklayer .o files into the protocol folder
- Done by the makefile in src, which compiles and puts the output in protocol folder
- To compile to objects, gcc needs the flag `-c` so that it doesn't need a reference to a main function in our library.
## 2. Compile the application with ./build.sh, which will build with your protocol .o code
- ```build.sh``` is called in the main make of the project
## 3. Compile and run the virtual cable program
- ```make cable``` compiles the cable and it has the fix for containers
## 4. Test the protocol

### 4.1 Run the receiver: 

~~```./main /dev/ttyS11 tx penguin-received.gif```~~ 

```./main /tmp/ttyS11 tx penguin-received.gif```

### 4.2 Run the transmitter: 
~~```./main /dev/ttyS10 tx penguin.gif```~~

```./main /tmp/ttyS10 tx penguin.gif``` 

### 4.3 Should have received a nice looking penguin

## 5. Test the protocol with cable unplugging and noise

### 5.1. Run receiver and transmitter again

### 5.2. Quickly move to the cable program console and press 0 for unplugging the cable, 2 to add noise, and 1 to normal 

### 5.3 Should have received a nice looking penguin even if cable disconnected or noise addded

---

# BUGFIX FOR Containers or non root users

- Change ``/dev/`` folder to ``/tmp/``

