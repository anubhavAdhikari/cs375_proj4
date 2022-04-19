#include <stdio.h>         // for printf, fprintf, perror
#include <stdlib.h>        // for exit
#include <unistd.h>        // for close
#include <errno.h>
#include <string.h>        // for memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>         // for getaddrinfo
#include <sys/time.h>
#include "packet.c"

#define MAXBUFLEN 1032
#define SERVER_PORT "4950"	// the port users will be connecting to

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "usage: talker hostname message\n");
		exit(1);
	}

    // use getaddrinfo to get the IP address of the hostname

    struct addrinfo hints,         // input to getaddrinfo()
	                *server_info;  // head of linked list of results from getaddrinfo()

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;         // IPv4
	hints.ai_socktype = SOCK_DGRAM;    // UDP socket

  int status = getaddrinfo(argv[1], SERVER_PORT, &hints, &server_info);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		exit(1);
	}

	// loop through all the results and make a socket
	int sockfd;
	struct addrinfo *ptr = server_info;  // pointer to current struct addrinfo
	while (ptr != NULL)
	{
		sockfd = socket(ptr->ai_family, ptr->ai_socktype, 0);
		if (sockfd != -1)
		{
			break;
		}
		ptr = ptr->ai_next;
	}

	if (ptr == NULL)
	{
		fprintf(stderr, "talker: failed to create socket\n");
		exit(2);
	}

	// we don't need to call bind() to bind our socket to a port because
	// we are only sending a message and do not expect to receive a response
  //----------------------------------------------------------------------------

	// Use timeval for timeout
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

  //building the packet



  // update declared seqnum_temp, ack_temp, control_temp, and length_temp as unsigned int instead of int
  unsigned int seqnum_temp = 0;
  unsigned int ack_temp = 0;
  unsigned int control_temp = 1;
  unsigned int length_temp = 0;
  // update declared seqnum, ack, control, length, and data as unsigned char instead of char
  unsigned char seqnum[4];
  unsigned char ack[1];
  unsigned char control[1];
  unsigned char length[2];
  char data [1024];

  unsigned char packet[MAXBUFLEN]; //combination of all char arrays from line 70-74



	buildPacket(seqnum_temp,ack_temp,control_temp,length_temp,data,&packet);
	//send nickname to server
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) > 0)
	{
		perror("Error");
		exit(1);
	}

  int numbytes = sendto(sockfd, packet, sizeof(packet), 0, ptr->ai_addr, ptr->ai_addrlen);
	if ((numbytes) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}

  freeaddrinfo(server_info);

  //receive acknowledgement from server
  printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
  struct sockaddr_storage their_addr;
  socklen_t addr_len = sizeof their_addr;
  char buffer[MAXBUFLEN];

  if ( (numbytes = recvfrom(sockfd,buffer,MAXBUFLEN -1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1)
  {
    perror("recvfrom");
    exit(1);
  }

	while(1)
	{
		if(buffer[4]==1)
		{

			break;
		} else{
				sendto(sockfd, packet, sizeof(packet), 0, ptr->ai_addr, ptr->ai_addrlen);
				recvfrom(sockfd,buffer,MAXBUFLEN -1, 0, (struct sockaddr *) &their_addr, &addr_len);
		}
	}
	while(1)
	{

		seqnum_temp = abs(seqnum_temp -1);
		printf("Enter a data: ");
		fgets(data, 1024, stdin);
		if(strncmp("exit",data,4)==0)
		{
			buildPacket(seqnum_temp,ack_temp,2,strlen(data)-1,data,&packet);
			sendto(sockfd, packet, sizeof(packet), 0, ptr->ai_addr, ptr->ai_addrlen);
			break;
		}
		buildPacket(seqnum_temp,ack_temp,0,strlen(data)-1,data,&packet);
		sendto(sockfd, packet, sizeof(packet), 0, ptr->ai_addr, ptr->ai_addrlen);
		recvfrom(sockfd,buffer,MAXBUFLEN -1, 0, (struct sockaddr *) &their_addr, &addr_len);
		unsigned int returnArr[4];
		unpackPacket(&buffer,&returnArr);

		while(1){
			if(seqnum_temp == returnArr[0])
			{
				sendto(sockfd, packet, sizeof(packet), 0, ptr->ai_addr, ptr->ai_addrlen);
				recvfrom(sockfd,buffer,MAXBUFLEN -1, 0, (struct sockaddr *) &their_addr, &addr_len);
			}
			else
			{
				break;
			}
		}

	}


  close(sockfd);

  return 0;
}
