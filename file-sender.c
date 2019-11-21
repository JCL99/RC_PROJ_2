#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define LOCALHOST "127.0.0.1"
#define MAXSIZE 1000
#define TRUE 1999
#define MYPORT 6969

int socket_fd;
struct sockaddr_in receiverAddress;

void createSocket(int port);
void sendPacket(char *payload);
int receivePacket();

int main(int argc, char **argv, char **envp){
  char *filename, *receiverName; int port, windowSize;

  if(argc < 4){
    fprintf(stderr, "main(): too few args!");
		exit(EXIT_FAILURE);
  }
  filename = argv[1];
  receiverName = argv[2];
  port = atoi(argv[3]);
  windowSize = atoi(argv[4]);

  createSocket(MYPORT);

  while(TRUE){
    //sendPacket(LOCALHOST);
    receivePacket();
    break;/**/
  }

	close(socket_fd);
	return 0;
}

void createSocket(int port){
  if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "createSocket(): socket() failed!");
		exit(EXIT_FAILURE);
  }

  memset(&receiverAddress, 0, sizeof(receiverAddress));

  receiverAddress.sin_family = AF_INET;
	receiverAddress.sin_port = htons(port);
	receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socket_fd, (const struct sockaddr *)&receiverAddress, sizeof(receiverAddress)) < 0){
    fprintf(stderr, "createSocket(): bind() failed!");
		exit(EXIT_FAILURE);
  }
}

void sendPacket(char *payload){
  sendto(socket_fd, (const char *)payload, strlen(payload), MSG_CONFIRM, (const struct sockaddr *) &receiverAddress, sizeof(receiverAddress));

}

int receivePacket(){
  int n, len;
  char buffer[MAXSIZE];

	n = recvfrom(socket_fd, (char *)buffer, MAXSIZE, MSG_WAITALL, (struct sockaddr *) &receiverAddress, &len);
	buffer[n] = '\0';

  fprintf(stderr, "Received: %s\n", buffer);

  return n;
}
