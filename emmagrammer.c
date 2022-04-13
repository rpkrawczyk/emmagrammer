#include "ngram-storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

enum Error_Codes {
  ERROR_CLI_PARAM = 1,
  ERROR_IO
};

ngram_storage_t *storage;
const char *fname;


void usage(int st) {
  fprintf(stderr, "Usage: ...\n");
  exit(st);
}


int command_store(int argc, char **argv) {
  uint8_t *ngptr;
  int i;
  char buf[1 << 11];
  uint8_t ngrambuf[1 << 12];

  storage = open_ngram_storage(fname);
  printf("combinations = %30.20Le\n", storage->combinations);
  if(storage == NULL) {
    perror("open storage");
    return ERROR_IO;
  }
  if(argc == 3) {
    ngptr = ngram_from_string(storage, argv[2]);
    printf("%p\n", ngptr);
    if(ngptr) {
      for(i = 0; i < storage->n; ++i) {
	printf(" %02x", ngptr[i]);
      }
      putchar('\n');
      set_ngram(storage, ngptr);
      close_ngram_storage(storage);
    }
  } else if(argc == 2) {
    while(!feof(stdin)) {
      if(fgets(buf, sizeof(buf), stdin) > 0) {
	ngptr = ngram_from_string_into(storage, buf, ngrambuf);
	if(ngptr) {
	  for(i = 0; i < storage->n; ++i) {
	    printf(" %02x", ngptr[i]);
	  }
	  putchar('\n');
	  set_ngram(storage, ngptr);
	} else {
	  fprintf(stderr, "Can not interpret n-gram line: '%s'!\n", buf);
	}
      }
    }
    close_ngram_storage(storage);
  } else usage(ERROR_CLI_PARAM);
  return 0;
}


int command_recall(int argc, char **argv) {
  uint8_t *ngptr;
  int i;
  char buf[1 << 11];
  long counter = 0;
  long found = 0;

  storage = open_ngram_storage(fname);
  if(storage == NULL) {
    perror("open storage");
    return ERROR_IO;
  }
  printf("combinations = %30.20Le\n", storage->combinations);
  if(argc == 3) {
    ngptr = ngram_from_string(storage, argv[2]);
    //printf("%p\n", ngptr);
    if(ngptr) {
      for(i = 0; i < storage->n; ++i) {
	printf(" %02x", ngptr[i]);
      }
      printf("\t %d\n", find_ngram(storage, ngptr));
    }
  } else if(argc == 2) {
    while(!feof(stdin)) {
      if(fgets(buf, sizeof(buf), stdin) > 0) {
	ngptr = ngram_from_string(storage, buf);
	//printf("%p\n", ngptr);
	if(ngptr) {
	  for(i = 0; i < storage->n; ++i) {
	    printf(" %02x", ngptr[i]);
	  }
	  i = find_ngram(storage, ngptr);
	  if(i != 0) found++;
	  printf("\t %d", i);
	  if(1) {
	    printf("\t | ");
	    for(i = 0; i < storage->n; ++i) {
	      if(ngptr[i] >= 0x20 && ngptr[i] < 0x7f) putchar(ngptr[i]); else putchar('.');
	    }
	  }
	  putchar('\n');
	}
	++counter;
      }
    }
    fprintf(stderr, "%08lx/%08lx %e\n", found, counter, (double)found / counter);
  } else usage(ERROR_CLI_PARAM);
  return 0;
}


int command_ngramify(int argc, char **argv) {
  int buf[32];
  int i;
  unsigned int skip = 0;

  storage = open_ngram_storage(fname);
  if(storage == NULL) {
    perror("open storage");
    return ERROR_IO;
  }
  for(i = 0; i < storage->n; ++i) buf[i] = fgetc(stdin);
  while(!feof(stdin)) {
    for(i = 0; i < storage->n; ++i) printf(" %02X", buf[(i + skip) % storage->n]);
    putchar('\n');
    if((buf[skip++ % storage->n] = fgetc(stdin)) == -1) break;
  }
  return 0;
}



int command_foltran(int argc, char **argv) {
  int i, j;
  int x, y;
  uint8_t fold_and_transform_table[256];
  uint8_t *ftable;
  int tpos = 0;

  if(argc == 2) {
    usage(ERROR_CLI_PARAM);    
  } else {
    ftable = fold_and_transform_table;
    storage = open_ngram_storage(fname);
    if(storage != NULL) {
      ftable = storage->last_fold_tranform_table;
    }
    for(i = 2; i < argc; ++i) {
      if(sscanf(argv[i], " %x - %x", &x, &y) == 2) {
	for(j = x; j <= y; ++j) {
	  if(j < 0 || j >= 256) {
	    fprintf(stderr, "Illegal value %02X.\n", j);
	    exit(ERROR_CLI_PARAM);
	  }
	  ftable[j] = tpos++;
	  if(tpos >= 256) goto follies_end;
	}
      } else if(sscanf(argv[i], " %x ~ %x", &x, &y) == 2) {
	for(j = x; j <= y; ++j) {
	  if(j < 0 || j >= 256) {
	    fprintf(stderr, "Illegal value %02X.\n", j);
	    exit(ERROR_CLI_PARAM);
	  }
	  ftable[j] = tpos;
	  if(tpos >= 256) goto follies_end;
	}
	++tpos;
      } else {
	fprintf(stderr, "Error at: %s\n", argv[i]);
	break;
      }
    }
  }
 follies_end:
  /* for(i = 0; i < 256; ++i) { */
  /*   fprintf(stderr, " %02X%c", ftable[i], ((i + 1) & 0x0f) == 0 ? '\n' : ' '); */
  /* } */
  while(!feof(stdin)) {
    i = getchar();
    if(i == EOF) break;
    putchar(ftable[i]);
  }
  return 0;
}


int command_create(int argc, char **argv) {
  int i, j;

  if(argc != 4) usage(ERROR_CLI_PARAM);
  i = atoi(argv[2]);
  if(i < 1 || i > 255) {
    fprintf(stderr, "gram_max not in [1..255]\n");
    return ERROR_CLI_PARAM;
  }
  j = atoi(argv[3]);
  if(j < 1 || j > 16) {
    fprintf(stderr, "n not in [1..16] (for now)\n");
    return ERROR_CLI_PARAM;
  }
  storage = create_ngram_storage(fname, i, j);
  if(!storage) {
    perror("create_ngram_storage");
    return ERROR_IO;
  }
  return 0;
}


int main(int argc, char **argv) {
  if(argc == 1) {
    usage(ERROR_CLI_PARAM);
  }
  fname = getenv("NGRAM_STORAGE");
  fname = fname == NULL ? DEFAULT_STORAGE_FILENAME : strdup(fname);
  if(strcmp(argv[1], "create") == 0) {
    return command_create(argc, argv);
  } else if(strcmp(argv[1], "store") == 0) {
    return command_store(argc, argv);
  } else if(strcmp(argv[1], "recall") == 0) {
    return command_recall(argc, argv);
  } else if(strcmp(argv[1], "ngramify") == 0) {
    return command_ngramify(argc, argv);
  } else if(strcmp(argv[1], "foltran") == 0) {
    return command_foltran(argc, argv);
  } else {
    fprintf(stderr, "Unknown command '%s'.\n", argv[1]);
    return 1;
  }
  return -1;
}
