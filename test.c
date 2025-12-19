#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>

void _rm_space(char* read) {
  char* write = read;
  while (*read) {
    if (*read != ' ') {
      *write++ = *read;
    }
    read++;
  }
  *write = '\0';
}

int main() {
  char t[] = "h  el l  o 11    67world .";
  _rm_space(t);
  printf("%d\n", sizeof("hello"));
  return 0;
}
