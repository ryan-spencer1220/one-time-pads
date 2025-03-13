#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void decrypt(char *message, char *key, char *plaintext) {
  int length = strlen(message);
  int keyLength = strlen(key);
  int keyIndex = 0;

  for (int i = 0; i < length; ++i) {
    if (message[i] == '\n') {  
      plaintext[i] = '\0';  // Properly terminate the string
      break;
    } 
    if (message[i] == ' ') {  
      // Preserve spaces in decryption
      plaintext[i] = ' ';
    } else if (isalpha(message[i])) {
        // Find the next alphabetic character in the key
        while (!isalpha(key[keyIndex % keyLength])) {
          keyIndex++;
        }

        int currMsg = toupper(message[i]) - 65;
        int currKey = toupper(key[keyIndex % keyLength]) - 65;
        int total = currMsg - currKey;

        // if total ascii value is less than 0, wrap around to end of alphabet
        if (total < 0) {
          total += 26;
        }

        // convert ASCII value and append to decrypted message
        plaintext[i] = (total % 26) + 65;
        keyIndex++;
    } else {
        // Preserve non-alphabetic characters
        plaintext[i] = message[i];
    }
  }
  plaintext[length] = '\0'; // Null-terminate
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber){
    memset((char*) address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
    int connectionSocket, charsRead;
    char buffer[256];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    while(1){
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);
        if (connectionSocket < 0){
            error("ERROR on accept");
        }

        // Get the message from the client and display it
        memset(buffer, '\0', 256);

        // Read the client's message from the socket
        charsRead = recv(connectionSocket, buffer, 255, 0);
        if (charsRead < 0){
            error("ERROR reading from socket");
        }

        // Break message into plaintext and key
        char *token;
        char message[4096];
        char key[4096] = "";
        char decKey[4096] = "";

        // Initialize message as an empty string
        message[0] = '\0';

        // Split the string by '+' and store the parts
        token = strtok(buffer, "+");
        if (token != NULL) {
            strcpy(message, token);  // First token is the message
            token = strtok(NULL, "+"); // Get the next token (key)
            if (token != NULL) {
                strcpy(key, token);  // Second token is the key
            }
        }

        decrypt(message, key, decKey);

        // Send a Success message back to the client
        charsRead = send(connectionSocket, decKey, 39, 0);
        if (charsRead < 0){
            error("ERROR writing to socket");
        }

        // Close the connection socket for this client
        close(connectionSocket);
    }

    // Close the listening socket
    close(listenSocket);
    return 0;
}
