#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "packet-format.h"
/* #include "common.h"*/ /* I'll implement this later... */

#define MAXSIZE 1000
#define TRUE 1999
#define MYPORT 6969

int socket_in_fd, socket_out_fd;
struct sockaddr_in receiverAddress;
struct sockaddr_in senderAddress;

char *filename, *receiverName; int port, windowSize;

void setupSockets(int port);
void sendPacket(struct data_pkt_t *payload);
int receivePacket();

int main(int argc, char **argv, char **envp){
  struct data_pkt_t packetBuffer; int bytesRead = 0, seq = 1;
  FILE *file = NULL;

  if(argc < 4){
    fprintf(stderr, "main(): too few args!");
		exit(EXIT_FAILURE);
  }

  filename = argv[1]; receiverName = argv[2];
  port = atoi(argv[3]); windowSize = atoi(argv[4]);

  setupSockets(port);

  /*Open and segment file into 1000 Bytes chunks*/
  file = fopen(filename, "rb");

  if (file != NULL){
    for(seq = 1; (bytesRead = fread(packetBuffer.data, 1, sizeof(char)*1000, file)) > 0; seq++){
      printf("I read %d bytes\n", bytesRead);
      packetBuffer.seq_num = seq;
      seq++;

      sendPacket(&packetBuffer);
      receivePacket();
    }
    /*while ((bytesRead = fread(packetBuffer.data, 1, sizeof(char)*1000, file)) > 0){

    }*/
  }
  /*---*/

  /*Send those packets and wait for acks*/
  /*while(TRUE){
    sendPacket("Send stuff!!!!!!!\n");
    receivePacket();
  }*/
  /*---*/

	close(socket_in_fd);
  close(socket_out_fd);
	return 0;
}

void setupSockets(int port){
  struct hostent *host;

  if((socket_in_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "setupSockets(): socket() failed! *** socket_out_fd ***\n");
		exit(EXIT_FAILURE);
  }
  if((socket_out_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "setupSockets(): socket() failed! *** socket_out_fd ***\n");
		exit(EXIT_FAILURE);
  }

  memset(&senderAddress, 0, sizeof(senderAddress));
  memset(&receiverAddress, 0, sizeof(receiverAddress));

  host = gethostbyname(receiverName);
	if(host == NULL) {
    fprintf(stderr, "[!] setupSockets(): gethostbyname() failed\n");
	}

  receiverAddress.sin_family = AF_INET;
	receiverAddress.sin_port = htons(port);
	bcopy((char *)host->h_addr, (char *)&receiverAddress.sin_addr.s_addr, host->h_length);

  senderAddress.sin_family = AF_INET;
	senderAddress.sin_port = htons(MYPORT);
	senderAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socket_in_fd, (const struct sockaddr *)&senderAddress, sizeof(senderAddress)) < 0){
    fprintf(stderr, "setupSockets(): bind() failed! *** socket_in_fd ***\n");
		exit(EXIT_FAILURE);
  }
}

void sendPacket(struct data_pkt_t *payload){
  sendto(socket_out_fd, (const struct data_pkt_t *)payload, 1004, MSG_CONFIRM, (const struct sockaddr *) &receiverAddress, sizeof(receiverAddress));
}

int receivePacket(){
  char buffer[MAXSIZE];
  int n, len;

  memset(buffer, 0, sizeof(buffer));

	n = recvfrom(socket_in_fd, (char *)buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *) &receiverAddress, &len);
	buffer[n] = '\0';

  fprintf(stderr, "Received: %s\n", buffer);

  return n;
}
