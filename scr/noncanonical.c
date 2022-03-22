/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int FLAG_RCV = FALSE;
int A_RCV = FALSE;
int C_RCV = FALSE;
int BCC_OK = FALSE;

#define F = 0x7E;
#define A 0x03;
#define C = 0x03;
#define BCC1 = 0x03 ^ 0x03;

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

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
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

  //le SET
  char ua[5];
  printf("||\n");
  res = read(fd, ua, 1);

  for (int i = 0; i < 5; i++)
    printf("|0x%02x|\n", ua[i]);

  //envia UA
  printf("--------------\n");
  ua[1] = 0x01;
  ua[2] = 0x07;
  ua[3] = ua[1] ^ ua[2];
  for (int i = 0; i < 5; i++)
    printf("|0x%02x|\n", ua[i]);

  write(fd, ua, 5);

  sleep(1);
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}

int check_flag_byte(char s)
{
  switch (s)
  {
  case 'a':
    /* code */
    break;

  default:
    break;
  }
  return 0;
}