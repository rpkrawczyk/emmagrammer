#include <inttypes.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <functional>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "fileformat.hh"

using namespace std;

typedef map<vector<uint8_t>,unsigned long int> Histogram_type;

Histogram_type create_histogram(unsigned int n, istream &inp) {
  string line;
  map<vector<uint8_t>,unsigned long int> histogram;
  
  auto read_line = bind(read_ngram_line, n, std::placeholders::_1);
  while(getline(cin, line)) {
    try {
      istringstream sin(line);
      vector<uint8_t> ngrams(read_line(sin));
      histogram[ngrams] += 1;
    }
    catch(std::exception &excp) {
      cerr << "Error in line '" << line << "': " << excp.what() << endl;
      throw;
    }
  }
  return histogram;
}

int main(int argc, char **argv) {
  auto header(read_header(cin));
  if(header.at("type") == "n-grams") {
    header["type"] = "n-grams histogram";
    Histogram_type histogram(create_histogram(boost::lexical_cast<unsigned int>(header.at("n")), cin));
    for(auto i : header) {
      cout << '#' << i.first << ": " << i.second << '\n';
    }
    cout << endl;
    for(auto i : histogram) {
      cout << hex;
      for(auto j : i.first) cout << ' ' << setw(2) << static_cast<unsigned int>(j);
      cout << boost::format("\t $%06lX %8ld\n") % i.second % i.second;
    }
    cerr << "Histogram entries " << histogram.size() << endl;
  } else {
    cerr << "Unknown input type!\n";
    return 1;
  }
  return 0;
}
