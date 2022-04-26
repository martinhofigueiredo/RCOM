#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "linklayer.h"
#include "util.h"

//to llclose
linkLayer aux;

//globalizaarrrrrr
int fd;
unsigned char C = C_0;
Statistics stats;
int attemptsCounter = 0;

unsigned char calculator(){
    return (C == C_0) ? C_1 : C_0 ;
}

unsigned char rej_calculator(unsigned char flag){
    return (flag == C_0) ? C_REJ1 : C_REJ;
}
    
unsigned char rr_calculator(unsigned char flag){
    return (flag == C_0) ? C_RR1 : C_RR ;
}


int llopen(linkLayer connectionParameters){
        
      	(void)signal(SIGALRM, alarmHandle);

    //Stats initialization
    aux = connectionParameters;
    stats.retransmissions = 0;
    stats.rej_sent = 0;
    stats.timeOuts = 0;


    printf("\t************Connection Parameters************\n");
    printf("\tPort: %s\n",connectionParameters.serialPort);
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if(fd < 0){
        printf("Enable to open port\n");
    }

    //Flag initialization
    unsigned char SET[5];
    unsigned char UA[5];


    unsigned char buf;

   if(connectionParameters.role == TRANSMITTER){
       set_SET(SET);

         // Save current port settings
            if (tcgetattr(fd, &oldtio) == -1) {
                perror("tcgetattr");
                exit(-1);
            }

            // Clear struct for new port settings
            bzero(&newtio, sizeof(newtio));

            newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
            newtio.c_iflag = IGNPAR;
            newtio.c_oflag = 0;

            // Set input mode (non-canonical, no echo,...)
            newtio.c_lflag = 0;
            newtio.c_cc[VTIME] = 1;  // Inter-character timer unused
            newtio.c_cc[VMIN]  = 0;  // Blocking read until 5 chars received

            // VTIME e VMIN should be changed in order to protect with a
            // timeout the reception of the following character(s)

            // Now clean the line and activate the settings for the port
            // tcflush() discards data written to the object referred  to
            // by  fd but not transmitted, or data received but not read,
            // depending on the value of queue_selector:
            //   TCIFLUSH - flushes data received but not read.
            tcflush(fd, TCIOFLUSH);

            // Set new port settings
            if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
                perror("tcsetattr");
                exit(-1);
            }

            int state = START;
            UA_RCV = FALSE;
            
            do{
                write(fd, SET,5);
                printf("SET sent\n");
                alarm(connectionParameters.timeOut);
                is_active = FALSE;
                state = START;
               
                while(!UA_RCV && !is_active){
                    read(fd,&buf,1);
                    state=state_machine(state,&buf,C_UA,A_E);
                }
            }while(is_active && (alarmCount < 3));

            printf("UA received\n");
            alarmCount=0;
            is_active = 0;         
            
        }
        else if(connectionParameters.role == RECEIVER){
            set_UA1(UA);
            
            // Save current port settings
            if (tcgetattr(fd, &oldtio) == -1) {
                perror("tcgetattr");
                exit(-1);
            }

            // Clear struct for new port settings
            bzero(&newtio, sizeof(newtio));

            newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
            newtio.c_iflag = IGNPAR;
            newtio.c_oflag = 0;

            // Set input mode (non-canonical, no echo,...)
            newtio.c_lflag = 0;
            newtio.c_cc[VTIME] = 1;  // Inter-character timer unused
            newtio.c_cc[VMIN]  = 0;  // Blocking read until 5 chars received

            // VTIME e VMIN should be changed in order to protect with a
            // timeout the reception of the following character(s)

            // Now clean the line and activate the settings for the port
            // tcflush() discards data written to the object referred  to
            // by  fd but not transmitted, or data received but not read,
            // depending on the value of queue_selector:
            //   TCIFLUSH - flushes data received but not read.
            tcflush(fd, TCIOFLUSH);

            // Set new port settings
            if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
                perror("tcsetattr");
                exit(-1);
            }

            unsigned char buf;
            STOP = FALSE;
            int state = START;

            while(state !=FINISH){
                
                read(fd,&buf,1);
                state = state_machine(state,&buf, C_SET,A_E);
            } 
            printf("SET received\n");
            write(fd,UA,5);
            printf("UA sent\n");

        }

        if(alarmCount >= 3){
            printf("Failed to connect\n");
            return -1;
        }
        else
            return 1;
}

int llwrite(unsigned char *buf, int bufSize){
    

    int newSize = bufSize + 6;

    unsigned char *newBuf = (unsigned char *)malloc(sizeof(unsigned char) * (bufSize + 6));
    int bytesCounter=0;
   
    unsigned char flag1, flag2;
 

    unsigned char cnt=0;


    int retransmissions = -1;
    unsigned char BCC2;
    
    BCC2 = set_BCC2(buf, bufSize);
    
    //timeout variables;
    int alarmCount=0;
    newSize = set_Package(newBuf, buf, bufSize, BCC2, C, newSize);
    printf("%d\n", newSize);
    do{
        retransmissions++;
        if(retransmissions > MAX_RETRANSMISSIONS_DEFAULT){
            stats.retransmissions = MAX_RETRANSMISSIONS_DEFAULT;
            printf("Retransmissions reached the MAX_DEFAULT_VALUE\n");
            return -1;
        }

        if(alarmCount == aux.numTries){
            
            printf("Failed %d\n", alarmCount);
            return -1;
        }

        while(bytesCounter != newSize){
            bytesCounter = write(fd, newBuf,newSize);
        }

        flag1 = rej_calculator(newBuf[2]);
        flag2 = rr_calculator(newBuf[2]);

        printf("Control_machine\n");
        
        cnt = control_machine(fd,flag1,flag2,alarmCount);
  
    
    }while(cnt == flag1);

    stats.retransmissions +=retransmissions;
    C = calculator();
    free(newBuf);
    return newSize;    
}


int llread(unsigned char *packet){

    //size cnt variables
    int newSize= 0, position;

    //state machine variables;
    unsigned char flag,cnt;
    int state = START;

    //Message variables
    unsigned char *newBuf = (unsigned char *) malloc(sizeof(char)*newSize);    
    unsigned char helper;
    
    //reutilize the state machine, but with an upgrade to message receving
    while(state!=FINISH){

        if(read(fd,&flag,1)<0)
            printf("Error opening device por (file descriptor)\n");   

        switch(state){

            case START:
                if(flag == FLAG)
                    state = FLAG_RCV;
                break;
            
            case FLAG_RCV:
                if(flag == A_E)
                    state = A_RCV;
                else{
                    if(flag==FLAG)
                        state = FLAG_RCV;
                    else
                        state =START;
                }
                break;
            case A_RCV:
                if(flag == C_0 || flag == C_1){
                    state = C_RCV;
                    cnt = flag;
                }
                else{
                    if (flag == FLAG)
                        state = FLAG_RCV;
                    else 
                        state = START;
                }
                break;
            case C_RCV:
                if (flag == (A_E^cnt)){
                    state = BCC_RCV;
                    position = 0;
                }
                else
                    state = START;
                break;
            case BCC_RCV:
                ++newSize;
                newBuf = (unsigned char *)realloc(newBuf, newSize*sizeof(unsigned char));
                newBuf[position] = flag;
                
                if(flag == FLAG){
                           
                    helper = newBuf[newSize-2];
                    newSize = newSize - bytedestuffing(newBuf,newSize -2) - 2;

                    newBuf = (unsigned char *)realloc(newBuf, newSize);

                

                    unsigned char BCC2 = set_BCC2(newBuf,newSize);
                    
                    if( BCC2 == helper){
                        printf("ENtrei aqui:\n");
                        if(cnt == C_0){
                            C = C_RR1;
                            unsigned char RR[5];
                            set_RR(RR,C_RR1);
                            write(fd,RR,5);
                         
                        }
                        else{
                            printf("ENtrei aqui:\n");

                            C = C_RR;
                            unsigned char RR[5];
                            set_RR(RR,C_RR);
                            write(fd,RR,5);
                            
                        }
                    } 
                    else{
                            if(cnt == C_0){
                                printf("Case C_0:\n");
                                C = C_REJ1;
                                unsigned char REJ[5];
                                set_REJ(REJ,C_REJ1);
                                write(fd,REJ,5);
                            }
                            else{
                                printf("Case C_1:\n");
                                C = C_REJ;
                                unsigned char REJ[5];
                                set_REJ(REJ,C_REJ);
                                write(fd,REJ,5);
                            }
                            stats.rej_sent ++;
                            newSize = -1;
                        }

                    if(newSize!= -1){
                        for(int i=0; i<newSize; i++){
                            packet[i] = newBuf[i];
                        }
                        state = FINISH;
                    }
                    else{
                        printf("REJ sent\n");
                        return 0;
                    }
        }
        position++;
        break;
        }
    }
   free(newBuf);
   return newSize;
}

int llclose(int showStatistics){
    
    unsigned char UA[5];
    unsigned char DISC[5];

    int state;
    unsigned char buf, buf2;


    if(aux.role == TRANSMITTER){

        set_UA2(UA);
        set_DISC1(DISC);
        
        do{
            write(fd,DISC,5);
            printf("DISC sent\n");
            alarm(TIMEOUT);
            is_active = FALSE;
            state = START;

            while(state !=FINISH &&  !is_active){

                read(fd,&buf,1);
                state = state_machine(state,&buf,C_DISC, A_R);
            }
        }while(is_active && alarmCount < 3);

        if(alarmCount == 3)
            printf("DISC not received\n");

        else{
           
            printf("DISC received\n");
            write(fd,UA,5);
            printf("UA Sent\n");
            if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
                perror("tcsetattr");
                exit(-1);
            }
        }
    }

    else if(aux.role == RECEIVER){
        set_DISC2(DISC);

        state = START;
        while(state != FINISH){
            
            read(fd,&buf,1);
            state = state_machine(state, &buf,C_DISC, A_E);
        }

        printf("DISC received\n");
        is_active = FALSE;
        alarmCount = 0;
        alarm(0);

        do{
            write(fd,DISC,5);
            printf("DISC sent\n");
            alarm(TIMEOUT);
            is_active = FALSE;
            state = START;

            while(state != FINISH && !is_active){

                read(fd,&buf2,1);
                state_machine(state,&buf2,C_UA,A_R);
            }
            
        }while(is_active && alarmCount < 3);

        if(alarmCount == 3)
            printf("Did not receive UA\n");
        else{
            printf("UA received\n");

            if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
                perror("tcsetattr");
                exit(-1);
            }

  
        }
    }

    if(showStatistics){
        printf("\n");
        printf("\t************Global Statistics************\n");
        printf("\n");
        printf("\t*****Number of timeouts: %d \n", stats.timeOuts);
        printf("\t*****Number of retransmissions: %d\n",stats.retransmissions);
        printf("\t*****Number of rejs sent: %d\n", stats.rej_sent);
        printf("\n");
        printf("\t************Global Statistics************\n");
        printf("\n");
    }
    
    return 1;
}



