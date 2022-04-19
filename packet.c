#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXBUFLEN 1032

void buildPacket(
  unsigned int seqnum_temp,
  unsigned int ack_temp,
  unsigned int control_temp,
  unsigned int length_temp,
  char data_1[1024],
  unsigned char (*packet)[]
){
  //building the packet
  // update declared seqnum, ack, control, length, and data as unsigned char instead of char
  unsigned char seqnum[4];
  unsigned char ack[1];
  unsigned char control[1];
  unsigned char length[2];


  // unsigned char packet[MAXBUFLEN]; //combination of all char arrays from line 16-20

  //----------------------------------------------------------------------------
  // new update change htons to htonl, and declared new_int as unsigned int instead of int
  // storing various int values in unsigned char arrays
  unsigned int new_int = htonl(seqnum_temp);
  memcpy(seqnum, &new_int, 4);

  memcpy(ack, &ack_temp, 1);

  memcpy(control, &control_temp, 1);

  unsigned int new_int_1 = htons(length_temp);
  memcpy(length, &new_int_1, 2);

  //----------------------------------------------------------------------------
	//combining all the char arrays into 'packet' array
	int index = 0;

  for(int i = 0; i < sizeof(seqnum); i++){
    (*packet)[index] = seqnum[i];
    index = index + 1;
  }
  for(int i = 0; i < sizeof(ack); i++){
    (*packet)[index] = ack[i];
    index = index + 1;
  }
  for(int i = 0; i < sizeof(control); i++){
    (*packet)[index] = control[i];
    index = index + 1;
  }
  for(int i = 0; i < sizeof(length); i++){
    (*packet)[index] = length[i];
    index = index + 1;
  }
  for(int i = 0; i < strlen(data_1)-1; i++){
    (*packet)[index] = data_1[i];
    index = index + 1;
  }
	(*packet)[index] = '\0';
	printf("index: %d\n",index);
	printf("size of packet: %ld\n",sizeof(packet));

}

void unpackPacket(char (*buffer)[], unsigned int (*returnArr)[]){
  printf("Entered unpackPacket...\n");

  unsigned int b0 = (*buffer)[0];
  unsigned int b1 = (*buffer)[1];
  unsigned int b2 = (*buffer)[2];
  unsigned int b3 = (*buffer)[3];
  unsigned int seqnum1 = (b0 << 24) + (b1 << 16) + (b2 << 8) + b3; //converting bytes back to int
  // printf("Seqnum_0: %d\n",seqnum1);
  (*returnArr)[0] = seqnum1;

  unsigned int c0;
  c0 = (*buffer)[4];
  unsigned int ack1 = c0; //converting bytes back to int
  // printf("ack_0: %d\n",ack1);
  (*returnArr)[1] = ack1;

  unsigned int t0;
  t0 = (*buffer)[5];
  unsigned int control1 = t0; //converting bytes back to int
  // printf("control_0: %d\n",control1);
  (*returnArr)[2] = control1;

  unsigned int l0,l1;
  l0 = (*buffer)[6];
  l1 = (*buffer)[7];
  unsigned int length1 = (l0 << 8) + l1; //converting bytes back to int
  // printf("length_0: %d\n",length1);
  (*returnArr)[3] = length1;

}
