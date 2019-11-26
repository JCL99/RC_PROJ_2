/* Yo Yo Yo */

#define LOCALHOST "127.0.0.1" /* Using this for ease of debugging*/
#define MAXSIZE 1000
#define TRUE 1999
#define MYPORT 6969

void setupSockets(int port, struct sockaddr_in *senderAddress, struct sockaddr_in *receiverAddress);

void sendPacket(int socket_out_fd, char *payload);

int receivePacket(int socket_in_fd);
