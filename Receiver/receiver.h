#include "packet-format.h"
#define MAXSIZE 1000

typedef struct ack {
    uint32_t seq_num;
    uint32_t selective_acks;
} ack;


void setupSockets(int port);
void sendPacket(struct data_pkt_t *payload);
int receivePacket();

