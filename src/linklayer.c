#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "linklayer.h"

//Receiver
#define FLAG_RCV 0x7E
#define A_RCV 0x01
#define C_RCV 0x07
//Transmiter
#define FLAG_TM 0x7E
#define A_TM 0x03
#define C_TM 0x03
#define ESCAPE 0x7d

int timeout = FALSE, count=1;
int STOP = FALSE;
int fd;
int ACK_RCV = FALSE;
int REJ = FALSE;
int Ns = 0, Nr = 1;
linkLayer linklayer;

//State machine
typedef enum{
  START, FLAG, A, C, BCC, S_STOP
}states;

//State machine to receive data
//State machine to receive data
typedef enum{
    data_START, data_FLAG, data_A, data_C, BCC1, DATA, BCC2, data_S_STOP
}data_states;

int send_SET(int fd);
int receive_SET(int fd);
int send_UA(int fd, char ch);
int receive_UA(int fd, char ch);
int send_DISC(int fd, char ch);
int receive_DISC(int fd, char ch);
void alarm_handler();
int send_DATA(int fd, char* buffer, int buffSize);
int receive_DATA(int fd, char* buffer, int buffSize);
int send_ACK(int fd, char ch);
int receive_ACK(int fd, char ch);
int byteDestuffing(char *trama, int length, char *novaTrama);
int byteStuffing(char* trama, int length, char* novaTrama);

//=====================================================================================
// Opens a conection using the "port" parameters defined in struct linkLayer,
// returns "-1" on error and "1" on sucess

int llopen(linkLayer connectionParameters){   

  int c, result;
  struct termios oldtio, newtio;
  char buf[255];
  int i, sum = 0, speed = 0;
  linklayer = connectionParameters;
    
  if ((strcmp("/dev/ttyS3", linklayer.serialPort)!=0) && 
        (strcmp("/dev/ttyS4", linklayer.serialPort)!=0) ) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  fd = open(linklayer.serialPort, O_RDWR | O_NOCTTY );
  if (fd <0) {perror(connectionParameters.serialPort); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = linklayer.baudRate | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0;   /* 1 second timeout t=0.1*TIME s*/
  newtio.c_cc[VMIN]  = 0;   /* read will be satisfied if a single is read*/

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  if(linklayer.role == 0){     //Transmitter

    (void) signal(SIGALRM, alarm_handler);

    result = send_SET(fd);   //SET send
    printf("First SET sended: %d\n", result);

    timeout=FALSE;
                                                                                                    
    while(STOP == FALSE && (count < 4)){  //retransmite até 3x

      // If exceeded (time-out), forces retransmission
      if(timeout == TRUE){ //&& (count < 4)){
        count++;
        result = send_SET(fd); //Send SET
        printf("Resending SET: %dbytes\n", result);
        timeout=FALSE;
      }
      
      if(timeout==FALSE){
        printf("timeout false\n");
        alarm(3);
        receive_UA(fd, 'R');
      }
        
    }
  }else if(linklayer.role == 1){     //Receiver

    receive_SET(fd);
    result = send_UA(fd,'R'); //Send UA from receiver
    printf("Bytes received: %d bytes\n", result);
  }
  alarm(0);
  return 1;

}

// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize){
  int result;

  (void) signal(SIGALRM, alarm_handler);
  result = send_DATA(fd, buf, bufSize);   //DATA sent from transmiter
  printf("First DATA sent: %d\n", result);
  timeout=FALSE;
  ACK_RCV = FALSE;
  REJ = FALSE;

  while(ACK_RCV == FALSE && (count < 4)){  //retransmite até 3x
    // If exceeded (time-out), forces retransmission
    if(timeout == TRUE || REJ == TRUE){ 
      count++;
      result = send_DATA(fd, buf, bufSize);
      printf("Resending DATA: %dbytes\n", result);
      timeout=FALSE;
      REJ = FALSE;
    }
    
    if(timeout==FALSE){
      printf("timeout false\n");
      alarm(3);
      if(REJ == TRUE){
        receive_ACK(fd, 'N'); //Receive Negative ACK (REJ)  from receiver
      }
      else{
        receive_ACK(fd, 'P'); //Receive positive ACK (RR) from receiver
      }
    }
  }
  //Ligação corre bem
  if(ACK_RCV == TRUE){
    if(Ns == 0 && Nr == 1){
      Ns = 1;
      Nr = 0;
    }
    else if(Ns == 1 && Nr == 0){
      Ns = 0;
      Nr = 1;
    }
  }
  alarm(0);
  return result;
}

// Receive data in packet
int llread(char* packet){
  int result; 

  if(ACK_RCV == TRUE){
    if(Ns == 0 && Nr == 1){
      Ns = 1;
      Nr = 0;
    }
    else if(Ns == 1 && Nr == 0){
      Ns = 0;
      Nr = 1;
    }
  }

  result = receive_DATA(fd, packet, 2000);
  printf("Bytes received: %d bytes", result);

  if(REJ == TRUE){
    send_ACK(fd, 'N'); //Send Negative ACK (REJ) from receiver
  }
  else{
    send_ACK(fd, 'P'); //Send Positive ACK (RR) from receiver
  }

  return result;
}

//======================================================================================
/* Closes previously opened connection; if showStatistics==TRUE, link layer should print 
statistics in the console on close*/

int llclose(int showStatistics){
  int result;

  if(linklayer.role == 0){

    (void) signal(SIGALRM, alarm_handler);
    result = send_DISC(fd, 'T');   //DISC sent from transmiter
    printf("First DISC sent: %d\n", result);
    timeout=FALSE;
    STOP = FALSE;

    while(STOP == FALSE && (count < 4)){  //retransmite até 3x
      // If exceeded (time-out), forces retransmission
      if(timeout == TRUE){ //&& (count < 4)){
        count++;
        result = send_DISC(fd, 'T');
        printf("Resending DISC: %dbytes\n", result);
        timeout=FALSE;
      }
      
      if(timeout==FALSE){
        printf("timeout false\n");
        alarm(3);
        receive_DISC(fd, 'R');
      }
    }

    if(STOP == TRUE){
      result = send_UA(fd, 'T');
      printf("UA sent: %d\n", result);
    }
    
  }
  else if(linklayer.role == 1){
    receive_DISC(fd, 'T');
    result = send_DISC(fd,'R');
    printf("Bytes sent from recetor: %d bytes\n", result);
    
    receive_UA(fd,'T');

  }
 
  close(fd);
}

//======================================================================================

void alarm_handler()  // atende alarme
{
  timeout = TRUE;
}

int send_SET(int fd){
  
  int res;
  //Frame
  unsigned char buffer[5]; 

  printf("Sending SET\n");
  buffer[0] = 0x7E;                 //F
  buffer[1] = 0x03;                 //A   Tx ---> Rx
  buffer[2] = 0x03;                 //C    
  buffer[3] = buffer[1]^buffer[2];  //BCC = A^C
  buffer[4] = 0x7E;                 //F
  
  res = write(fd, buffer, sizeof(buffer));
  if(res < 0) printf("Error reading\n");

  return res;
}

int receive_SET(int fd){
  printf("Entrei no receive_SET\n");
  states state = START;
  int res = 0;
  char character;
  unsigned char buffer[5];
  int pos = 0;
  STOP = FALSE;

  while(STOP == FALSE){
    res = read(fd, &character, 1);   /* returns after 1 chars have been input */
    if(res < 0){
      printf("Error reading\n");
    }
    buffer[pos]=character;

    switch(state){
      case START:
        if(buffer[pos] == FLAG_TM){
          state = FLAG;
          pos = 1;
        }
         else {
           state = START;
           pos = 0;                                                      
           }
        break;

      case FLAG:
        if(buffer[pos] == A_TM){
          state = A;
          pos = 2;
        }
        else if(buffer[pos] == FLAG_TM){
          state = FLAG; 
          pos = 1;
        }
         else {
           state = START;
           pos = 0;
         }
        break;

      case A:
        if(buffer[pos] == C_TM){
          state = C;
          pos = 3;
        }
        else if(buffer[pos] == FLAG_TM){
          state = FLAG;
          pos = 2;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case C:
        if(buffer[pos] == A_TM^C_TM){
          state = BCC;
          pos = 4;
        }
        else if(buffer[pos] == FLAG_TM){
          state = FLAG;
          pos = 1;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case BCC:
        if(buffer[pos] == FLAG_TM){
          state = S_STOP;
        }
        else {
          state = START;
          pos = 0;
        }
        break;

      case S_STOP:
        STOP = TRUE;
        break;
    }
  }
  return res;
}

int send_DISC(int fd, char ch){
  int res;
  unsigned char buffer[5]; 

  if(ch == 'T'){
    printf("\nSending DISC from transmiter\n");
    buffer[0] = 0x7E;                 //F
    buffer[1] = 0x03;                 //A   Tx ---> Rx
    buffer[2] = 0x0B;                 //C    
    buffer[3] = buffer[1]^buffer[2];  //BCC = A^C
    buffer[4] = 0x7E;                 //F
  }
  else if(ch == 'R'){
    printf("Sending DISC from recetor\n");
    buffer[0] = 0x7E;                 //F
    buffer[1] = 0x01;                 //A   Rx ---> Tx
    buffer[2] = 0x0B;                 //C    
    buffer[3] = buffer[1]^buffer[2];  //BCC = A^C
    buffer[4] = 0x7E;                 //F
  }

  res = write(fd, buffer, sizeof(buffer));
  if(res < 0) printf("Error reading\n");

  return res;
}

int receive_DISC(int fd, char ch){
  printf("\nEntrei no receive_DISC\n");
  states state = START;
  int res = 0;
  char character;
  unsigned char buffer[5];
  int pos = 0;
  char adress, control=0x0B;
  STOP = FALSE;

  if(ch == 'T'){
    adress = 0x03;
  }
  else if(ch = 'R'){
    adress = 0x01;
  }

  while(STOP == FALSE){
    res = read(fd, &character, 1);   /* returns after 1 chars have been input */
    if(res < 0){
      printf("Error reading\n");
    }
    buffer[pos]=character;

    if(timeout==TRUE){ //&& ch == 'R'){
      return 0;
    }

    switch(state){
      case START:
        if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 1;
        }
         else {
           state = START;
           pos = 0;                                                      
           }
        break;

      case FLAG:
        if(buffer[pos] == adress){
          state = A;
          pos = 2;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG; 
          pos = 1;
        }
         else {
           state = START;
           pos = 0;
         }
        break;

      case A:
        if(buffer[pos] == control){
          state = C;
          pos = 3;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 2;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case C:
        if(buffer[pos] == adress^control){
          state = BCC;
          pos = 4;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 1;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case BCC:
        if(buffer[pos] == FLAG_RCV){
          state = S_STOP;
        }
        else {
          state = START;
          pos = 0;
        }
        break;

      case S_STOP:
        STOP = TRUE;
        break;
    }
  }
  return res;
}

int send_UA(int fd, char ch){
  printf("Entrei no send_UA\n");
  int res;
  //Frame
  unsigned char buffer[5]; 

  if(ch == 'T'){
    printf("Sending UA from transmiter\n");
    buffer[0] = 0x7E;                 //F
    buffer[1] = 0x03;                 //A   Tx ---> Rx
    buffer[2] = 0x07;                 //C    
    buffer[3] = buffer[1]^buffer[2];  //BCC = A^C
    buffer[4] = 0x7E;                 //F
  }
  else if(ch == 'R'){
    printf("Sending UA from recetor\n");
    buffer[0] = 0x7E;                 //F
    buffer[1] = 0x01;                 //A   Rx ---> Tx
    buffer[2] = 0x07;                 //C    
    buffer[3] = buffer[1]^buffer[2];  //BCC = A^C
    buffer[4] = 0x7E;                 //F
  }

  res = write(fd, buffer, sizeof(buffer));
  if(res<0) printf("Error reading\n");
    
  return res;
}

int receive_UA(int fd, char ch){
  printf("Entrei no receive_UA\n");

  states state = START;
  int res;
  char character;
  unsigned char buffer[5];
  int pos = 0;
  char adress;
  STOP = FALSE;

  if(ch == 'T'){
    adress = 0x03;
  }
  else if(ch = 'R'){
    adress = 0x01;
  }

  while(STOP == FALSE){
    
    res = read(fd, &character, 1);   /* returns after 1 chars have been input */
    //printf("Cheguei aqui\n");
    if(res < 0){
      printf("Error reading\n");
    }
    buffer[pos] = character;

    if(timeout==TRUE){
      return 0;
    }

    switch(state){
      
      case START:
        if(buffer[pos] == FLAG_RCV){
          //printf("State machine do SET: START");
          state = FLAG;
          pos = 1;
        }
         else {
           state = START;
           pos = 0;                                                      
           }

        break;

      case FLAG:
        if(buffer[pos] == adress){
          state = A;
          pos = 2;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG; 
          pos = 1;
        }
         else {
           state = START;
           pos = 0;
         }
        break;

      case A:
        if(buffer[pos] == C_RCV){
          state = C;
          pos = 3;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 2;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case C:
        if(buffer[pos] == adress^C_RCV){
          state = BCC;
          pos = 4;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 1;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case BCC:
        if(buffer[pos] == FLAG_RCV){
          state = S_STOP;
        }
        else {
          state = START;
          pos = 0;
        }
        break;

      case S_STOP:
        STOP = TRUE;
        printf("\nLigação completa\n");
        break;
    }
  }
  printf(buffer);
  return res;
}

int send_DATA(int fd, char* buffer, int buffSize){
  printf("Entrei no send_DATA\n");
  int res, i, new_size;
  char new_buf[2048], aux_buf[buffSize + 6];
  char flag_BCC2 = 0x00;
  
  //Frame
  aux_buf[0] = 0x7E;                 //F
  aux_buf[1] = 0x03;                 //A   Tx ---> Rx

  //C
  if(Ns == 0){                       //   0 = 0x00 e 1 = 0x40
    aux_buf[2] = 0x00;
    printf("send_data: Ns=0\n");
  }
  else if(Ns ==1){
    aux_buf[2] = 0x40;
    printf("send_data: Ns=1\n");
  }
  aux_buf[3] = aux_buf[1]^aux_buf[2];  //BCC1 = A^C
  
  for (int i = 0; i < buffSize; i++){
    aux_buf[4 + i] = buffer[i];
    flag_BCC2 ^= buffer[i];
  }

  aux_buf[buffSize + 4] = flag_BCC2;   //BCC2 = D^D^D^D
  aux_buf[buffSize + 5] = 0x7E; 

  new_size = byteStuffing(aux_buf, buffSize + 6, new_buf);

  res = write(fd, new_buf, new_size);
  if(res<0) printf("Error reading\n");
    
  return res;
}

int receive_DATA(int fd, char* buffer, int buffSize){
  printf("\nEntrei no receive_DATA\n");
  data_states state = data_START;
  int res = 0, i = 0, j, pos = 0, new_size;
  char character, flag_BCC2 = 0x00, new_buf[buffSize], control;      //Tamanho para os dados
  unsigned char aux_buffer[5];
  STOP = FALSE;

  if(Ns == 0){                       //C   0 = 0x00 e 1 = 0x40
    control = 0x00;
    printf("receive_data: Ns=0\n");
  }
  else if(Ns ==1){
    control = 0x40;
    printf("receive_data: Ns=1\n");
  }

  while(STOP == FALSE){
    res = read(fd, &character, 1);   // returns after 1 chars have been input 
    if(res < 0){
      printf("Error reading\n");
    } 
    aux_buffer[pos]=character;

    if(timeout==TRUE){
      return 0;
    }

    switch(state){
      case data_START:
        //printf("estou no START do receive_DATA\n");
        if(aux_buffer[pos] == FLAG_TM){
          state = data_FLAG;
          pos = 1;
        }
         else {
           state = data_START;
           pos = 0;                                                      
           }
        break;

      case data_FLAG:
        //printf("estou no FLAG do receive_data\n");
        if(aux_buffer[pos] == A_TM){
          state = data_A;
          pos = 2;
        }
        else if(aux_buffer[pos] == FLAG_TM){
          state = data_FLAG; 
          pos = 1;
        }
         else {
           state = data_START;
           pos = 0;
         }
        break;

      case data_A:
        //printf("estou no A do receive_data\n");
        if(aux_buffer[pos] == control){
          state = C;
          pos = 3;
        }
        else if(aux_buffer[pos] == FLAG_TM){
          state = data_FLAG;
          pos = 2;
        }
        else {
          state = data_START;
          pos = 0;
        }
        break;
      
      case data_C:
        //printf("estou no C do receive_data\n");
        if(aux_buffer[pos] == A_TM^control){
          state = BCC1;
          pos = 4;
        }
        else if(aux_buffer[pos] == FLAG_TM){
          state = data_FLAG;
          pos = 1;
        }
        else {
          state = data_START;
          pos = 0;
        }
        break;

      case BCC1:
        //printf("estou no BCC1 do receive_DATA\n");
        new_buf[i] = aux_buffer[pos];  //pos = 4
        i++;

        if(aux_buffer[pos] == FLAG_RCV){
          state = DATA;
          i--;
          printf("i=%d\n", i + 5); //Primeiro = 1012
        }
        break;
      
      case DATA:
        printf("estou no DATA do receive_DATA\n");
        new_size = byteDestuffing(new_buf, i-1, buffer);
        printf("new_size: %d\n", new_size);
        //Calcula BCC2
        for(j = 0; j < new_size; j++){
          flag_BCC2 ^= buffer[j];
        }

        if(new_buf[i-1] == flag_BCC2){
          state = BCC2;
        }
        else{
          REJ = TRUE;
        }
        
        break;
      
      case BCC2:
        printf("estou no BCC2 do receive_DATA\n");
        if(new_buf[i] == FLAG_TM){
          printf("Entrei aqui\n");
          state = data_S_STOP;
        }
        else {
          state = data_START;
          pos = 0;
        }
        break;

      case data_S_STOP:
        printf("estou no STOP do receive_DATA\n");
        i=0;
        STOP = TRUE;
        break;
    }
  }
  return new_size;
}

int send_ACK(int fd, char ch){
  printf("Entrei no send_ACK\n");
  int res;
  unsigned char buffer[5];  

  //Frame
  buffer[0] = 0x7E;                 //F
  buffer[1] = 0x03;                 //A   Tx ---> Rx

  //C
  if(ch == 'P'){  //Positive ACK
    //printf("Sending Positive ACK (RR) from transmiter\n");
    if(Nr == 0){                       
      buffer[2] = 0x05;
      printf("send_ack: sent RR Nr=0\n");
    }
    else if(Nr == 1){
      buffer[2] = 0x85;
      printf("send_ack: sent RR Nr=1\n");
    }
  }
  else if(ch == 'N'){ //Negative ACK
    //printf("Sending Negative ACK (REJ) from transmiter\n");
    if(Nr == 0){                       
      buffer[2] = 0x01;
      printf("send_ack: sent REJ Nr=0\n");
    }
    else if(Nr ==1){
      buffer[2] = 0x81;
      printf("send_ack: sent REJ Nr=1\n");
    }
  }

  buffer[3] = buffer[1]^buffer[2];  //BCC = A^C
  buffer[4] = 0x7E;                 //F

  res = write(fd, buffer, sizeof(buffer));
  if(res<0) printf("Error reading\n");
    
  return res;
}

int receive_ACK(int fd, char ch){
  printf("Entrei no receive_ACK\n");
  states state = START;
  int res, pos = 0;
  char character, control;
  unsigned char buffer[5];
  STOP = FALSE;

  if(ch == 'P'){                //Positive ACK (RR)
    if(Nr == 0){                //0 = 0x01 e 1 = 0x21         
      control = 0x05;
      printf("receive_ack: received RR Nr=0\n");
    }
    else if(Nr == 1){
      control = 0x85;
      printf("receive_ack: received RR Nr=1\n");
    }      
  }
  else if(ch == 'N'){           //Negative ACK (REJ)
    if(Nr == 0){                //0 = 0x03 e 1 = 0x23         
      control = 0x01;
      printf("receive_ack: received REJ Nr=0\n");
    }
    else if(Nr == 1){
      control = 0x81;
      printf("receive_ack: received REJ Nr=1\n");
    }       
  }

  while(STOP == FALSE){
    
    res = read(fd, &character, 1);   /* returns after 1 chars have been input */
    //printf("Cheguei aqui\n");
    if(res < 0){
      printf("Error reading\n");
    }
    buffer[pos] = character;

    if(timeout==TRUE){
      return 0;
    }

    switch(state){
      
      case START:
        if(buffer[pos] == FLAG_RCV){
          //printf("State machine do SET: START");
          state = FLAG;
          pos = 1;
        }
         else {
           state = START;
           pos = 0;                                                      
           }

        break;

      case FLAG:
        if(buffer[pos] == A_TM){
          state = A;
          pos = 2;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG; 
          pos = 1;
        }
         else {
           state = START;
           pos = 0;
         }
        break;

      case A:
        if(buffer[pos] == control){
          state = C;
          pos = 3;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 2;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case C:
        if(buffer[pos] == A_TM^control){
          state = BCC;
          pos = 4;
        }
        else if(buffer[pos] == FLAG_RCV){
          state = FLAG;
          pos = 1;
        }
        else {
          state = START;
          pos = 0;
        }
        break;
      
      case BCC:
        if(buffer[pos] == FLAG_RCV){
          state = S_STOP;
        }
        else {
          state = START;
          pos = 0;
        }
        break;

      case S_STOP:
        STOP = TRUE;
        ACK_RCV = TRUE;
        printf("\nLigação completa\n");
        break;
    }
  }
  printf(buffer);
  return res;
}

int byteStuffing(char* trama, int length, char* novaTrama){

  int newLength = 1;
  novaTrama[0] = trama[0];

  for (int i = 1; i < length - 1; i++){
    if (trama[i] == 0x7e){
      novaTrama[newLength++] = ESCAPE;
      novaTrama[newLength++] = 0x5e;//(0x7e ^ 0x20);
    }
    else if (trama[i] == ESCAPE) {
      novaTrama[newLength++] = ESCAPE;
      novaTrama[newLength++] = 0x5d;//(ESCAPE ^ 0x20);
    }
    else {
      novaTrama[newLength++] = trama[i];
    }
  }

  novaTrama[newLength++] = trama[length - 1];

  return newLength;
}

int byteDestuffing(char *trama, int length, char *novaTrama){

  int newLength = 0;

  for (int i = 0; i < length; i++){
    if (trama[i] == ESCAPE && trama[i + 1] == 0x5e){
      novaTrama[newLength++] = 0x7e;
      i++;
    }
    else if (trama[i] == ESCAPE && trama[i + 1] == 0x5d){
      novaTrama[newLength++] = 0x7d;
      i++;
    }
    else{
      novaTrama[newLength++] = trama[i];
    }
  }

  return newLength;
}
