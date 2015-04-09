#define _GNU_SOURCE

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"


typedef struct payload_s {
	uint32_t field_a;
	uint32_t field_b;
} __attribute__ ((__packed__)) payload_t;


static int create_cpu_udp_socket(struct in_addr *local_ip, struct in_addr *remote_ip, int port) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in cpu;
	memset(&cpu, 0, sizeof(cpu));
	cpu.sin_family = AF_INET;
	cpu.sin_port = htons(port);

	cpu.sin_addr = *local_ip;
	bind(sock, (struct sockaddr *)&cpu, sizeof(cpu));

	cpu.sin_addr = *remote_ip;
    connect(sock, (const struct sockaddr*) &cpu, sizeof(cpu));

	return sock;
}


void exchangeFrames(int sock) {
	payload_t source_frame;
	payload_t received_frame;

	for (int j = 0; j < 100; j++) {
		source_frame.field_a = rand();
		source_frame.field_b =  rand();

		send(sock, &source_frame, sizeof(payload_t), 0);
		printf("Sent: a = 0x%-8x b = 0x%-8x  ", source_frame.field_a, source_frame.field_b);

		recv(sock, &received_frame, sizeof(payload_t), 0);
		printf("Received: a = 0x%-8x b = 0x%-8x\n", received_frame.field_a, received_frame.field_b);

		if (received_frame.field_a != source_frame.field_b || received_frame.field_b != source_frame.field_a) {
			printf("Error!\n");
			exit(1);
		}
	}
}


int main(int argc, char *argv[]) {
	if(argc < 4) {
		printf("Usage: $0 dfe_ip cpu_ip netmask\n");
		return 1;
	}

	struct in_addr dfe_ip;
	inet_aton(argv[1], &dfe_ip);
	struct in_addr cpu_ip;
	inet_aton(argv[2], &cpu_ip);
	struct in_addr netmask;
	inet_aton(argv[3], &netmask);
	const int port = 5007;

	max_file_t *maxfile = FieldSwap_init();
	max_engine_t * engine = max_load(maxfile, "*");

	max_ip_config(engine, MAX_NET_CONNECTION_CH2_SFP1, &dfe_ip, &netmask);
	max_udp_socket_t *dfe_socket = max_udp_create_socket(engine, "udp_ch2_sfp1");
	max_udp_bind(dfe_socket, port);
	max_udp_connect(dfe_socket, &cpu_ip, port);

	int cpu_socket = create_cpu_udp_socket(&cpu_ip, &dfe_ip, port);

	exchangeFrames(cpu_socket);

	max_udp_close(dfe_socket);

	max_unload(engine);
	max_file_free(maxfile);

	printf("Done.\n");
	return 0;
}
