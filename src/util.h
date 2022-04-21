#ifndef _UTIL_H_
#define _UTIL_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

struct termios oldtio;
struct termios newtio;

#define TRUE 1
#define FALSE 0

//Trama stuff
#define FLAG 0x7e

#define A_R 0X01
#define A_E 0x03


#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0b
#define C_RR 0x01
#define C_REJ 0x05

//REJs e RRs
#define C_0 0x00
#define C_1 0x20
#define C_RR1 0x21
#define C_REJ1 0x25

//State machine
#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4
#define FINISH 5


int STOP;
int UA_RCV;

#define TIMEOUT 3


//Alarm stuff
int alarmCount;
int is_active;

//Statistics
typedef struct Statistics{
    int retransmissions;
    int rej_sent;
    int timeOuts;
}Statistics;



//For stuffing and Destuffinng
#define ESCAPE_1 0x7d
#define ESCAPE_2 0x5e
#define ESCAPE_3 0x5d

void set_SET(unsigned char *buf);
void set_UA1(unsigned char *buf);
void set_UA2(unsigned char *buf);
void set_DISC1(unsigned char *buf);
void set_DISC2(unsigned char *buf);
void set_RR(unsigned char *buf, unsigned char RR);
void set_REJ(unsigned char *buf, unsigned char REJ);
unsigned char set_BCC2(unsigned char *buf, int size);
void alarmHandler();
int state_machine(int state, unsigned char *buf, int C, int A);
int set_Package(unsigned char *newBuf, char *buf, int bufSize, unsigned char BCC2, unsigned char C,int newSize);
int stuffing(char *buf, unsigned char *newBuf, int bufSize, int newSize);
int destuffing(unsigned char *buf, int size);
unsigned char control_machine(int fd,unsigned char flag1, unsigned char flag2, int alarmCount);

#endif // _UTIL_H_