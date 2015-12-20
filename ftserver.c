/******************************************************************
 * File Transfer Application
 *
 * Programmer: Bobby Hines
 *
 * Description:
 * This is the "server" side of the file transfer program. It will
 * run until told to die via signal (or crashes for some reason).
 *
 * Input: The user must specify a control port in order to start
 * this program. This is passed as a command line argument. It 
 * also receives input via the control socket from the client app.
 *
 * Output: On the server side, the only output are console messages
 * with status updates about activity between clients. It also 
 * outputs messages and files through its control and data sockets
 * to connected clients.
 *
 ******************************************************************/

#include <stdlib.h> // atoi(), strtol(), etc.
#include <stdio.h> // Basic input/output
#include <unistd.h> // read(), open(), etc.
#include <string.h> // Strings
#include <sys/socket.h> // Socket library
#include <ctype.h> // isdigit()
#include <netinet/in.h> // Internet socket
#include <stdbool.h> //Booleans
#include <dirent.h> // Display contents of working directory

#define BUFSIZE 1024 // Controls the size of all string buffers

// Function declarations
char* substring(const char* str, size_t begin, size_t len);
void error(const char *errMsg);
bool isNumber(const char *s);
int bytes_added(int result_of_sprintf);

int main(int argc, char *argv[]) {
	int sockfd, acceptfd, acceptfd2, datafd, controlPort, dataPort, b, b2, n, cliUsrLen, cliPwLen, filenameLen;
	struct sockaddr_in serverAddr, clientAddr, serverAddr2, clientAddr2;
	char recvbuf[BUFSIZE], sendbuf[BUFSIZE], command[20];
	socklen_t cliLen, cliLen2; // Initialize client address length
	char *cliUsr, *cliPw, *sendTxt, *cflag, *filename;

	// Hardcoded credentials (for now)
	char *username = "subraman";
	char *password = "password";

	// Validate command-line args (was a portnum specified?)
	if (argc != 2 || !isNumber(argv[1])) error("Usage: ./ftserver <port number>\n"); 
	else {
		controlPort = atoi(argv[1]);
		if (controlPort < 1024 || controlPort > 65535) 
			error("Error: Control port must be between 1024 and 65535.\n");
	}

	// Create socket with IPv4 Internet protocols, 2-way stream
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("There was a problem opening the socket!\n");

    bzero((char *) &serverAddr, sizeof(serverAddr)); //Zero out the memory address
    // Define properties of server address
    serverAddr.sin_port = htons(controlPort); // Convert to network byte order
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;

	// Bind socket to specified port
    b = bind(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (b < 0) error("There was a problem binding the socket!\n");

	// Wait for client request (for now, connection queue is only 1)
	b = listen(sockfd, 1);
	if (b < 0) {
		error("There was a problem listening for connections!\n");
	}

	printf("Server open on %d.\n", controlPort);

	cliLen = sizeof(clientAddr);

    while(1) {
    	// Establish control connection on port #
		acceptfd = accept(sockfd,(struct sockaddr*)&clientAddr, &cliLen);
		if (acceptfd < 0) {
			error("There was a problem accepting connections!\n");
		}

		printf("Incoming client request...Verifying credentials...\n");

	    // Get username from client
	    bzero(recvbuf, BUFSIZE); // Clear the string buffer for received messages
	    n = read(acceptfd, recvbuf, BUFSIZE); // Read any incoming messages
	    if (n < 0) {
	    	error("There was a problem reading from socket!\n");
	    }

	    cliUsr = recvbuf;
	    cliUsrLen = atoi(substring(cliUsr, 0, 4)); // Extract string length from header
	    cliUsr = substring(cliUsr, 4, cliUsrLen);
	    printf("Username: %s\n", cliUsr);
	    fflush(stdout); // Flush the output buffer

		// Get password from client
	    bzero(recvbuf, BUFSIZE); // Clear the string buffer for received messages
	    n = read(acceptfd, recvbuf, BUFSIZE); // Read any incoming messages
	    if (n < 0) {
	    	error("There was a problem reading from socket!\n");
	    }

	    cliPw = recvbuf;
	    cliPwLen = atoi(substring(cliPw, 0, 4)); // Extract string length from header
	    cliPw = substring(cliPw, 4, cliPwLen);
	    printf("Password: %s\n", cliPw);
	    fflush(stdout); // Flush the output buffer

	    if (strcmp(cliUsr, username) != 0 || strcmp(cliPw, password) != 0) {
	    	sendTxt = "ACCESS DENIED";
	    	bzero(sendbuf, BUFSIZE);
	        snprintf(sendbuf, BUFSIZE, "%s", sendTxt);
	        n = write(acceptfd, sendbuf, strlen(sendbuf));
	        if (n < 0) {
	        	error("There was a problem writing to the socket!\n");
	        }

	        fflush(stdout); // Flush the output buffer
	        printf("Invalid credentials, access denied.\n");
	    } else {
	        sendTxt = "Access granted!";
	    	bzero(sendbuf, BUFSIZE);
	        snprintf(sendbuf, BUFSIZE, "%s", sendTxt);
	        n = write(acceptfd, sendbuf, strlen(sendbuf));
	        if (n < 0) {
	        	error("There was a problem writing to the socket!\n");
	        }
	    	
	    	// Get command
	        bzero(recvbuf, BUFSIZE); // Clear the string buffer for received messages
		    n = read(acceptfd, recvbuf, BUFSIZE); // Read any incoming messages
		    if (n < 0) {
		    	error("There was a problem reading from socket!\n");
		    }

		    cflag = recvbuf;

		    DIR *d;
			struct dirent *dir;
			d = opendir(".");
			bzero(sendbuf, BUFSIZE);

		    if (strcmp(cflag, "-l") == 0) {
		    	printf("The user would like a list of the current directory.\n");
		    	printf("Sending current directory contents...\n");

				if (d) {
					int length = 0;
				    while ((dir = readdir(d)) != NULL) {
				      	//printf("%s\n", dir->d_name);
				    	length += bytes_added(sprintf(sendbuf+length, "%s\n", dir->d_name));
				      	//snprintf(sendbuf, BUFSIZE, "%s\n", dir->d_name);
				    }
				    closedir(d);
				}

				n = write(acceptfd, sendbuf, strlen(sendbuf));
		        if (n < 0) {
		        	error("There was a problem writing to the socket!\n");
		        }

		        printf("Done.\n");

		    } else if (strcmp(cflag, "-g") == 0) {

		    	// get filename
		        bzero(recvbuf, BUFSIZE); // Clear the string buffer for received messages
			    n = read(acceptfd, recvbuf, BUFSIZE); // Read any incoming messages
			    if (n < 0) {
			    	error("There was a problem reading from socket!\n");
			    }
			    filename = recvbuf;
			    filenameLen = atoi(substring(filename, 0, 3)); // Extract string length from header
			    filename = substring(filename, 3, filenameLen);

			    printf("The user has requested the file %s.\n", filename);
		    	printf("Scanning current directory for file...\n");

		    	bool fileExists = false;
		    	// Scan for file to see if it exists
				if (d) { 
				    while ((dir = readdir(d)) != NULL) {
				      	//printf("%s\n", dir->d_name);
				    	//sprintf(sendbuf, "%s\n", dir->d_name));
				      	//snprintf(sendbuf, BUFSIZE, "%s\n", dir->d_name);
				      	if (strcmp(dir->d_name, filename) == 0) {
				      		fileExists = true;
				      	}
				    }
				    closedir(d);
				}

				if (fileExists == false) {
					sendTxt = "NOT FOUND";

			    	bzero(sendbuf, BUFSIZE);
			        snprintf(sendbuf, BUFSIZE, "%s", sendTxt);

			        n = write(acceptfd, sendbuf, strlen(sendbuf));
			        if (n < 0) error("There was a problem writing to the socket!\n");
				} else if (fileExists == true) {
					printf("File located!\n");
					sendTxt = "FOUND!";

			    	bzero(sendbuf, BUFSIZE);
			        snprintf(sendbuf, BUFSIZE, "%s", sendTxt);

			        n = write(acceptfd, sendbuf, strlen(sendbuf));
			        if (n < 0) error("There was a problem writing to the socket!\n");

			        // Get the client's preferred data port
			        bzero(recvbuf, BUFSIZE); // Clear the string buffer for received messages

				    n = read(acceptfd, recvbuf, BUFSIZE); // Read any incoming messages
				    if (n < 0) error("There was a problem reading from socket!\n");
				    
				    dataPort = atoi(recvbuf);

				    printf("Preparing to send file over port %i...\n", dataPort);

				    // Open new socket with user's data port
					datafd = socket(AF_INET, SOCK_STREAM, 0);
					if (datafd < 0) error("There was a problem opening the socket!\n");

					bzero((char *) &serverAddr2, sizeof(serverAddr2)); //Zero out the memory address
				    // Define properties of server address
				    serverAddr2.sin_port = htons(dataPort); // Convert to network byte order
					serverAddr2.sin_addr.s_addr = INADDR_ANY;
					serverAddr2.sin_family = AF_INET;

					// Bind socket to specified data port
				    b2 = bind(datafd, (struct sockaddr*) &serverAddr2, sizeof(serverAddr2));
				    if (b2 < 0) error("There was a problem binding the data socket!\n");

				    // Wait for client request (for now, connection queue is only 1)
					b2 = listen(datafd, 1);
					if (b2 < 0) error("There was a problem listening for connections!\n");

					printf("Now listening on %d.\n", dataPort);

					cliLen2 = sizeof(clientAddr2);
					acceptfd2 = accept(datafd,(struct sockaddr*)&clientAddr2, &cliLen2);
					if (acceptfd2 < 0) error("There was a problem accepting connections!\n");

					printf("Incoming client request on data port %d...\n", dataPort);

				    // Open the text file they asked for
				    printf("Attempting to open file %s...\n", filename);
					FILE *fp = fopen(filename, "r");
			        if (fp == NULL) error("There was an error opening the file.\n");

				    // Section by section, dump the text contents into the socket
			        while(1) {
			            unsigned char newbuf[256] = {0};
			            int bytesRead = fread(newbuf, 1, 256, fp);       

			            if(bytesRead > 0) {
			                printf("Transmitting chunk...\n");
			                write(acceptfd2, newbuf, bytesRead);
			            }

			            if (bytesRead < 256){
			                if (feof(fp)) printf("End of file reached.\n");
			                if (ferror(fp)) error("There was an error reading the file.\n");

			                break;
			            }
			        }

			        close(datafd);
			        close(acceptfd2);
			        fclose(fp);
			        sleep(1);

					printf("File sent.\n");
				}
		    }
		}
    }

	return 0;
}

void error(const char *msg) {
    printf("%s", msg);
    exit(1);
}

// Attribution: http://stackoverflow.com/questions/19206660/how-
// to-write-own-isnumber-function
bool isNumber(const char *str) {
	char *e = NULL;
	(void) strtol(str, &e, 0);
	return e != NULL && *e == (char)0;
}

// Attribution: http://www.linuxquestions.org/questions/programming-9/
// extract-substring-from-string-in-c-432620/
char* substring(const char* str, size_t begin, size_t len) { 
  	if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len)) 
    	return 0; 

  	return strndup(str + begin, len); 
}

// Attribution: http://stackoverflow.com/questions/2674312/how-to-
// append-strings-using-sprintf
int bytes_added(int result_of_sprintf) {
    return (result_of_sprintf > 0) ? result_of_sprintf : 0;
}
