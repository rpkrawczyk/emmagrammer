#ifndef __FILEFORMAT_HH_2016__
#define __FILEFORMAT_HH_2016__
#include <inttypes.h>
#include <map>
#include <fstream>
#include <string>
#include <vector>

typedef std::map<std::string,std::string> Header_type;

/*! \brief read header from text file
 *
 * File format is not finalised yet. 
 *
 * \param in input file stream
 * \return a dictionary of key value pairs.
 */
Header_type read_header(std::istream &in);

std::vector<uint8_t> read_ngram_line(unsigned int n, std::istream &in);

#endif
