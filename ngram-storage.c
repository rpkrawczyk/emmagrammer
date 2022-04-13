#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include "ngram-storage.h"

#define STORAGE_MAGIC "⚗ n-GRAM LOCAL STORAGE\x04"

uint64_t calc_max_index(int gram_max, int n) {
  int i;
  uint64_t maxindex = 1;

  for(i = 0; i < n; ++i) maxindex *= (gram_max + 1);
  return maxindex + 1;
}


static uint64_t calc_size(int gram_max, int n) {
  uint64_t size, maxindex;

  maxindex = calc_max_index(gram_max, n);
  size = sizeof(struct Ngram_Storage) + maxindex / 8;
  return size + 1;
}

uint8_t *ngram_from_string_into(ngram_storage_t *ngramstorage, const char *hextex, uint8_t *target) {
  int conv, n;
  unsigned int val;
  const char *end;
  int idx = 0;

  for(end = hextex + strlen(hextex); hextex < end; hextex += n) {
    conv = sscanf(hextex, " %x%n", &val, &n);
    if(conv == EOF || conv < 1) break;
    target[idx++] = val;
    if(idx >= ngramstorage->n) return target;
  }
  return NULL;
}


uint8_t *ngram_from_string(ngram_storage_t *ngramstorage, const char *hextex) {
  int conv, n;
  unsigned int val;
  const char *end;
  int idx = 0;

  for(end = hextex + strlen(hextex); hextex < end; hextex += n) {
    conv = sscanf(hextex, " %x%n", &val, &n);
    if(conv == EOF || conv < 1) break;
    //printf("%X\t %04X %04x\n", conv, val, n);
    ngramstorage->ngram_buffer[idx] = val;
    if(++idx >= MAX_NGRAM_BUFFER) break;
  }
  if(idx < ngramstorage->n) return NULL;
  return ngramstorage->ngram_buffer;
}


ngram_storage_t *open_ngram_storage(const char *fname) {
  FILE *f;
  struct Ngram_Storage *ptr = NULL;
  uint64_t maxindex, size;
  struct stat fstat;
  int err;
  int gram_max;
  int n;

  if(stat(fname, &fstat) != 0) {
    perror("open_ngram_storage(fstat)");
    return NULL;
  }
  f = fopen(fname, "r+");
  if(!f) return NULL; else {
    size = sizeof(ngram_storage_t);
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f), 0);
    if(ptr == MAP_FAILED) {
      perror("mmap");
      ptr = NULL;
    } else {
      assert(strlen(STORAGE_MAGIC) < sizeof(ptr->MAGIC));
      if(strcmp(STORAGE_MAGIC, ptr->MAGIC) != 0) { fprintf(stderr, "magic?\n"); errno = EINVAL; return NULL; }
      /* TODO: check versions major==, minor>= */
      gram_max = ptr->gram_max;
      n = ptr->n;
      //assert(fprintf(stderr, "%d %d\n", gram_max, n));
      maxindex = calc_max_index(gram_max, n);
      size = calc_size(gram_max, n);
      if(munmap(ptr, sizeof(ngram_storage_t)) != 0) perror("munmap");
      ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f), 0);
      assert(fprintf(stderr, "ptr = %p size = $%"PRIX64" ptr->SIZE = %"PRIX64"\n", ptr, size, ptr->SIZE));
      if(!ptr) goto errend;
      if(ptr->SIZE != size) { fprintf(stderr, "SIZE?\n"); errno = EINVAL; return NULL; }
      if(ptr->gram_max != gram_max) { fprintf(stderr, "gram_max?\n"); errno = EINVAL; return NULL; }
      if(ptr->n != n) { fprintf(stderr, "n?\n"); errno = EINVAL; return NULL; }
      if(ptr->maxindex != maxindex) { fprintf(stderr, "maxindex?\n"); errno = EINVAL; return NULL; }
      if(ptr->combinations != powl((long double)gram_max, (long double)n)) { fprintf(stderr, "combinations?\n"); errno = EINVAL; return NULL; }
      if(ptr->fstat.st_dev != fstat.st_dev) { fprintf(stderr, "Error! File was moved (dev).\n"); errno = EINVAL; return NULL; }
      if(ptr->fstat.st_ino != fstat.st_ino) { fprintf(stderr, "Error! File was moved (ino).\n"); errno = EINVAL; return NULL; }
      /* Do not use st_blocks as this may change (sparse files)! */
      if(ptr->fstat.st_size != fstat.st_size) { fprintf(stderr, "Error! File was moved (size).\n"); errno = EINVAL; return NULL; }
      
    } 
  }
 errend:
  err = errno;
  fclose(f);
  errno = err;
  return ptr;
}

ngram_storage_t *create_ngram_storage(const char *fname, int gram_max, int n) {
  FILE *f;
  struct Ngram_Storage *ptr;
  uint64_t size;
  uint64_t maxindex;

  maxindex = calc_max_index(gram_max, n);
  f = fopen(fname, "w+");
  if(!f) return NULL;
  size = calc_size(gram_max, n);
  if(ftruncate(fileno(f), size) != 0) perror("ftruncate");
  //fwrite(f, sizeof(FILE), 1, f);
  //The following flush is needed, otherwise file may not be truncated in time.
  fflush(f);
  assert(printf("%lX\n", (unsigned long int)size));
  ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f), 0);
  if(ptr == MAP_FAILED) {
    perror("mmap");
  } else {
    assert(printf("ptr = %p\n", ptr));
    strcpy(ptr->MAGIC, STORAGE_MAGIC);
    ptr->majorversion = NGRAMMAJORVERSION;
    ptr->minorversion = NGRAMMINORVERSION;
    ptr->SIZE = size;
    ptr->gram_max = gram_max;
    ptr->n = n;
    ptr->maxindex = maxindex;
    ptr->combinations = powl(gram_max, n);
  }
  fclose(f);
  if(stat(fname, &ptr->fstat) != 0) {
    perror("fstat");
  }
  msync(ptr, ptr->SIZE, MS_SYNC);
  return ptr;
}

void set_ngram(ngram_storage_t *ngramstorage, uint8_t *grams) {
  int i;
  uint64_t pos;

  pos = *grams;
  for(i = 1; i < ngramstorage->n; ++i) {
    assert(grams[i] <= ngramstorage->gram_max);
    pos *= (ngramstorage->gram_max + 1);
    pos += grams[i];
  }
  //assert(printf("%08lX %08lx %02x %x\n", (long)pos, (long)(pos >> 3), (int)(1 << (pos & 7)), (int)(pos & 7)));
  ngramstorage->bits[pos >> 3] |= 1 << (pos & 7);
  /* putchar('\t'); */
  /* for(i = 0; i < 16; ++i) printf(" $%02X", (int)ngramstorage->bits[i]); */
  /* putchar('\n'); */
}

int find_ngram(ngram_storage_t *ngramstorage, uint8_t *grams) {
  int i;
  uint64_t pos;

  pos = *grams;
  for(i = 1; i < ngramstorage->n; ++i) {
    assert(grams[i] <= ngramstorage->gram_max);
    pos *= (ngramstorage->gram_max + 1);
    pos += grams[i];
  }
  return ngramstorage->bits[pos >> 3] & (1 << (pos & 7));
}


void close_ngram_storage(ngram_storage_t *ngramstorage) {
  ngramstorage->counter--;
  msync(ngramstorage, ngramstorage->SIZE, MS_SYNC);
  munmap(ngramstorage, ngramstorage->SIZE);
}

double population_count(ngram_storage_t *ngramstorage) {
  uint64_t i;
  int v;
  unsigned long count = 0;

  for(i = 0; i < ngramstorage->maxindex / 8; ++i) {
    int a, b, c;
    v = ngramstorage->bits[i];
    a = (v & 0x55) + ((v >> 1) & 0x55);
    b = (a & 0x33) + ((a >> 2) & 0x33);
    c = (b & 0x0f) + ((b >> 4) & 0x0f);
    count += c;
  }
  return (double)count / i;
}

