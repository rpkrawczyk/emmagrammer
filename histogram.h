#ifndef __HISTOGRAM_H_20161013_
#define __HISTOGRAM_H_20161013_
#include <inttypes.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*! \brief Calculate the histogram of the byte values.
   *
   * This function takes an array of unsigned chars and calculates the
   * normalised histogram frequencies of the bytes found in the
   * buffer. It returns the highest value before normalisation. On
   * error -1 is returned.
   *
   * If no pointer to an array for the storage of the histogram
   * frequencies is given (it is NULL) then an array is allocated and
   * must be freed using free().
   *
   * \param buf pointer to array of bytes
   * \param len length of the array
   * \param hist pointer to an array of 256 doubles or NULL
   * \return maximum value in the histogram or -1 on error
   */
  double simple_histogram(unsigned char *buf, size_t len, double *hist);

#ifdef __cplusplus
};
#endif
#endif
