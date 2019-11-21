/* Yo Yo Yo */

#define LOCALHOST "127.0.0.1" /* Using this for ease of debugging*/
#define MAXSIZE 1000
#define TRUE 1999
#define MYPORT 6969

void createSocket(int port);

void sendPacket(char *payload);

int receivePacket();
