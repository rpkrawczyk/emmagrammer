#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <iostream>

#define MAX_N_GRAM 1024

using namespace std;


void print_header(int n, const char *fname) {
  cout << "#type: n-grams\n";
  cout << "#n: " << n << endl;
  if(fname != NULL) {
    cout << "#fname: " << fname << endl;
  }
  cout << endl;
}


int ngramify(int n, FILE *fin, bool verbose) {
  int buf[MAX_N_GRAM];
  int i;
  unsigned int skip = 0;
  char ch;

  assert(n < MAX_N_GRAM);
  for(i = 0; i < n; ++i) buf[i] = fgetc(fin);
  while(!feof(fin)) {
    for(i = 0; i < n; ++i) printf(" %02X", buf[(i + skip) % n]);
    if(verbose) {
      printf("\t| ");
      for(i = 0; i < n; ++i) {
	ch = buf[(i + skip) % n];
	if(!isprint(ch)) ch = '.';
	putchar(ch);
      }
    }
    putchar('\n');
    if((buf[skip++ % n] = fgetc(fin)) == -1) break;
  }
  return 0;
}


int main(int argc, char **argv) {
  int n = -1;
  const char *fname = NULL;
  bool verbose = false;
  FILE *fin;
  int opt;
  
  while ((opt = getopt(argc, argv, "n:v")) != -1) {
    switch (opt) {
    case 'n':
      n = atoi(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s  [-n n-gram] [-v] [<name>]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if(optind < argc) fname = argv[optind++];
  if(n < 1) {
    fprintf(stderr, "You need to provide a value for n.\n");
    exit(EXIT_FAILURE);
  } else if(n >= MAX_N_GRAM) {
    fprintf(stderr, "Maximum n-gram value is %d!\n", MAX_N_GRAM - 1);
  }
  if(fname != NULL) {
    fin = fopen(fname, "r");
    if(!fin) {
      perror("Error! Can not open file");
      exit(EXIT_FAILURE);
    }
  } else {
    fin = stdin;
  }
  print_header(n, fname);
  ngramify(n, fin, verbose);
  if(fin != stdin) {
    if(fclose(fin) == EOF) {
      perror("Failure on closing file:");
    }
  }
  return 0;
}
