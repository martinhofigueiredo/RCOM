/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define MINTIME 5 // in tenth of seconds
#define MINBYTES 0
#define MAXTRIES 3
#define ALARMTIMEOUT 3

volatile int STOP = FALSE;
int tries = 1, flag = 1, fd, received_set = 0;
char set[5], ua[5];

void timeout_set();

int main(int argc, char **argv)
{
  int c, res;
  struct termios oldtio, newtio;
  char buf[255];
  int i, sum = 0, speed = 0;

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

  newtio.c_cc[VTIME] = MINTIME; /* inter-character timer unused */
  newtio.c_cc[VMIN] = MINBYTES; /* blocking read until 5 chars received */

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

  if (fgets(buf, 255, stdin) == NULL)
  {
    fprintf(stderr, "Could not read user input");
    exit(EXIT_FAILURE);
  }

  //printf("0x%02x\n", i);

  // Enviar SET
  char F = 0x7E;
  char A = 0x03;
  char C = 0x03;
  char BCC1 = A ^ C;

  set[0] = F;
  set[1] = A;
  set[2] = C;
  set[3] = BCC1;
  set[4] = F;

  (void)signal(SIGALRM, timeout_set);

  size_t s;
  write(fd, set, 5);

  printf("Sent set\n");
  for (int i = 0; i < 5; i++)
    printf("0x%02x\n", set[i]);

  // waits for ua and is responsible for set timeout behavior
  // Duvida: e se o ua so mandar 4 bytes
  int bytes_received;
  while (!received_set && tries <= MAXTRIES)
  {
    if (read(fd, ua, 5) == 5)
    {
      received_set = 1;
      break;
    }
    if (flag)
    {
      alarm(ALARMTIMEOUT); // activates timeout_set() in ALARMTIMEOUT seconds
      flag = 0;
    }
  }
  if (!received_set)
  {
    fprintf(stderr, "Unable to connect after %d tries\n", MAXTRIES);
    exit(EXIT_FAILURE);
  }

  printf("----------------------\n");
  printf("Received ua\n");
  for (int i = 0; i < 5; i++)
    printf("0x%02x\n", ua[i]);

  exit(EXIT_SUCCESS);

  // adding 1 to compensate '\0'
  int bsize = strlen(buf) + 1;

  res = write(fd, buf, bsize);
  printf("%d bytes written\n", res);

  // setting to empty string
  memset(buf, 0, 255);

  read(fd, buf, bsize);
  printf("Received: %s", buf);

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

  sleep(1);
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}

void timeout_set()
{
  if (received_set)
    return;
  printf("escrevendo # %d\n", tries);
  tries++;
  write(fd, set, 5);
  flag = 1;
}