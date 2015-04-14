#define _GNU_SOURCE

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"

typedef struct spread_s
{
    float spread;
    float leg_b;
    float leg_s;
} __attribute__ ((__packed__)) spread_t;

static int create_cpu_tcp_socket(struct in_addr *, int);
void exchangeFrames(int); 

int main(int argc, char *argv[]) 
{
    if(argc < 4) 
    {
	printf("Usage: $0 dfe_ip cpu_ip netmask\n");
	return 1;
    }

    struct in_addr dfe_ip, cpu_ip, netmask;
    
    inet_aton(argv[1], &dfe_ip);
    inet_aton(argv[2], &cpu_ip);
    inet_aton(argv[3], &netmask);
    const int tcp_port = 5008;
    
    max_file_t *maxfile = FieldSwap_init();
    max_engine_t *engine = max_load(maxfile, "*");
    
    max_ip_config(engine, MAX_NET_CONNECTION_CH2_SFP1, &dfe_ip, &netmask);

    max_tcp_socket_t *dfe_tcp_socket = max_tcp_create_socket_with_number(engine, "tcp_ch2_sfp1", 0);
    max_tcp_listen(dfe_tcp_socket, tcp_port);
    max_tcp_await_state(dfe_tcp_socket, MAX_TCP_STATE_LISTEN, NULL);
    int cpu_tcp_socket = create_cpu_tcp_socket(&dfe_ip, tcp_port);
    
    exchangeFrames(cpu_tcp_socket);

    max_tcp_close(dfe_tcp_socket);
    max_unload(engine);
    max_file_free(maxfile);
    
    printf("Done.\n");
    return 0;
}

static int create_cpu_tcp_socket(struct in_addr *remote_ip, int port) 
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int state = 1;

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &state, sizeof(state));

    struct sockaddr_in cpu;
    memset(&cpu, 0, sizeof(cpu));
    cpu.sin_family = AF_INET;
    cpu.sin_port = htons(port);
    cpu.sin_addr = *remote_ip;

    connect(sock, (const struct sockaddr*) &cpu, sizeof(cpu));

    return sock;
}

void exchangeFrames(int sock) 
{
    struct spread_s source_frame;
    // struct spread_s received_frame;

    source_frame.spread = 200.1;
    source_frame.leg_b = 300;
    source_frame.leg_s = 100;
   
    send(sock, &source_frame, sizeof(spread_t), 0);
}
