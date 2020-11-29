#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


int main() {
    int                 clientSocket, addrSize, bytesReceived;
    struct sockaddr_in  clientAddr;
    char                buffer[80];   // stores sent and received data
  
    // Register with the server
    clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket < 0) {
      printf("*** CLIENT ERROR: Could open socket.\n");
      exit(-1);
    }
  
    // Setup address 
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    clientAddr.sin_port = htons((unsigned short) SERVER_PORT);
    //sending the test command to establish a connection with the server
    char*  test = "test";
    printf("CLIENT: Sending \"%s\" to server.\n", test);
    sendto(clientSocket, test, strlen(test), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
  
    addrSize = sizeof(clientAddr);
    bytesReceived = recvfrom(clientSocket, buffer, 80, 0,(struct sockaddr *) &clientAddr, &addrSize);
    buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
  
    float arr[5];
    memcpy(&arr,buffer,sizeof(arr));
    printf("CLIENT: Got back response \"%d\" from server.\n", (int)arr[0]);
    printf("Connection established, sending the STOP command!\n");
    // sending the STOP command
    arr[0] = (float)STOP;
    sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
    exit(0);

}