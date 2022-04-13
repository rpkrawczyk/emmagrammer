#include "histogram.h"
#include <assert.h>
#include <stdlib.h>

static double simple_histogram_internal(unsigned char *buf, size_t len, double *hist) {
  double max;
  size_t i;

  assert(hist != NULL);
  for(i = 0; i < 256; ++i) hist[i] = 0;
  for(i = 0; i < len; ++i) hist[buf[i]]++;
  max = 0;
  for(i = 0; i < 256; ++i) {
    if(hist[i] > max) max = hist[i];
  }
  if(max > 0) {
    for(i = 0; i < 256; ++i) hist[i] /= max;
  }
  return max;
}


double simple_histogram(unsigned char *buf, size_t len, double *hist) {
  if(hist == NULL) {
    hist = malloc(sizeof(double) * 256);
    if(!hist) return -1;
  }
  return simple_histogram_internal(buf, len, hist);
}
