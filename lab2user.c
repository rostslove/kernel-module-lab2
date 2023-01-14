#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
  unsigned int pid;
  FILE *file = fopen("/sys/kernel/debug/lab2/lab2file", "r+");
  if (file == NULL) {
    printf("No file\n");
    return 0;
  }
  if (sscanf(argv[1], "%x", &pid)) {
    char *buffer[BUFFER_SIZE];
    fprintf(file, "pid: %x", pid);
    while (!feof(file)) {
      char *result = fgets(buffer, BUFFER_SIZE, file);
      printf(result);
    }
  } else {
      printf("Invalid input");	 
  }
  fclose(file);
  return 0;
}
