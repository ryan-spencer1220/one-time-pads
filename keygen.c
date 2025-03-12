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

  // Allocate memory for the key based on the desired length
  char *key = (char *)malloc(keyLength + 1);
  if (key == NULL) {
    perror("Failed to allocate memory for the key");
    exit(1);
  }

  srand(time(NULL));

  for (int i = 0; i < keyLength; ++i) {
    int min = 65; // ASCII value associated with 'A'
    int max = 90; // ASCII value associated with 'Z'

    key[i] = min + rand() % (max - min + 1);
  }

  key[keyLength] = '\0';
  printf("%s\n", key);

  free(key);
  return 0;
}
