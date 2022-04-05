/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "lib.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define MINTIME 5 // in tenth of seconds
#define MINBYTES 0

int RECEIVED_UA, FLAG_RCV, C_RCV, A_RCV, BCC1_OK;

typedef unsigned char BYTE;
LinkLayer link_layer;
AppLayer app_layer;
// Used to safe old port settings to restore when disconnecting
struct termios oldtio, newtio;

volatile int STOP = FALSE;
// tries and flag are part of the timeout_set function used for the signal
int tries = 1, flag = 1;
BYTE set[5];

void timeout_set();
int check_ua_byte(BYTE s);

int main(int argc, char **argv)
{
  link_layer.baudRate = B38400;
  strncpy(link_layer.port, argv[1], 20);
  link_layer.timeout = 3;
  link_layer.numTransmissions = 3;

  app_layer.file_descriptor = open(link_layer.port, O_RDWR | O_NOCTTY);
  app_layer.status = TRANSMITTER;

  BYTE buf[255];

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  if (app_layer.file_descriptor < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(app_layer.file_descriptor, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = MINTIME; /* inter-character timer unused */
  newtio.c_cc[VMIN] = MINBYTES; /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(app_layer.file_descriptor, TCIOFLUSH);

  if (tcsetattr(app_layer.file_descriptor, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  if (fgets(buf, 255, stdin) == NULL)
  {
    fprintf(stderr, "Could not read user input");
    exit(EXIT_FAILURE);
  }

  //printf("0x%02x\n", i);

  // llopen STATE
  RECEIVED_UA = FLAG_RCV = A_RCV = C_RCV = BCC1_OK = FALSE;

  // Enviar SET
  set[0] = F_SET;
  set[1] = A_SET;
  set[2] = C_SET;
  set[3] = BCC1_SET;
  set[4] = F_SET;

  (void)signal(SIGALRM, timeout_set);


  write(app_layer.file_descriptor, set, 5);
  printf("Sent set\n");
  for (int i = 0; i < 5; i++)
    printf("0x%02x\n", set[i]);

  // no retransmission required exit llopen
  if (link_layer.timeout == 0 || link_layer.numTransmissions == 0)
    exit(EXIT_SUCCESS);

  BYTE ua;
  printf("Received ua\n");
  while (!RECEIVED_UA)
  {
    if (flag)
    {
      alarm(link_layer.timeout); // signal function
      flag = 0;
    }
    // if nothing is read it is not required to enter the if statement
    if (read(app_layer.file_descriptor, &ua, 1) > 0)
    {
      check_ua_byte(ua);
      printf("|0x%02x|\n", ua);
    }
  }

  exit(EXIT_SUCCESS);

  // adding 1 to compensate '\0'
  int bsize = strlen(buf) + 1;

  int res;
  res = write(app_layer.file_descriptor, buf, bsize);
  printf("%d bytes written\n", res);

  // setting to empty string
  memset(buf, 0, 255);

  read(app_layer.file_descriptor, buf, bsize);
  printf("Received: %s", buf);

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

  sleep(1);
  if (tcsetattr(app_layer.file_descriptor, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(app_layer.file_descriptor);
  return 0;
}

void timeout_set()
{
  if (RECEIVED_UA)
    return;
  if (tries > link_layer.numTransmissions)
  {
    fprintf(stderr, "Unable to connect after %d tries\n", link_layer.numTransmissions);
    exit(EXIT_FAILURE);
  }
  printf("escrevendo # %d\n", tries);
  tries++;
  write(app_layer.file_descriptor, set, 5);
  flag = 1;
}

int check_ua_byte(BYTE s)
{
  switch (s)
  {
  case F_UA:
    // Read 2nd Flag (End of UA)
    if (BCC1_OK)
      RECEIVED_UA = TRUE;
    // Received normal Flag
    else
    {
      FLAG_RCV = TRUE;
      A_RCV = C_RCV = BCC1_OK = FALSE;
    }
    break;
  case A_UA:
    if (FLAG_RCV)
      A_RCV = TRUE;
    break;
  case C_UA:
    if (A_RCV)
      C_RCV = TRUE;
    break;
  case BCC1_UA:
    if (C_RCV)
      BCC1_OK = TRUE;
    break;
  default:
    // Restarting SET verification
    FLAG_RCV = A_RCV = C_RCV = BCC1_OK = FALSE;
    break;
  }
  return TRUE;
}

