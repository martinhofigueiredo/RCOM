#include "../application/linklayer.h"
#include <stdio.h>

int main(){
    printf("Hello World!\n");
    printf("llopen return: %d\n", llopen());
    printf("llwrite return: %d\n", llwrite());
    printf("llread return: %d\n", llread());
    printf("llclose return: %d\n", llclose());
}

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(){//linkLayer connectionParameters){
    return 1;
}
// Sends data in buf with size bufSize
int llwrite(){//char* buf, int bufSize){
    return 2;
}
// Receive data in packet
int llread(){//char* packet){
    return 3;
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(){//int showStatistics){
    return 4;
}