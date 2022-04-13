#ifndef __NGRAMSTORE_2014_H__
#define __NGRAMSTORE_2014_H__
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NGRAM_BUFFER 0x1000
#define DEFAULT_STORAGE_FILENAME "N-GRAM_STORAGE"
#define NGRAMMAJORVERSION 1
#define NGRAMMINORVERSION 3

typedef struct Ngram_Storage {
  union {
    uint8_t header[1L << 16];
    struct {
      char MAGIC[29];
      unsigned int majorversion;
      unsigned short minorversion;
      int gram_max; //!< currently using uint8_t, so maximum is 255 anyway
      int n;
      uint64_t maxindex;
      uint64_t SIZE;
      long double combinations;
      struct stat fstat;
      unsigned long counter;
    };
  };
  uint8_t ngram_buffer[MAX_NGRAM_BUFFER];
  uint8_t last_fold_tranform_table[256];
  uint8_t bits[];
} ngram_storage_t;

ngram_storage_t *open_ngram_storage(const char *fname);
ngram_storage_t *create_ngram_storage(const char *fname, int gram_max, int n);

void set_ngram(ngram_storage_t *ngramstorage, uint8_t *grams);
int find_ngram(ngram_storage_t *ngramstorage, uint8_t *grams);
void close_ngram_storage(ngram_storage_t *ngramstorage);
uint8_t *ngram_from_string(ngram_storage_t *ngramstorage, const char *hextex);
/*! \brief read values from string and write into target array
 *
 * Reads space separated hex values and writes them into the target
 * array. The array must have space for at least n elements.
 *
 * \param ngramstorage pointer to the storage data-structure
 * \param hextex string with hex values
 * \param target pointer to a buffer
 * \return pointer to taget on success, NULL on failure
 */
uint8_t *ngram_from_string_into(ngram_storage_t *ngramstorage, const char *hextex, uint8_t *target);
double population_count(ngram_storage_t *ngramstorage);

#ifdef __cplusplus
};
#endif

#endif
