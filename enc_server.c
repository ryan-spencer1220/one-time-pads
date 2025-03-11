#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

        printf("SERVER: Connected to client running at host %d port %d\n",
                ntohs(clientAddress.sin_addr.s_addr),
                ntohs(clientAddress.sin_port));

        // Get the message from the client and display it
        memset(buffer, '\0', 256);

        // Read the client's message from the socket
        charsRead = recv(connectionSocket, buffer, 255, 0);
        if (charsRead < 0){
            error("ERROR reading from socket");
        }
        printf("SERVER: I received this: \"%s\"\n", buffer);

        // Break message into plaintext and key
        char *token;
        char message[4096];
        char key[4096] = "";
        char encKey[4096] = "";

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

        for(int i = 0; i < strlen(message); ++i){
          // convert ascii values to alphabet index values (A = 0, B = 1, etc.)
          int currMsg = message[i] - 65;
          int currKey = key[i] - 65;
          int total = currMsg + currKey;

          // if total ascii value exceeds 26, wrap around to beginning of alphabet
          if (total > 26){
            total -= 26;
          }

          // convert ASCII value and append to encrypted message
          char temp[10];
          sprintf(temp, "%d", total); 
          strcat(encKey, temp); 
        }

        // Print message and key broken up
        printf("SERVER: THIS IS THE MESSAGE: %s\n", message);
        printf("SERVER: THIS IS THE KEY: %s\n", key);
        printf("SERVER: THIS IS THE Encrypted Message: %s\n", encKey);

        // Send a Success message back to the client
        charsRead = send(connectionSocket,
                         "I am the server, and I got your message", 39, 0);
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
