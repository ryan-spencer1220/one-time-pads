#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
    error("CLIENT: ERROR, no such host\n"); 
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

  // Check for valid key file
  if(strlen(message) > strlen(key)){
    fprintf(stderr, "Error: key %s is too short\n", argv[2]);
    exit(1);
  }

  // Check for valid message
  for(int i = 0; i < strlen(message); ++i){
    if ((message[i] < 'A' || message[i] > 'Z') && message[i] != ' ' && message[i] != '\n') {
      error("enc_client error: message contains bad characters\n");
      exit(1);
    }
  }

  // Check for valid key
  for(int i = 0; i < strlen(key); ++i){
    if ((key[i] < 'A' || key[i] > 'Z') && key[i] != ' ' && key[i] != '\n') {
      error("enc_client error: key contains bad characters\n");
      exit(1);
    }
  }

  strcat(messageBuffer, "+");
  strcat(messageBuffer, keyBuffer);

  // Send message to server
  charsWritten = send(socketFD, messageBuffer, strlen(messageBuffer), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }

  if (charsWritten < strlen(messageBuffer)){
    error("CLIENT: WARNING: Not all data written to socket!\n");
  }

  memset(messageBuffer, '\0', sizeof(messageBuffer));
  charsRead = recv(socketFD, messageBuffer, sizeof(messageBuffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }

  // print message to output file
  printf("%s\n", messageBuffer);

  close(socketFD); 
  return 0;
}