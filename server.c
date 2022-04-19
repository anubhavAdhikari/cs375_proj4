#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "packet.c"
#include <stdbool.h>

#define MYPORT 4950

int main(void)
{
  //Setting up socket file descriptor
  printf("starting listener...\n");
  int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
  if(sockfd == -1){
    perror("listener: socket");
    exit(1);
  }

  //Setting up socket attributes
  struct sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(MYPORT);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero);

  //Binding socket
  if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1){
    close(sockfd);
    perror("listener: bind");
    exit(1);
  }

  //----------------------------------------------------------------------------
  //Connecting to user
  printf("listener: waiting(0) to recvfrom...\n");

  char buffer[MAXBUFLEN];
  struct sockaddr_storage sender_addr;
  socklen_t addr_len = sizeof sender_addr;

  unsigned int controlVal = 0;
  bool initial = true;
  unsigned int seqVal = 2;
  unsigned char packetBackup[MAXBUFLEN];

  unsigned int seqnum_temp = 0;
  unsigned int ack_temp = 1;
  unsigned int control_temp = 1;
  unsigned int length_temp = 0;
  char *data = "";

  while(controlVal != 2){
    //receiving client initial packet
    int numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1, 0, (struct sockaddr *)&sender_addr, &addr_len);
    if(numbytes == -1){
      perror("listener:recvfrom");
      exit(1);
    }
    buffer[numbytes] = '\0';

    //----------------------------------------------------------------------------
    unsigned int returnArr[4];
    unpackPacket(&buffer, &returnArr);
    if(seqVal != 2 && seqVal != returnArr[0]){
      initial = false;
    }

    printf("Seqnum: %d\n", returnArr[0]);
    printf("ack: %d\n", returnArr[1]);
    printf("control: %d\n", returnArr[2]);
    printf("length: %d\n", returnArr[3]);

    unsigned char packet[MAXBUFLEN];
    if(returnArr[2] == 1 && initial){
      seqVal = returnArr[0];
      buildPacket(seqVal, 1, 1, 0, data, &packet);
      sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&sender_addr, addr_len);
    }

    else if(returnArr[2] == 0 && !initial){
      if(seqVal == returnArr[0]){
        sendto(sockfd, packetBackup, sizeof(packet), 0, (struct sockaddr *)&sender_addr, addr_len);
      }
      else{
        seqVal = returnArr[0];
        buildPacket(abs(seqVal-1), 1, 0, 0, data, &packet);
        strcpy(packetBackup,packet);
        sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&sender_addr, addr_len);
      }
    }

    else if(returnArr[2] == 2){
      controlVal = 2;
    }

    else{
      break;
    }
  }

  //----------------------------------------------------------------------------

  close(sockfd);

  return 0;
}
