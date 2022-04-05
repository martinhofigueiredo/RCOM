// SET
#define F_SET 0x7E
#define A_SET 0x03
#define C_SET 0x03
#define BCC1_SET (A_SET ^ C_SET)

// UA
#define F_UA 0x7E
#define A_UA 0x03
#define C_UA 0x07
#define BCC1_UA (A_UA ^ C_UA)

#define TRANSMITTER 1;
#define RECEIVER 0;

#define MAX_SIZE 20

typedef struct AppLayer
{
    int file_descriptor; // Descriptor that matches intended port
    int status;          // TRANSMITTER | RECEIVER
} AppLayer;

typedef struct LinkLayer
{
    char port[20];                 /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate;                  /*Velocidade de transmissão*/
    unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
    unsigned int timeout;          /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    char frame[MAX_SIZE];          /*Trama*/
} LinkLayer;

