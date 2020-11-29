Name : Tharindu Hatharasinghage
Student ID : 101109324
Source files submitted : display.c , makefile , robotClient.c , stop.c , environmentServer.c , Readme.txt , simulator.h

Note: The server nor the robotClient processes will print anything on the terminal. the robotClient.c is set up such that as soon as the program is run
      it will register a robot to the server, neither one of the excutable files will take any input from the terminal. It is recommended to run the 
      robotClient processes on the background. To stop all processes included in this program run the stop.c file. 

To compile : use "make" command
To run the server : use "./environmentServer" command 
to run the robotClient : use "./robotClient&" command (runs the program on the background)
to send the STOP command : use "./stop" command