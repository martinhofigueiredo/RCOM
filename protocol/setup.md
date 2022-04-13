# Setup
For virtual Serial Ports


## 1. Copy your linklayer .o files into the protocol folder
- Done by the makefile in src, which compiles and puts the output in protocol folder
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

