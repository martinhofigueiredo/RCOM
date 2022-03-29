/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "lib.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int RECEIVED_SET, FLAG_RCV, A_RCV, C_RCV, BCC1_OK;

typedef unsigned char BYTE;
int check_set_byte(BYTE s);

volatile int STOP = FALSE;

int main(int argc, char **argv)
{
  int fd, c, res;
  struct termios oldtio, newtio;
  char buf[255];

  // if ( (argc < 2) ||
  //      ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  //       (strcmp("/dev/ttyS1", argv[1])!=0) )) {
  //   printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
  //   exit(1);
  // }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  // VER SLIDE 17 do guiao de trabalho
  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1)
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

  newtio.c_cc[VTIME] = 5; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  // Cannot exceed 255 (buffer size)
  int bsize = 0;

  /* do
  {
    ++bsize;
    res = read(fd, (buf + bsize-1), 1);
  } while (buf[bsize-1] != '\0');
  printf("%s%d\n", buf, bsize);
  write(fd, buf, bsize);
*/

  // llopen State
  BYTE set_byte;
  RECEIVED_SET = FLAG_RCV = A_RCV = C_RCV = BCC1_OK = FALSE;
  printf("||\n");
  while (!RECEIVED_SET)
  {
    read(fd, &set_byte, 1);
    check_set_byte(set_byte);
    printf("|0x%02x|\n", set_byte);
  }

  BYTE ua[5];
  //envia UA
  printf("--------------\n");
  ua[0] = F_UA;
  ua[1] = A_UA;
  ua[2] = C_UA;
  ua[3] = BCC1_UA;
  ua[4] = F_UA;

  for (int i = 0; i < 5; i++)
    printf("|0x%02x|\n", ua[i]);
  write(fd, ua, 5);



  sleep(1);
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}

int check_set_byte(BYTE s)
{
  // Switch used to simulate SET State Machine
  switch (s)
  {
  case F_SET:
    // Received end of UA
    if (BCC1_OK)
      RECEIVED_SET = TRUE;
      
    // Received normal Flag
    else
    {
      FLAG_RCV = TRUE;
      A_RCV = C_RCV = BCC1_OK = FALSE;
    }
    break;
  case A_SET:
    // C_SET case (which coincidentally has the same number as A_SET)
    if (A_RCV)
      C_RCV = TRUE;
    // Actual A_SET case
    else if (FLAG_RCV)
      A_RCV = TRUE;
    break;
  case BCC1_SET:
    if (C_RCV)
      BCC1_OK = TRUE;
    break;
  default:
    FLAG_RCV = A_RCV = C_RCV = BCC1_OK = FALSE;
    break;
  }
  return TRUE;
}

int llopen(int porta, int mode){

  return 0;
}