#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
/* #include "common.h" */ /* We need to use this to lower code duplication!*/

#define LOCALHOST "127.0.0.1" /* Using this for ease of debugging*/
#define MAXSIZE 1000
#define TRUE 1999
#define MYPORT 1234

int socket_in_fd, socket_out_fd;
struct sockaddr_in receiverAddress;
struct sockaddr_in senderAddress;

char *filename, *receiverName; int port, windowSize;

void createSocket(int port);
void sendPacket(char *payload);
int receivePacket();

int main(int argc, char **argv, char **envp){
  if(argc < 4){
    fprintf(stderr, "main(): too few args!");
		exit(EXIT_FAILURE);
  }

  filename = argv[1]; receiverName = argv[2];
  port = atoi(argv[3]); windowSize = atoi(argv[4]);

  createSocket(port);

  while(TRUE){
    sendPacket("Send stuff!!!!!!!\n");
    receivePacket();
  }

	close(socket_in_fd);
  close(socket_out_fd);
	return 0;
}

void createSocket(int port){
  struct hostent *host;

  if((socket_in_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "createSocket(): socket() failed! *** socket_out_fd ***\n");
		exit(EXIT_FAILURE);
  }
  if((socket_out_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "createSocket(): socket() failed! *** socket_out_fd ***\n");
		exit(EXIT_FAILURE);
  }

  memset(&senderAddress, 0, sizeof(senderAddress));
  memset(&receiverAddress, 0, sizeof(receiverAddress));

  host = gethostbyname(receiverName);
	if(host == NULL) {
    fprintf(stderr, "[!] createSocket(): gethostbyname() failed\n");
	}

  receiverAddress.sin_family = AF_INET;
	receiverAddress.sin_port = htons(port);
	bcopy((char *)host->h_addr, (char *)&receiverAddress.sin_addr.s_addr, host->h_length);

  senderAddress.sin_family = AF_INET;
	senderAddress.sin_port = htons(MYPORT);
	senderAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socket_in_fd, (const struct sockaddr *)&receiverAddress, sizeof(receiverAddress)) < 0){
    fprintf(stderr, "createSocket(): bind() failed! *** socket_in_fd ***\n");
		exit(EXIT_FAILURE);
  }
}

void sendPacket(char *payload){
  sendto(socket_out_fd, (const char *)payload, strlen(payload), MSG_CONFIRM, (const struct sockaddr *) &receiverAddress, sizeof(receiverAddress));
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
