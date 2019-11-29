#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdint.h>

#define TIMEOUT 1000 // ms
#define MAX_RETRIES 3
#define MAXSIZE 1000
#define MYPORT 6969
#define WINDOW_MAX 32
#define FALSE 0
#define TRUE 1

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

int socket_sender;
struct sockaddr_in receiverAddress;
struct sockaddr_in senderAddress;
char *receiverName;
int port, windowSize;
struct hostent *host;
void setupSockets(int port);
void sendPacket(struct data_pkt_t *payload, int size);
int receivePacket(struct ack_pkt_t *ack);

int main(int argc, char **argv, char **envp)
{
    struct data_pkt_t packetBuffer;
    int seq = 1;
    struct ack_pkt_t ackaux, lastackpacket;
    FILE *file = NULL;
    int sizefile;
    int verify;
    int n_try, sendRead=0;

    if (argc != 5)
    {
        fprintf(stderr, "main(): Invalid  args!\n");
        exit(EXIT_FAILURE);
    }

    memset(&lastackpacket, 0, sizeof(lastackpacket));
    memset(&ackaux, 0, sizeof(ackaux));
    lastackpacket.seq_num = 1;

    port = atoi(argv[3]);
    windowSize = atoi(argv[4]);

    if (port < 0)
    {
        fprintf(stderr, "main(): invalid port!");
        exit(EXIT_FAILURE);
    }

    if (windowSize <= 0)
    {
        fprintf(stderr, "main(): invalid windowSize");
        exit(EXIT_FAILURE);
    }

    host = gethostbyname(argv[2]);
    if (host == NULL)
    {
        fprintf(stderr, "[!] setupSockets(): gethostbyname() failed\n");
        exit(EXIT_FAILURE);
    }
    setupSockets(port);

    /*Get size file*/
    file = fopen(argv[1], "r");
    if (file == NULL)
    {
        fprintf(stderr, "error open file\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    sizefile = ftell(file);
    rewind(file);
    int n_chunks = (sizefile / MAXSIZE) + 1;

    while (TRUE)
    {
        seq = lastackpacket.seq_num;
        while ((seq < lastackpacket.seq_num + windowSize) && (seq <= n_chunks))
        {
            if ((seq != lastackpacket.seq_num)&&((1 << (seq - lastackpacket.seq_num - 1) & (lastackpacket.selective_acks))))
            {
                printf("oi gata\n");
                seq++;
                continue;
            }
            packetBuffer.seq_num = htonl(seq);
            memset(&packetBuffer.data, 0, sizeof(packetBuffer.data));

            printf("seq no %d\n", seq);

            fseek(file, (seq-1) * MAXSIZE, SEEK_SET);
            if ((sendRead = fread(packetBuffer.data, 1, MAXSIZE, file)) < 0){
                fclose(file);
                close(socket_sender);
                fprintf(stderr, "main(): fread\n");
                exit(EXIT_FAILURE);
            }
            sendPacket(&packetBuffer, sizeof(packetBuffer.seq_num) + sendRead);
            seq++;

        }

        n_try = 0;
        while (lastackpacket.seq_num < seq)
        {
            verify = receivePacket(&ackaux);
             if (verify < 0)
            {
                n_try++;
                if (n_try < 3)
                {
		            sendPacket(&packetBuffer, sizeof(packetBuffer.seq_num) + sendRead);
                    continue;
                }
                else
                {
                    close(socket_sender);
                    fprintf(stderr, "n_try exceeded ***\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                ackaux.selective_acks = htonl(ackaux.selective_acks);
                ackaux.seq_num = htonl(ackaux.seq_num);
                if ((lastackpacket.seq_num < ackaux.seq_num) || (ackaux.selective_acks &= ~(lastackpacket.selective_acks)))
                {
                    lastackpacket = ackaux;
                }
            }

        }
        if ((sizefile < (seq - 1) * MAXSIZE)&&(lastackpacket.seq_num==seq))
        {
            break;
        }
    }

    close(socket_sender);
    fclose(file);
    exit(EXIT_SUCCESS);

    return 0;
}

void setupSockets(int port)
{int enable = 1;

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if ((socket_sender = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "setupSockets(): socket() failed! *** socket_out_fd ***\n");
        exit(EXIT_FAILURE);
    }

    setsockopt(socket_sender, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(struct timeval));

	if (setsockopt(socket_sender, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");

    memset(&senderAddress, 0, sizeof(senderAddress));
    memset(&receiverAddress, 0, sizeof(receiverAddress));

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(port);
    bcopy((char *)host->h_addr, (char *)&receiverAddress.sin_addr.s_addr, host->h_length);
}

void sendPacket(struct data_pkt_t *payload, int size)
{
    int verify;
    verify = sendto(socket_sender, (const struct data_pkt_t *)payload, size, MSG_CONFIRM, (const struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
    if (verify < 0)
    {
        fprintf(stderr, "sendPacket(): sendto() failed!\n");
        exit(EXIT_FAILURE);
    }
}

int receivePacket(struct ack_pkt_t *ack)
{
    int verify;
    socklen_t sendsize = sizeof(senderAddress);
    verify = recvfrom(socket_sender, (void *)ack, sizeof(ack), 0, (struct sockaddr *)&senderAddress, &sendsize);

    return verify;
}
