#include "common.h"
#include "charlist.h"

void help();

int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    help();
    return 0;
  }
  for (i = 1; i < argc; i++) {
    int length;
    unsigned char *buffer = decode_string_to_bytes(argv[i], &length);
    dump_character_data(buffer, length);
    free(buffer);
  }

  return 0;
}

void help() {
  printf("Convert a string to a text-graphic representation of a character.\n");
}
