%module emmagrammer
%{
#include "ngram-storage.h"
%}

#include "ngram-storage.h"

ngram_storage_t *open_ngram_storage(const char *fname);
ngram_storage_t *create_ngram_storage(const char *fname, int gram_max, int n);

void set_ngram(ngram_storage_t *ngramstorage, uint8_t *grams);
int find_ngram(ngram_storage_t *ngramstorage, uint8_t *grams);
void close_ngram_storage(ngram_storage_t *ngramstorage);
uint8_t *ngram_from_string(ngram_storage_t *ngramstorage, const char *hextex);
double population_count(ngram_storage_t *ngramstorage);
