#include "common.h"
#include "charutils.h"

void help();

int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    help();
    return 0;
  }
  for (i = 1; i < argc; i++) {
    dump_code(argv[i]);
  }

  return 0;
}

void help() {
  printf("Convert a string to a text-graphic representation of a character.\n");
}
