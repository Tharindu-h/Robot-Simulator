#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"




// This is the main function that simulates the "life" of the robot
// The code will exit whenever the robot fails to communicate with the server
int main() {
    int                 clientSocket, addrSize, bytesReceived;
    struct sockaddr_in  clientAddr;
    char                inStr[80];    // stores user input from keyboard
    char                buffer[80];   // stores sent and received data
    float 	      arr2[5];
    char*  	      test = "test";
    int 		      turnAngle = -1;
    // Create socket
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
  
    // Set up the random seed
    srand(time(NULL));
  
    //printf("CLIENT: Sending \"%s\" to server.\n", test);
    sendto(clientSocket, test, strlen(test), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));

    addrSize = sizeof(clientAddr);
    bytesReceived = recvfrom(clientSocket, buffer, 80, 0,(struct sockaddr *) &clientAddr, &addrSize);
    buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
    
    
    
  // Go into an infinite loop exhibiting the robot behavior
    if(strcmp(&buffer[0],"5")==0){
    	
    	arr2[0] = (float)REGISTER;
    	sendto(clientSocket, arr2, sizeof(arr2), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
 	while (1) {
    		addrSize = sizeof(clientAddr);
    		bytesReceived = recvfrom(clientSocket, buffer, 80, 0,(struct sockaddr *) &clientAddr, &addrSize);
    		buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
    		float arr[5];
    		
    		usleep(3000);
    		memcpy(&arr,buffer,sizeof(arr));
    		//printf("CLIENT: Got back response \"%d\" from server.\n", (int)arr[0]);
    		turnAngle = rand()%(2);// random number between 0 and 1, determines whather the direction of the robots movement will be 
    				      // increased or decresed.
    		// if the server any respnse other than 'OK' the robotClient will enter this loop. 
    		// this loop changes the direction of the robot until the server sends the 'OK' response.
    		while(arr[0] != (float) OK){
    			if(arr[0] == (float) NOT_OK_BOUNDARY){
    				if(turnAngle == 1){
    					arr[4] += ROBOT_TURN_ANGLE;
    					arr[0] = (float)CHECK_COLLISION;
 					sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
    				}else{
    					arr[4] -= ROBOT_TURN_ANGLE;
    					arr[0] = (float)CHECK_COLLISION;
 					sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
    				}
    			}else if(arr[0] == (float) NOT_OK_COLLIDE){
    				if(turnAngle == 1){
    					arr[4] += ROBOT_TURN_ANGLE;
    					arr[0] = (float)CHECK_COLLISION;
 					sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
    				}else{
    					arr[4] -= ROBOT_TURN_ANGLE;
    					arr[0] = (float)CHECK_COLLISION;
 					sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
    				}
    			}else if(arr[0] == (float)LOST_CONTACT){
    				printf("LOST_CONTACT,SHUTTING DOWN\n");
    				exit(-1);
    			}else if(arr[0] == (float)NOT_OK){
    				printf("Sorry cannot register too many robots! shutting down.\n");
    				exit(-1);
    			}
    			bytesReceived = recvfrom(clientSocket, buffer, 80, 0,(struct sockaddr *) &clientAddr, &addrSize);
    			buffer[bytesReceived] = 0;
    			memcpy(&arr,buffer,sizeof(arr));
    		
    		}
    		
    		if(arr[0] == (float) OK){
 			arr[0] = (float)CHECK_COLLISION;
 			sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
 			arr[0] = (float)STATUS_UPDATE;
 			sendto(clientSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
 		}
  	}
  }
}
