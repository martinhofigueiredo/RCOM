#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "util.h"



void set_SET(unsigned char *buf){
     //Trama
    // F | A | C | Bcc1 | F
    buf[0] = FLAG;
    buf[1] = A_E;
    buf[2] = C_SET;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

void set_UA1(unsigned char *buf){
    //Trama
    buf[0] = FLAG;
    buf[1] = A_E;
    buf[2] = C_UA;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

void set_UA2(unsigned char *buf){
    //Trama
    buf[0] = FLAG;
    buf[1] = A_R;
    buf[2] = C_UA;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

void set_DISC1(unsigned char *buf){
    //Trama
    // F | A | C | Bcc1 | F
    buf[0] = FLAG;
    buf[1] = A_E;
    buf[2] = C_DISC;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

void set_DISC2(unsigned char *buf){
    //Trama
    // F | A | C | Bcc1 | F
    buf[0] = FLAG;
    buf[1] = A_R;
    buf[2] = C_DISC;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

void set_RR(unsigned char *buf, unsigned char RR){
    buf[0] = FLAG;
    buf[1] = A_E;
    buf[2] = RR;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

void set_REJ(unsigned char *buf, unsigned char REJ){
    buf[0] = FLAG;
    buf[1] = A_E;
    buf[2] = REJ;
    buf[3] = buf[1]^buf[2];
    buf[4] = FLAG;
}

unsigned char set_BCC2(unsigned char *buf,int size){
    unsigned char BCC2 = buf[0];
      for(int i = 1; i < size; i++)
            BCC2 ^= buf[i];

    return BCC2;
}

void alarmHandler(){

    alarmCount++;
    is_active = TRUE;
       
}

int state_machine(int state, unsigned char *buf, int C, int A){
    switch(state){
        case START:
            if(*buf == FLAG)
                state = FLAG_RCV;
            break;

        case FLAG_RCV:
            if(*buf == A)
                state = A_RCV;
            else if(*buf == FLAG)
                state = FLAG_RCV;
            else
                state = START;
            break;

        case A_RCV:
            if(*buf == C)
                state = C_RCV;
            else 
                state = START;
            break;

        case C_RCV:
            if(*buf== (A^C))
                state = BCC_RCV;
            else
                state = START;
            break;

        case BCC_RCV:
            if(*buf == FLAG){
                STOP = TRUE;
                alarm(0);
                UA_RCV = 1;
                state = FINISH;
            }
            else 
                state = START;
            break; 
        default:
            break;           

   
    }
    return state;
}


int set_Package(unsigned char *newBuf, char *buf, int bufSize, unsigned char BCC2, unsigned char C, int newSize){
    newBuf[0] = FLAG;
    newBuf[1] = A_E;
    newBuf[2] = C;
    newBuf[3] = newBuf[1]^newBuf[2];
    newSize = bytestuffing(buf, newBuf,bufSize, newSize);
    newBuf[newSize -2] = BCC2;
    newBuf[newSize -1] = FLAG; 
    return newSize;
}


int bytestuffing(char *buf, unsigned char *newBuf, int bufSize, int newSize){
    int index =4;

    for(int i=0; i<bufSize; i++){
        if(buf[i] == FLAG){
            newSize++;
            newBuf = (unsigned char *) realloc(newBuf,newSize);
            newBuf[index] = ESCAPE_1;
            newBuf[index+1] = ESCAPE_2; 
            index +=2;
        }
        else if(buf[i] == ESCAPE_1){
            newSize++;
            newBuf = (unsigned char *) realloc(newBuf,newSize);
            newBuf[index] = ESCAPE_1;
            newBuf[index+1] = ESCAPE_3;     
            index+=2;
        }
        else{
            newBuf[index] = buf[i];
            index++;
        }
    }
    
        printf("Stuffing all done\n");

    return newSize;
}

int bytedestuffing(unsigned char *buf, int size){
    int counter = 0;

    for(int i=0;i<size;i++){
        if(buf[i] == ESCAPE_1){
            if(buf[i+1]== ESCAPE_2){
                buf[i] = FLAG;
                for(int j=i+1;j<=size;j++){
                    buf[j] = buf[j+1];
                }
                counter++;
            }
            if(buf[i+1] == ESCAPE_3){
                for(int j = i+1;j<=size;j++){
                    buf[j] = buf[j+1];
                }
                counter++;
            }
        }
    }
    printf("Destuffing all done\n");
    return counter;
}

unsigned char control_machine(int fd,unsigned char flag1, unsigned char flag2, int alarmCount){
   
    int state = START;
    unsigned char flag;
    unsigned char received, received_C;
    unsigned char control;

      while(state!=FINISH){

        if(read(fd, &flag,1)<0)
            printf("Erro\n");

        switch(state){
            case START:
                if(flag == FLAG)
                    state=FLAG_RCV;
                break;

            case FLAG_RCV:
                received = flag;
                if(flag == A_E)
                    state = A_RCV;
                else if(flag != FLAG)
                    state = START;
                break;

            case A_RCV:
               received_C = flag;
                if(flag == flag1 || flag == flag2){
                    control = flag;
                    state = C_RCV;
                }
                else if (flag != FLAG)
                    state = FLAG_RCV;
                else 
                    state = START;
                break;
            
            case C_RCV:
                
                if(flag == (received ^ received_C))
                    state = BCC_RCV;
                else {
                    state = START;
                }
                break;
            
            case BCC_RCV:
                if(flag == FLAG)
                    state = FINISH;
                else 
                    state = START;
                break;

        }
     }


    alarm(0);
    alarmCount = 0;
    is_active = 0;
    return control;
}


