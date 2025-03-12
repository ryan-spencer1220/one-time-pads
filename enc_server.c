#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void encrypt(char *message, char *key, char *ciphertext) {
  int length = strlen(message);
  for (int i = 0; i < length; ++i){
    // convert ascii values to alphabet index values (A = 0, B = 1, etc.)
    if (message[i] == '\n') {  
      break;
    } 
    if (message[i] == ' ') {  
      // Preserve spaces in encryption
      ciphertext[i] = ' ';
    } else {
        int currMsg = message[i] - 65;
        int currKey = key[i] - 65;
        int total = currMsg + currKey;

        // if total ascii value exceeds 26, wrap around to beginning of alphabet
        if (total > 26) {
          total -= 26;
        }

        // convert ASCII value and append to encrypted message
        ciphertext[i] = (total % 26) + 65;
    }
  }
  ciphertext[length] = '\0'; // Null-terminate
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

int main(int argc, char *argv[]) {
    int connectionSocket, charsRead;
    char buffer[BUFFER_SIZE];
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
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    while (1) {
        // Accept a connection request
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }

        // Clear buffer before receiving
        memset(buffer, '\0', sizeof(buffer));

        // Receive data from the client
        charsRead = recv(connectionSocket, buffer, BUFFER_SIZE - 1, 0);
        if (charsRead < 0) {
            error("ERROR reading from socket");
        } else if (charsRead == 0) {
            printf("Client disconnected.\n");
            close(connectionSocket);
            continue;
        }

        buffer[charsRead] = '\0'; // Null-terminate received data

        // Parse message and key
        char message[BUFFER_SIZE], key[BUFFER_SIZE], encKey[BUFFER_SIZE];
        memset(message, '\0', sizeof(message));
        memset(key, '\0', sizeof(key));
        memset(encKey, '\0', sizeof(encKey));

        char *token = strtok(buffer, "+");
        if (token != NULL) {
            strcpy(message, token);
            token = strtok(NULL, "+");
            if (token != NULL) {
                strcpy(key, token);
            }
        }

        encrypt(message, key, encKey);

        // Send encrypted message back to client
        charsRead = send(connectionSocket, encKey, strlen(encKey), 0);
        if (charsRead < 0) {
            error("ERROR writing to socket");
        }

        close(connectionSocket);  // Close connection after processing
    }

    close(listenSocket);  // Never reached in normal execution
    return 0;
}
