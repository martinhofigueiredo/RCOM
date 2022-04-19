/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 50
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define TRANSMITTER 0
#define RECEIVER 1
#define FLAG 0x7e
#define ADDRESS 0x03
#define SET 0x03
#define DISC 0x0b
#define UA 0x07
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81
#define TRAMA0 0x00
#define TRAMA1 0x40
#define ESCAPE 0x7d
#define BUFFER_SIZE 520
int rpad,lpad;
int val=0;
int contador_progresso=0;
double percentagem;
int fer = 0, modo;
volatile int STOP=FALSE,END=FALSE;
int fd, i=0, sequencia = -1;
struct termios oldtio,newtio;
unsigned char trama_esperada = TRAMA0;
int flag=1, conta=1;
int totalbytessent = 0;

int llclose(int port, int MODE);
void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}


void errorgenerator(unsigned char *buffer, int length) {

	int indice=0,p;
	time_t t;
	srand((unsigned) time(&t));
	p = rand() % 100 + 1;
	if(p<=fer) {
		do {

			indice = rand() % (length - 3) + 1;

		} while (buffer[indice] == 0x7d || buffer[indice] == 0x7e || buffer[indice] == 0x5e || buffer[indice] == 0x5d);
	}

	buffer[indice] = 0x00;
	return;

}

int packet_deconstructor(unsigned char *buffer, int length) {

	int i, j;
	unsigned char packet[BUFFER_SIZE];
	memset(packet,0,BUFFER_SIZE);

	length -= 4;

	for(i=0;i<length;i++) {
		packet[i] = buffer[i+4];
	}

	packet[i] = 0;
	length--;

	for(i=0;i<length;i++) {

		if(packet[i]==ESCAPE) {
			packet[i] = packet[i+1] ^ 0x20;
			length--;

			for(j=i+1;j<length;j++) {
				packet[j] = packet[j+1];
			}
		}
	}

	memcpy(buffer,packet,BUFFER_SIZE);
	return length;
}

int ler_trama(int port, unsigned char *buffer) {
(void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao
	long seconds, seconds_start;
	int res = 0;
	short STOP = 0, flags = 0, check;
	unsigned char trama[BUFFER_SIZE], read_char = 0;

	memset(trama,0,sizeof(trama));

	while(!STOP) {
		check = read(port, &read_char, 1);
		while(conta<3 && check==0){
			if(flag){
	       alarm(3);                 // activa alarme de 3s
	       flag=0;
	    } else return -404;

		}


alarm(0);
flag=1;

		if(check==1)
			conta=0;

			if(conta==3 && check==0 ){
			 exit(-1);
			}
		if(check==-1) {
			return -4;
		}
		if(check==0)
		{
			return -404;
		}


		if(read_char == FLAG) {
			if(res==0) {
				trama[res]=read_char;
				res++;
			} else {
				if(trama[res-1] == FLAG) {
					res = 0;
					memset(trama,0,sizeof(trama));
					trama[res] = read_char;
					res++;
				} else {
					trama[res] = read_char;
					res++;
					STOP = 1;
				}
			}
		} else if(res>0) {
			conta=0;
			trama[res] = read_char;
			res++;
		}
	}
	//Verificação de erros

	if(res < 5){
		return -1;
	} else if((trama[1]^trama[2])!=trama[3]){
		return -2;
	} else {
		if(res>BUFFER_SIZE) {
			return -5;
		}
		if(modo == RECEIVER)
			errorgenerator(trama,res);
		memcpy(buffer,trama,BUFFER_SIZE);
		return res;
	}
} //retorna tamanho da trama ou negativo em caso de erro

int enviar_trama(int port, unsigned char *buffer, int length) {		//Retorna tamanho da trama enviada ou negativo em caso de erro

	int res, i;

	if(buffer[0] != FLAG || buffer[length-1] != FLAG) {
		return -1;
	}

	res = write(port, buffer, length);

	if(res != length) {
		return -2;
	}
	if(res>0)
		totalbytessent += res;

	return res;
}

int resposta_receiver(int port, unsigned char c) {				//Retorna 5 (tamanho da trama) ou outro valor em caso de erro

	unsigned char trama[5];

	trama[0] = FLAG;
	trama[1] = ADDRESS;
	trama[2] = c;
	trama[3] = trama[1]^trama[2];
	trama[4] = FLAG;



	return enviar_trama(port,trama,5);
}

int llread(int port, unsigned char *buffer) {

	short state = 0, lido = 0;
	unsigned char aux, n_trama;
	int res, sizepacket, i, j;

	while(!lido){
		switch(state){
			case 0:
				res = ler_trama(port, buffer);


				if(res<0)
					if(res!=-404)
						return -1; //-1 significa que o erro veio de ler_trama

				if(res>5)		//vai para state 1 se se tratar de uma trama I
					state = 1;
				break;

			case 1:				//destuffing e verificar BCC2

				n_trama = buffer[2];

				sizepacket = packet_deconstructor(buffer, res);


				aux = 0; //aux usado para calcular bcc

				for(i = 0;i < sizepacket-1; i++) {
					aux ^= buffer[i];
				}

				if(aux != buffer[sizepacket-1])
				{
					state = 2; //REJ
				}
				else
				{
					state = 3; //RR
				}
				break;

			case 2:
				if(n_trama == TRAMA0)
				{
					resposta_receiver(port,REJ0);
				}
				else
				{
					resposta_receiver(port,REJ1);
				}

				res = ler_trama(port, buffer);


				if(res<0)
					if(res!=-404)
						return -1; //-1 significa que o erro veio de ler_trama

				if(res>5) {		//vai para state 1 se se tratar de uma trama I
					state = 1;
				}
				break;

			case 3:
				if(trama_esperada == n_trama){
					if(trama_esperada == TRAMA0){
						trama_esperada = TRAMA1;
						resposta_receiver(port,RR1);
					} else {
						trama_esperada = TRAMA0;
						resposta_receiver(port,RR0);
					}
				} else {
					if(trama_esperada == TRAMA0) {
						resposta_receiver(port,RR0);
					}
					else {
						resposta_receiver(port,RR1);
					}
					return -2;
				}

				lido = 1;
				break;

			default:
				break;
		}
	}

	return res;

}

void llopen(int MODE, unsigned char * buf) {
	int res,teste=0;
	if (MODE == RECEIVER)
	{
			res = read(fd,buf,5);   /* returns after 1 chars have been input */
	                   /* so we can printf... */


	    if (conta == 5)
	    {
	        printf("Timeout: Reiniciando...\n");
	        sleep(1);
	        conta = 0;
	    }

		if (res == 5)
		{
	


			if ((buf[0] == FLAG) &&
						(buf[1] == ADDRESS) &&
						(buf[2] == SET) &&
						(buf[3] == (buf[1]) ^ (buf[2])) &&
						(buf[4] == FLAG)) // recebido inicio e final de trama
			{

			    sleep(1);
			    memset(buf, 0, 5);

			    buf[0] = FLAG;
			    buf[1] = ADDRESS;
			    buf[2] = UA;
			    buf[3] = ((buf[1]) ^ (buf[2]));
			    buf[4] = FLAG;

			    write(fd, buf, 5); // manda a trama de 5 bytes
					STOP=TRUE;

			}	 else return -1000;
	 	}
	}
	else if (MODE == TRANSMITTER)
	{
		buf[0] = FLAG;
		buf[1] = ADDRESS;
		buf[2] = SET;
		buf[3] = ((buf[1]) ^ (buf[2]));
		buf[4] = FLAG;
		for (i = 0; i < 5; i++)
		{
			teste =	write(fd, buf+i, 1);
			if(teste>0)
				totalbytessent += teste;
		}
		memset(buf, 0, 5);

		res = 0;
		i = 0;
		flag = 0;
		while (STOP == FALSE)
		{
				res = read(fd, buf+i, 1);
				if (buf[i] == FLAG) {
					flag++;
				}
				if(flag == 2){
					STOP = TRUE;
				}
				i++;
		}

		if ((buf[0] == FLAG) &&
					(buf[1] == ADDRESS) &&
					(buf[2] == UA) &&
					(buf[3] == (buf[1]) ^ (buf[2])) &&
					(buf[4] == FLAG)) // recebido inicio e final de trama
		{
			STOP=TRUE; // acaba
		} else return -1000;
	}
}

int llclose(int port, int MODE) {

	unsigned char trama_s[BUFFER_SIZE];
	int state = 0, STOP = 0, res;

	trama_s[0] = FLAG;
	trama_s[1] = ADDRESS;
	trama_s[2] = DISC;
	trama_s[3] = trama_s[1] ^ trama_s[2];
	trama_s[4] = FLAG;

  if(MODE == RECEIVER) {
  	while(!STOP) {
  		switch(state) {
  			case 0:

  				res = enviar_trama(port, trama_s, 5);

  				if(res<0)
  					return -1;

  				state = 1;
  				break;

  			case 1:

  				res = ler_trama(port, trama_s);

  				if(res == -404)
  					break;
  				else if(res < 0)
  					return -2;

  				if(trama_s[2] == UA)
  					STOP = 1;
  				else {
  					state = 0;
  				}

  				break;

  			default:
  				break;
  		}
  	}

  	return 0;
  } else if (MODE == TRANSMITTER) {
    enviar_trama(port, trama_s, 5);
    memset (trama_s, 0, 5);

		if(END) 	{
			exit(0);
		}

    while(STOP==FALSE)
  	{
  		res=ler_trama(fd,trama_s);
  		sleep(1);

  		if((trama_s[0]==FLAG)
  		&& (trama_s[1]==ADDRESS)
  		&& (trama_s[2]==DISC)
  		&& (trama_s[3]== trama_s[1]^trama_s[2])
  		&& (trama_s[4]==FLAG)) //condicao para verificacao de DISC
  		{
  			sleep(1);
  			STOP=TRUE;
  		}

      else
      {
        return -1;
      }
  	}
    	trama_s[0] = FLAG;
    	trama_s[1] = ADDRESS;
    	trama_s[2] = UA;
    	trama_s[3] = trama_s [1]^trama_s[2];
    	trama_s[4] = FLAG;
  		res = write(fd,trama_s,5);

		if(res>0)
		totalbytessent += res;

    return 0;
  }
	if(END)
	exit(0);
  return -2;
}

int packet_header_generator(int id_packet,unsigned char *dados,int length)
{
  unsigned char bcc = 0;
  char *fname;
  int i,j, fname_s;
			switch(id_packet)
		  {

			case 1:   //vai gerar o pacote de dados do ficheiro

				if(length+5 > BUFFER_SIZE) {
					return -1;
				}

				for(i=length;i>0;i--) {
					dados[i+3] = dados[i-1];
				}

				dados[0]=1;
				dados[1]=sequencia%256;
				dados[2]=length/256;
				dados[3]=length%256;
				length = length+4;

        break;

			case 2:   //vai criar pacote de START

        fname_s = strlen(dados);
        fname = (char*) malloc(fname_s*sizeof(char));
        memcpy(fname,dados,fname_s);
        memset (dados, 0, BUFFER_SIZE);
        dados[0]=2;
        dados[1]=0; dados[2]=4;
        dados[3]=length>>24;	dados[4]=length>>16;	dados[5]=length>>8; dados[6]=length;
        dados[7]=1; dados[8]=(unsigned char) fname_s;
        for(i=0;i<fname_s;i++) {
          dados[9+i] = fname[i];
        }

        length = 9+fname_s;

      break;

			case 3:  //vai criar pacote de END

        fname_s = strlen(dados);
        fname = (char*) malloc(fname_s*sizeof(char));
        memcpy(fname,dados,fname_s);
				memset (dados, 0, BUFFER_SIZE);
				dados[0]=3;
				dados[1]=0; dados[2]=4;
				dados[3]=length>>24;	dados[4]=length>>16;	dados[5]=length>>8; dados[6]=length;
				dados[7]=1; dados[8]=(unsigned char) fname_s;
        for(i=0;i<fname_s;i++) {
          dados[9+i] = fname[i];
        }

        length = 9+fname_s;

        break;
      }

      for (i = 0; i < length; i++)
      {
        bcc ^= dados[i];
      }

      dados[length] = bcc;
      length++;

      for(i=0;i<length;i++)
		  {
			     if(dados[i]==ESCAPE || dados [i] == FLAG)				//byte stuffing
			     {
				     for(j=length;j>i;j--)
				     {
		           dados[j]=dados[j-1];
			       }
		         length++;
		         i++;
             dados[i]=dados[i-1]^0x20;
	           dados[i-1]=ESCAPE;
			     }
		  }


      return length;

}

int frame_header_generator(unsigned char *buffer, int length, unsigned char trama_esperada) { //retorna tamanho total da trama ou negativo em erro

	int i;

	if(length>BUFFER_SIZE-5) {
		return -1;
	}

	for(i=length;i>0;i--)
		buffer[i+3]=buffer[i-1];	//move todos os valores 4 posições para ter espaço para o cabeçalho da trama

	buffer[0] = FLAG;
	buffer[1] = ADDRESS;
	buffer[2] = trama_esperada;
	buffer[3] = buffer[1]^buffer[2];
	buffer[length+4] = FLAG;

	return length+5;
}

int llwrite(int port, unsigned char *buffer, int length) {

	short STOP = 0, state = 0;
	int i, res;
	unsigned char resposta[BUFFER_SIZE];

	length = frame_header_generator(buffer, length, trama_esperada);
	if(length == -1)
		return -1; //-1 significa que o erro veio de frame_header_generator

	while(!STOP) {
		switch(state) {
			case 0:

				res = enviar_trama(port, buffer, length);

				if(res<0)
					return -2; //-2 significa que o erro veio de enviar_trama

				state = 1;
				break;


			case 1:	//ler a resposta

				i = ler_trama(port, resposta);

				if(i<0)
          if(i!=-404) {

					  return -3; //-3 significa que o erro veio de ler_trama
            }
				if (i == 5)
					state=2;
        if(i==-404)
          state=0;
			break;

			case 2:
				if((trama_esperada == TRAMA0 && resposta[2] == RR1)
				|| (trama_esperada == TRAMA1 && resposta[2] == RR0)) {

					trama_esperada = trama_esperada ^ 0x40;
					STOP = 1;

				} else if ((trama_esperada == TRAMA0 && resposta[2] == REJ0)
				        || (trama_esperada == TRAMA1 && resposta[2] == REJ1)) {

					state = 0;
				} else if (resposta[2] == DISC) {
					sleep(2);
					exit(1);
					return -7;
				} else {
					state = 0;
				}


				break;

			default:
				break;
		}
	}

	return res;

}

int receiver() {


	if ( tcgetattr(fd,&oldtio) == -1)
	{
	    /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 1;/* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0;/* blocking read until 5 chars received */

	/*
	   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	   leitura do(s) prximo(s) caracter(es)
	 */

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1)
	{
    	perror("tcsetattr");
    	exit(-1);
	}

	//(void) signal(SIGALRM, atende); // instala  rotina que atende interrupcao


  FILE *gifp;
  short state = 0, size_s = 0, name_s = 0;
  unsigned char buf[BUFFER_SIZE];
  char *fname, *fname_compare;
  int filesize, datasize, ultima_seq = 0;

  while (STOP == FALSE)
	{
		llopen(RECEIVER,buf);
	}

	STOP = FALSE;

	while(STOP == FALSE) {
		switch(state) {
			case 0:	//espera pela trama start

				if(llread(fd,buf)==-2)
					break;

				if(buf[0]==2) {
					state = 1;
				}

				break;

			case 1:
				size_s = buf[2]; //tamanho da informacao do tamanho do ficheiro

				for(i=0;i<size_s;i++) {
					filesize += (int) buf[i+3] << (8*(size_s-(i+1)));
				}

				name_s = buf[3+size_s+1]; //tamanho da informacao do nome do ficheiro
				fname = (char*) malloc(name_s+1); //espaco para \0

				for(i=0;i<name_s;i++) {
					fname[i] = buf[3+size_s+2+i];
				}
				fname[name_s] = '\0';
				gifp = fopen(fname,"wb");

				state = 2;
				break;

			case 2:	//ler dados ate receber end

				if(llread(fd,buf)==-2) {
					break;
				}

				if(buf[0] == 3) {
					state = 3;
					break;
				}

				if(buf[1] == ultima_seq%256)
					ultima_seq++;

				datasize = (int) buf[2]*256 + (int) buf[3];

				for(i=0;i<datasize;i++) {
					fputc(buf[i+4], gifp);
				}

				break;

			case 3:

				size_s = buf[2]; //tamanho da informacao do tamanho do ficheiro

				for(i=0;i<size_s;i++) {
					filesize -= (int) buf[i+3] << (8*(size_s-(i+1)));
				}

				name_s = buf[3+size_s+1]; //tamanho da informacao do nome do ficheiro
				fname_compare = (char*) malloc(name_s+1);  //espaco para \0

				for(i=0;i<name_s;i++) {
					fname_compare[i] = buf[3+size_s+2+i];
				}
				fname_compare[name_s] = '\0';


				fclose(gifp);
				free(fname);
				free(fname_compare);

				while(buf[2] != DISC)
					ler_trama(fd,buf);
				STOP = TRUE;
				break;
		}
	}
	llclose(fd,RECEIVER);
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
	return 0;
}

int transmitter(char* fname) {

	if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
									perror("tcgetattr");
									exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,.7e35622040029271b70696e6775696d2e676966000007e..) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 1;/* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0;/* blocking read until 5 chars received */



	/*
	   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	   leitura do(s) prï¿½ximo(s) caracter(es)
	 */



	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
									perror("tcsetattr");
									exit(-1);
	}


  unsigned char character_of_file, replyc;
  unsigned char dados[BUFFER_SIZE];
  FILE  *img;
  int length_file=0, packetsize;
  char inicio=0;
  char ciclo=0;
  char id_pacote=0;
  int res = 0;
  int contador_byteswritten=0;
  int i;

  llopen(TRANSMITTER, dados);
  memset(dados,0,BUFFER_SIZE);

  //||||||||||||||||||||||||||||||||||||||||||||||||||||| ABRIR O DOCUMENTO PINGUIM.GIF||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  img=fopen(fname,"rb");
  character_of_file=0;
  fseek(img,0L,SEEK_END);
  length_file=ftell(img);
  //|||||||||||||||||||||||||||||||||||||||||||||||||||||ENVIO DE TRAMA START||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  memcpy(dados,fname,strlen(fname)+1);
  packetsize = packet_header_generator(2,dados,length_file); //cria pacote de dados START,END ou SUPERVISION
  llwrite(fd,dados,packetsize);

  fseek(img,0L,SEEK_SET);
  //comecar a comunicar de forma a enviar info
  while(!feof(img) && !END)
  {
      if(contador_byteswritten<256)
    {
			character_of_file=fgetc(img);
      dados[contador_byteswritten]=character_of_file;
      contador_byteswritten++;
    }

    if(contador_byteswritten==256)
    {

      sequencia++;
      packetsize = packet_header_generator(1,dados,contador_byteswritten);
      llwrite(fd,dados,packetsize);
			contador_progresso+=256;
			percentagem=((double)contador_progresso/(double)length_file);
			val = (int) (percentagem*100)+1;
			lpad = (int) (percentagem * PBWIDTH);
			rpad = PBWIDTH - lpad;
    	printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
			fflush (stdout);
			if(END)
				break;

      contador_byteswritten=0;

    }
  }
  sequencia++;
  contador_byteswritten--; // ignora-se o byte a mais // nao e uma mudanca definitiva
  packetsize = packet_header_generator(1,dados,contador_byteswritten);
	if(!END)
  llwrite(fd,dados,packetsize);
  memcpy(dados,fname,strlen(fname)+1);
  packetsize = packet_header_generator(3,dados,length_file);
	if(!END)
  llwrite(fd,dados,packetsize);
  fclose(img);
	if(!END)
  res = llclose(fd,TRANSMITTER);
	else
	llclose(fd,RECEIVER);
  if (res == 1)
  {
      printf("\nConexão encerrada...\n");
      sleep(3);
  }


  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }
  close(fd);
  return 0;

}
/*
int main(int argc, char** argv) {

		struct timeval starttime, endtime;
		int res;

	if ( (argc < 3) ||
	((strcmp("/dev/ttyS0", argv[1])!=0) &&
	(strcmp("/dev/ttyS1", argv[1])!=0) ) ||
  ((strcmp("TX", argv[2])!=0) || argc < 4 )&&
  (strcmp("RX", argv[2])!=0) )
	{
		printf("Usage:\tnserial SerialPort TX|RX FileName\n\tex: nserial /dev/ttyS1 TX pinguim.gif\n");
		exit(1);
	}

    fd = open(argv[1], O_RDWR | O_NOCTTY );
  	if (fd <0) {perror(argv[1]); exit(-1); }



	/*
	   Open serial port device for reading and writing and not as controlling tty
	   because we don't want to get killed if linenoise sends CTRL-C.
	 */



	printf("Start of FILESENDER..\n\n\n\n\n\n\n\n\n\n\n\n");

  if(!strcmp("RX",argv[2]))
	{
		printf("Escolha a probabilidade de erro: ");
		scanf("%d",&fer);
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
		modo = RECEIVER;

    return receiver();
	}
  else if(!strcmp("TX",argv[2])) {
		modo = TRANSMITTER;
		gettimeofday(&starttime,NULL);
	  res = transmitter(argv[3]);
		gettimeofday(&endtime,NULL);
		printf("\nDemorou %f segundos.\n",(double)(endtime.tv_usec - starttime.tv_usec) / 1000000 + (double)(endtime.tv_sec - starttime.tv_sec));
		printf("\nO transmissor enviou %d bytes.\n",totalbytessent);
		return res;
	}

  return -7;
}*/
