#include "packet-format.h"
#define MAXSIZE 1000

void setupSockets(int port);
void sendPacket(struct data_pkt_t *payload);
int receivePacket();

