#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <unistd.h>
#include <time.h>


int main(int argc, char *argv[]) {
  // Error handling
  if (argc < 2) { 
    printf("Please enter a number representing the desired key length\n"); 
    exit(0); 
  } 

  int keyLength = atoi(argv[1]);
  char key[256];
  srand(time(NULL));

  for (int i = 0; i < keyLength; ++i) {
    int min = 65; // ASCII value associated with 'A'
    int max = 90; // ASCII value assocaitd with 'Z'

    key[i] = min + rand() % (max - min + 1);
  }

  key[keyLength] = '\0';  // Null-terminate the string
  printf("%s", key);
  
  return 0;
}