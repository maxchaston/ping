#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

// C ICMP ping utility

// Requires "raw" sockets, as traditional stream and datagram sockets force use of TCP and UDP respectively.
// ICMP is a separate protocol and as such requires manual packing of an IPv4 packet (or IPv6 in the case of ICMPv6, which is not implemented here).
// Socket info can be found at https://man7.org/linux/man-pages/man2/socket.2.html
// ICMP is defined https://datatracker.ietf.org/doc/html/rfc792
// raw socket info can be found in volume 1 of Unix Network Programming by W. Richard Stevens
// man raw(7)

// In order to create a raw socket, CAP_NEW_RAW permissions are required. Typically this is reserved for the root user on a linux system.

struct icmphdr
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t sequence;
	uint16_t data;
};

struct icmp
{
	struct icmphdr hdr;
	uint8_t data[64];
};

int main()
{
	int sockfd;
	sockfd = socket(AF_INET, SOCK_RAW, 1); // ICMP is protocol 1, the protocol is inserted into the IPv4 packet header. 
	if (sockfd == -1)
	{
		fprintf(stderr, "Error opening socket, %d %s\n", errno, strerror(errno));
		exit(1);
	}

	struct icmp icmp;
	/* memset(&icmp, 0, sizeof(struct icmp)); // ensures struct is empty */
	icmp.hdr = (struct icmphdr){8, 0, htons(38203), 0, 0}; // 8 for echo message, 0 for echo reply, checksum manually calculated, htons needed for 16 bit numbers
	memset(icmp.data, 0, sizeof icmp.data);
	icmp.data[1] = 'a';
	icmp.data[2] = 'b';
	icmp.data[3] = 'c';

	// man getaddrinfo(3)
	struct addrinfo hints;
	struct addrinfo *servinfo; // results
	memset(&hints, 0, sizeof hints); // ensures struct is empty
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_RAW;
	/* hints.ai_flags = AI_PASSIVE; */ // irrelevant when node (localhost) is defined
	int status;
	// service must be NULL, raw sockets do not support the concept of services according to the EAI_SERVICE error description
	if ((status = getaddrinfo("localhost", NULL, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "Error filling addrinfo struct, %d %s\n", status, strerror(status));
		exit(1);
	}

	sendto(sockfd, &icmp, sizeof icmp, 0, servinfo->ai_addr, servinfo->ai_addrlen);
	printf("Ping successful\n");
	return 0;
}
