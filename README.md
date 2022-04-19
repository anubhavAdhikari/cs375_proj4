### CS375-Project 4: Stop-and-wait protocol
### Submitted by: Anubhav Adhikari, Brendon Jean-Baptiste
### Submitted to: Dr. Jessen Havill
### Date: April 18, 2022

### Project 4:
In this project, we implemented a simple stop-and-wait protocol between a client and a server. Here, the client sends an initial packet as a request to the server to initiate a connection, and subsequently sends packets of data to the server. The server acknowledges the packet, and sends an ACK to the client with the opposite sequence number. We keep track of new packets by analyzing the sequence number `seqVal` that is sent by either client or server. If the sequence number is same as the last packet's, then the same packet has been resent and there has either been a problem in the transmission, or the timeout was activated in the client.

### Packet:
In this project, the packet is a 1032 byte unsigned char array, divided further into these unsigned char arrays: `seqnum` (4 bytes), `ack` (1 byte), `control` (1 byte), `length` (2 bytes), `data` (1024 bytes, at maximum).
```c
  unsigned char seqnum[4];
  unsigned char ack[1];
  unsigned char control[1];
  unsigned char length[2];
  char data [1024];
```

### Build Packet:
We implemented a function `buildPacket` that combines all the different variables into the final packet `packet`:
```c
void buildPacket(unsigned int seqnum_temp, unsigned int ack_temp, unsigned int control_temp, unsigned int length_temp,
  char data_1[1024], unsigned char (*packet)[])
{
  unsigned char seqnum[4];
  unsigned char ack[1];
  unsigned char control[1];
  unsigned char length[2];

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
}
```

### Unpack Packet:
Once a packet is sent to the server/client, it is unpacked, so that the content can be transferred back to their original variable types and used for further logic in the code (for example, `seqnum` is an unsigned int). We implemented a function `unpackPacket` that unpacks the packet:
```c
void unpackPacket(char (*buffer)[], unsigned int (*returnArr)[]){

  unsigned int b0 = (*buffer)[0];
  unsigned int b1 = (*buffer)[1];
  unsigned int b2 = (*buffer)[2];
  unsigned int b3 = (*buffer)[3];
  unsigned int seqnum1 = (b0 << 24) + (b1 << 16) + (b2 << 8) + b3; //converting bytes back to int
  (*returnArr)[0] = seqnum1;

  unsigned int c0;
  c0 = (*buffer)[4];
  unsigned int ack1 = c0; //converting bytes back to int
  (*returnArr)[1] = ack1;

  unsigned int t0;
  t0 = (*buffer)[5];
  unsigned int control1 = t0; //converting bytes back to int
  (*returnArr)[2] = control1;

  unsigned int l0,l1;
  l0 = (*buffer)[6];
  l1 = (*buffer)[7];
  unsigned int length1 = (l0 << 8) + l1; //converting bytes back to int
  (*returnArr)[3] = length1;

}
```

### Initiate Connection:
To initiate a connection with the server, the client sends an initial packet with `control` value as `1`, indicating that it is trying to connect with the server. The server makes this check by unpacking the packet, and checking the `control` value, which is stored in the variable `returnArr[2]`. If it finds that this is the first packet received from this client (`initial == True`), then it sends the first ACK back, as follows:
```c
  if(returnArr[2] == 1 && initial){
      printf("Sending initial packet...");
      seqVal = returnArr[0];
      buildPacket(seqVal, 1, 1, 0, data, &packet);
      sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&sender_addr, addr_len);
    }
```

### Teardown Connection:
To teardown a connection and close the sockets, the client sends a packet with `control` value 2, which the server reads and uses as a signal to close the connection.

### Server Logic: Stop-and-wait
In the server, the packet that is received is unpacked, and the `control` value is checked, to see which type of message the packet is intending to transmit. We have seen that `control` with value 1 and 2 indicate connection setup and teardown respectively. When a packet arrives with `control` value 0, it is indicative of a packet with some data.

The value for `control` is unpacked into the variable `returnArr[2]` by the `unpackPacket` function. After checking for the `control` value, the server checks to see if the sequence number in the packet, unpacked into the variable `returnArr[0]` is the same as the last packet's sequence number, stored in the variable `seqVal`. If it is the same, then the server will assume that the ACK for that last packet never got received by the client, so it will resend that same packet, that has been backed up in the variable `packetBackup`. If it is a new packet, then the server will create a new ACK and send it to the client.
```c
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
```

### Client Logic: Stop-and-wait
In the client, before a packet is sent, the user is prompted to type a message in the terminal, which is received as the `data` that will be appended to the `packet`. This is done through the `fgets` function, as follows:
```c
   fgets(data, 1024, stdin);
```
Once a packet is sent to the server, the client waits for an ACK. If it does not receive an ACK, after a certain timeout, it resends the packet. The timeout is implemented using the `struct timeval` library, as follows:
```c
struct timeval timeout;
timeout.tv_sec = 0.5;
timeout.tv_usec = 0;
if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) > 0)
{
    perror("Error");
	exit(1);
}
```
Once a client resends a packet, it will enter a second while loop, that will continue resending the packet until an ACK is received for that packet. It checks for a new ACK by comparing the sequence number `seqnum_temp` it last sent, to the new sequence number it unpacked and stored in the variable `returnArr[0]`, as follows:
```c
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
```
### Further Considerations:
In the scope of this project, the `data` sent by the client does not get unpacked by the server. Right now, we have worked on the stop-and-wait protocol only, ensuring that any lost packets can be retransmitted and there is reliable data transmission. Further work can be done on this project, particularly to save the `data` in the server side, and create a message history, or a messaging system.

In this way, we have implemented a stop-and-wait protocol.
