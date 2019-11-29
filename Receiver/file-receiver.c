#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdint.h>

#define MAX_WINDOW_SIZE 32
#define TIMEOUT 1000 // ms
#define MAX_RETRIES 3
#define MAXSIZE 1000

typedef struct __attribute__((__packed__)) data_pkt_t
{
    uint32_t seq_num;
    char data[1000];
} data_pkt_t;

typedef struct __attribute__((__packed__)) ack_pkt_t
{
    uint32_t seq_num;
    uint32_t selective_acks;
} ack_pkt_t;


int port, windowsize;

int main(int argc, char const *argv[])
{
    int readLength, raw_socket, writeLength = 1000;
    uint32_t mask = 0;
    uint32_t base = 1;
    FILE *fp;
    struct sockaddr_in servaddr;
    data_pkt_t data;
    ack_pkt_t packet;

    if (argc != 4)
    {
        fprintf(stderr, "main(): too few args!");
        exit(-1);
    }
    port = atoi(argv[2]);
    windowsize = atoi(argv[3]);

    if ((raw_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "main(): socket error!");
        exit(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(raw_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("main(): bind error!");
        exit(-1);
        close(raw_socket);
    }

    base = 1;
    fp = fopen(argv[1], "w");

    if (fp == NULL)
    {
        perror("main(): fopen error!");
        exit(-1);
        close(raw_socket);
    }

    socklen_t len = sizeof(servaddr);
    while (1)
    {

        if ((readLength = recvfrom(raw_socket, &data, sizeof(data), 0, (struct sockaddr *)&servaddr, &len)) < 1)
        {
            fprintf(stderr, "main(): recvfrom error!");
            fclose(fp);
            close(raw_socket);
            exit(-1);
        }
        fprintf(stdout, " read length %d\n", readLength );
        data.seq_num = htonl(data.seq_num);
        fprintf(stdout, "Received: %d\n", data.seq_num);
        if (data.seq_num > base + windowsize)
        {
            /*drop packet*/
            continue;
        }
        if (data.seq_num < base)
        {
            /*This means it was already acknowledged*/
            packet.seq_num = ntohl(base);
            packet.selective_acks = ntohl(mask);
        }
        else if (data.seq_num == base)
        {
            /*shifts the base, sends ack*/
            while (mask << 31 != 0)
            {
                base = base + 1;
                mask = mask >> 1;
                printf("window scroll %d", base);
            }
            base = base + 1;
            mask = mask >> 1;
            packet.seq_num = ntohl(base);
            packet.selective_acks = ntohl(mask);
        }

        else if (data.seq_num > base && data.seq_num < base + windowsize)
        {
            /* if it's in the middle of the window, the base stays the same, the selectiveAck is changed*/
            int s = data.seq_num - base;
            mask = mask | (1 >> s);
            packet.seq_num = ntohl(data.seq_num);
            packet.selective_acks = ntohl(mask);
        }
        if (sendto(raw_socket, (void *)&packet, sizeof(packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("Send to");
            close(raw_socket);
            fclose(fp);
            exit(-1);
        }

        fseek(fp, (data.seq_num - 1) * 1000, SEEK_SET);
        writeLength = fwrite(data.data, 1, readLength - sizeof(data.seq_num), fp);
        fprintf(stdout, "%d\n", writeLength);
        if (writeLength < 1000)
            { /*last packet was reached*/
                fprintf(stdout, "finished");
                fclose(fp);
                close(raw_socket);
                break;
            }
    }
    return 0;
}