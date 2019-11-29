#include "receiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

char *filename;
int port, windowsize;

int main(int argc, char const *argv[])
{
    int n, base, raw_socket, writeLength;
    uint32_t mask;
    FILE *fp;
    struct sockaddr_in servaddr;
    data_pkt_t data;
    ack_pkt_t packet;

    if (argc != 4)
    {
        fprintf(stderr, "main(): too few args!");
        exit(-1);
    }
    filename = argv[1];
    port = argv[2];
    windowsize = argv[3];

    if ((raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        fprintf(stderr, "main(): socket error!");
        exit(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(raw_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("main(): bind error!");
        exit(-1);
        close(raw_socket);
    }

    base = 1;
    fp = fopen(filename, "w");

    if (fp == NULL) {
        perror("main(): fopen error!");
        exit(-1);
        close(raw_socket);
    }

    socklen_t len = sizeof(servaddr);
    while (1)
    {

        if (writeLength == sizeof(data.data))
        {
            
            if ((n = recvfrom(raw_socket, (data_pkt_t *)data, MAXSIZE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len)) < 1) {
                fprintf(stderr, "main(): recvfrom error!");
                fclose(fp);
                close(raw_socket);
                exit(-1);
            }

            fprintf(stdout, "Received: %s\n", data.data);
            if (data.seq_num > base + windowsize) {
                /*drop packet*/
                continue;
            }

            if (data.seq_num < base) {
                /*This means it was already acknowledged*/
                packet.seq_num = base;
                packet.selective_acks = mask;
            }

            if (data.seq_num == base) {
                /*shifts the base, sends ack*/
                while (mask << 31 != 0) {
                    base = base + 1;
                    mask = mask >> 1;
                }
                base = base + 1;
                mask = mask >> 1;
                packet.seq_num = base;
                packet.selective_acks = mask;
            }

            if (data.seq_num > base && data.seq_num < base + windowsize) {
                /* if it's in the middle of the window, the base stays the same, the selectiveAck is changed*/
                int s = data.seq_num - base;
                mask = mask | (1 >> s);
                packet.seq_num = data.seq_num;
                packet.selective_acks = mask;
            }

            if (sendto(raw_socket, (void *)&packet, sizeof(packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("Send to");
                close(raw_socket);
                fclose(fp);
                exit(-1);
            }
            fseek(fp, (packet.seq_num - 1) * 1000, SEEK_SET);
            writeLength = fwrite(data.data, sizeof(char), sizeof(data.data), fp);
        }
        else { /*last packet was reached*/
            fprintf(stdout, "Finished!");
            fclose(fp);
            close(raw_socket);
            break;
        }
    }
    return 0;
}