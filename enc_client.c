#include <netdb.h>      // gethostbyname()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // send(),recv()
#include <sys/types.h>  // ssize_t
#include <unistd.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arguments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, charsWritten, charsRead;
  struct sockaddr_in serverAddress;

  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[3]); 
    exit(0); 
  }

  FILE *messageFile = fopen(argv[1], "r");
  if (messageFile == NULL) {
      perror("Error opening text message file");
      return 1;
  }

  FILE *keyFile = fopen(argv[2], "r");
  if (keyFile == NULL) {
      perror("Error opening encryption key file");
      return 1;
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  char message[4096] = "";
  char messageBuffer[256];
  while (fgets(messageBuffer, sizeof(messageBuffer), messageFile) != NULL) {
      strcat(message, messageBuffer);
  }

  char key[4096] = "";
  char keyBuffer[256];
  while (fgets(keyBuffer, sizeof(keyBuffer), keyFile) != NULL) {
      strcat(key, keyBuffer);
  }

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, messageBuffer, strlen(messageBuffer), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(messageBuffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  memset(messageBuffer, '\0', sizeof(messageBuffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, messageBuffer, sizeof(messageBuffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  printf("CLIENT: I received this from the server: \"%s\"\n", messageBuffer);

  close(socketFD); 
  return 0;
}