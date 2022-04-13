#include "ngram-storage.h"
#include <stdio.h>

int main(int argc, char **argv) {
  ngram_storage_t *storage;
  const char *fname;
  uint8_t grambuf[12];
  uint8_t *gramptr;

  if(argc == 2) fname = argv[1]; else fname = "STORAGE";
  storage = open_ngram_storage(fname);
  if(storage == NULL) {
    perror("storage(r+)");
    storage = create_ngram_storage(fname, 98, 3);
  }    
  if(!storage) {
    perror("create_ngram_storage");
    return 1;
  }
  printf("storage = %p\n", storage);
  printf("combinations = %30.20Le\n", storage->combinations);
  set_ngram(storage, (uint8_t*)"\x00\x00\x01");
  set_ngram(storage, (uint8_t*)"\x00\x00\x02");
  set_ngram(storage, (uint8_t*)"\x00\x00\x03");
  set_ngram(storage, (uint8_t*)"\x00\x00\x04");
  set_ngram(storage, (uint8_t*)"\x00\x00\x05");
  set_ngram(storage, (uint8_t*)"\x00\x00\x06");
  set_ngram(storage, (uint8_t*)"\x00\x00\x07");
  set_ngram(storage, (uint8_t*)"\x00\x00\x08");
  set_ngram(storage, (uint8_t*)"\x00\x00\x09");
  set_ngram(storage, (uint8_t*)"\x00\x00\x0a");
  set_ngram(storage, (uint8_t*)"\x00\x00\x0b");
  set_ngram(storage, (uint8_t*)"\x00\x01\x00");
  set_ngram(storage, (uint8_t*)"\x01\x00\x00");
  grambuf[0] = 0;
  grambuf[1] = 0;
  grambuf[2] = 0;
  set_ngram(storage, grambuf);
  grambuf[0] = 98;
  grambuf[1] = 98;
  grambuf[2] = 98;
  set_ngram(storage, grambuf);

  set_ngram(storage, (uint8_t*)"\x01\x00\x06");
  printf("%x, %x\n", find_ngram(storage, (uint8_t*)"\x1\0\x6"), find_ngram(storage, (uint8_t*)"\x1\0\x5"));
  printf("%x\n", find_ngram(storage, (uint8_t*)"\x0\0\x0"));
  printf("%x\n", find_ngram(storage, (uint8_t*)"\x30\x3a\x61"));
  gramptr = ngram_from_string(storage, "0a dd 2e 99    de AD");
  if(!gramptr) puts("conversion failed");
  gramptr = ngram_from_string(storage, "0a  AD");
  if(!gramptr) puts("conversion failed");
  close_ngram_storage(storage);
  return 0;
}
