#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "simulator.h"


Environment    environment;  // The environment that contains all the robots

// this function handles incoming client requests from the server socket. It repeatedly grab incoming messages and processes them.
// if the server recieves a REGISTER request, it will register and display the new robot client.
// if the server recieves a CHECK_COLLISION request, it will check if robot will either collide with the borders of the window or another robot and sends
// back a 'OK' if the robot can move forward,'NOT_OK_BOUNDARY' if the robot will crash on to a wall of the window if it were to move, or a 'NOT_OK_COLLIDE'
// if the robot would crash with another robot if it were to move, or a 'NOT_OK' if theres too many robots in the server and new robots are no longer
// accepted.
// when the server recieves the 'STOP' command it will set the shutdown property of the envirnment struct to true and proceed to send 'LOST_CONTACT' signals
// to all registered robots at which point all robots will stop their processes, once all robots are off the server will also stop itself.
void *handleIncomingRequests(void *e) {
	char   online = 1;
  	int                 serverSocket;
  	struct sockaddr_in  serverAddr, clientAddr;
  	int                 status, addrSize, bytesReceived;
  	fd_set              readfds, writefds;
  	char                buffer[30];
  	float 		    arr [5];
  	char*   	    arr2 [sizeof(arr)];
  	float 		    x;
  	float		    y;
  	int 		    direction; 
  	float 		    clientInput[5];
  	

  	// Initialize the server
  	// Create the server socket
  	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  	if (serverSocket < 0) {
    		printf("*** SERVER ERROR: Could not open socket.\n");
    		exit(-1);
  	}
   	// Setup the server address
  	memset(&serverAddr, 0, sizeof(serverAddr)); // zeros the struct
  	serverAddr.sin_family = AF_INET;
  	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  	serverAddr.sin_port = htons((unsigned short) SERVER_PORT);
  	// Bind the server socket
  	status = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  	if (status < 0) {
    		printf("*** SERVER ERROR: Could not bind socket.\n");
    		exit(-1);
  	}
  	// Wait for clients now
	while (online) {
    		FD_ZERO(&readfds);
    		FD_SET(serverSocket, &readfds);
    		FD_ZERO(&writefds);
    		FD_SET(serverSocket, &writefds);
    		status = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
    		if (status == 0) {
      			// Timeout occurred, no client ready
      			printf("no client ready\n");
    		}else if (status < 0) {
      			printf("*** SERVER ERROR: Could not select socket.\n");
      			exit(-1);
    		}else {
      			addrSize = sizeof(clientAddr);
      			bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer),0, (struct sockaddr *) &clientAddr, &addrSize);
      		if (bytesReceived > 0) {
        		buffer[bytesReceived] = '\0';
    			
      		}
      		memcpy(&clientInput,buffer,sizeof(clientInput));
        	//printf("SERVER: Received client request: %s\n", buffer);
      		if(strcmp(buffer, "test") == 0){
      			char * res = "5";
			//printf("SERVER: got the test Sending \"%f\" to client\n", arr2[0]);
      			sendto(serverSocket, res, strlen(res), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
      		}
		else if (clientInput[0] == (float)REGISTER){// (strcmp(&buffer[0], "1") == 0
			if(environment.numRobots < 20){
				x = rand()%((ENV_SIZE - ROBOT_RADIUS) - ROBOT_RADIUS + 1) + ROBOT_RADIUS;
				y = rand()%((ENV_SIZE - ROBOT_RADIUS) - ROBOT_RADIUS + 1) + ROBOT_RADIUS;
				direction = rand()%(180);
				direction = direction - rand()%(180);
				if(environment.numRobots > 0){
					int willOverlap = 1;
					while(willOverlap == 1){
						willOverlap = 0;
						for (int i = 0; i < environment.numRobots; i++){
							if(i == environment.numRobots-1){
								continue;
							}
							float checkX = environment.robots[i].x;
      							float checkY = environment.robots[i].y;
      							if(sqrt(pow((checkX-x), 2) + pow((checkY-y),2) ) <= (2 * ROBOT_RADIUS)){
      								x = rand()%((ENV_SIZE - ROBOT_RADIUS) - ROBOT_RADIUS + 1) + ROBOT_RADIUS;
								y = rand()%((ENV_SIZE - ROBOT_RADIUS) - ROBOT_RADIUS + 1) + ROBOT_RADIUS;
								direction = rand()%(180);
								direction = direction - rand()%(180);
								willOverlap = 1;
      							}
						}
					}
				}
				environment.robots[environment.numRobots].x = x;
				environment.robots[environment.numRobots].y = y;
				environment.robots[environment.numRobots].direction = direction;
				arr[0] = (float)5;
				arr[1] = (float) environment.numRobots;
				arr[2] = x;
				arr[3] = y;
				arr[4] = (float)direction;
				environment.numRobots++;
				memcpy(arr2,&arr,sizeof(arr));
				//printf("SERVER: Sending \"%f\" to client\n", arr2[0]);
      				sendto(serverSocket, arr2, sizeof(arr2), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
      			}else{
				arr[0] = (float) 6;
				//printf("SERVER: Sending \"%f\" to client\n", arr[0]);
      				sendto(serverSocket, arr, sizeof(arr), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
      			}
      			
		}
      		else if (clientInput[0] == (float)CHECK_COLLISION){
      			if(environment.shutDown == 1){
      				clientInput[0] = (float)9;
      				sendto(serverSocket,clientInput, sizeof(clientInput), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
      				environment.numRobots--;
      				if(environment.numRobots == 0){
      					printf("all robots have recieved LOST_CONTACT signal, sutting down now!\n");
      					exit(-1);
      				}
      			}
      			
      			
      			
      			int robotID = (int)clientInput[1];
      			environment.robots[robotID].direction = clientInput[4];
      			float newX = (float)(environment.robots[robotID].x + ROBOT_SPEED * cos(((environment.robots[robotID].direction*PI)/180)));
      			float newY = (float)(environment.robots[robotID].y + ROBOT_SPEED *sin(((environment.robots[robotID].direction*PI)/180)));
      			int allGood = 1;
      			if ((newX > (ENV_SIZE - ROBOT_RADIUS)) || (newY > (ENV_SIZE - ROBOT_RADIUS)) ||(newX <= (ROBOT_RADIUS)) || (newY <= (ROBOT_RADIUS))){
      				clientInput[0] = (float)7;
      				//printf("SERVER: Sending \"%f\" to client\n", clientInput[0]);
      				allGood = 0;
      				sendto(serverSocket,clientInput, sizeof(clientInput), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));	
      				
      			}else{
      				for(int i = 0; i < environment.numRobots; i++){
      					if(i == robotID){
      						continue;
      					}
      					float tempX = environment.robots[i].x;
      					float tempY = environment.robots[i].y;
      					float s = sqrt(pow((tempX-newX), 2) + pow((tempY-newY),2) ); 
      					if(sqrt(pow((tempX-newX), 2) + pow((tempY-newY),2) ) <= (2 * ROBOT_RADIUS)){
      						clientInput[0] = (float)8;
      						sendto(serverSocket,clientInput, sizeof(clientInput), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
      						allGood = 0;
      						break;
      					}
      				}
      				if(allGood == 1){
					clientInput[0] = (float)5;
					clientInput[2] = newX;
					clientInput[3] = newY;
      					//printf("SERVER: Sending \"%f\" to client bc robot will not collide\n", clientInput[0]);
      					sendto(serverSocket,clientInput, sizeof(clientInput), 0,(struct sockaddr *) &clientAddr, sizeof(clientAddr));
      					environment.robots[(int)clientInput[1]].x = newX;
					environment.robots[(int)clientInput[1]].y = newY;
      				}
      			}
		
		}
		else if (clientInput[0] == (float)STATUS_UPDATE){
      			environment.robots[(int)clientInput[1]].x = clientInput[2];
			environment.robots[(int)clientInput[1]].y = clientInput[3];
			environment.robots[(int)clientInput[1]].direction = clientInput[4];
		}
      		// If the client said to stop, then I'll stop myself
      		else if (clientInput[0] == (float)STOP){  
			environment.shutDown = 1;
			}
    		}
  	}
}




int main() {
	// So far, the environment is NOT shut down
	environment.shutDown = 0;
	pthread_t thread_1, thread_2;
  
	// Set up the random seed
	srand(time(NULL));

	// Spawn an infinite loop to handle incoming requests and update the display
	while(1){
		pthread_create(&thread_1,NULL,handleIncomingRequests,&environment);
		pthread_create(&thread_2,NULL,redraw,&environment);
		pthread_join(thread_1, NULL);
		pthread_join(thread_2, NULL);
	}
}