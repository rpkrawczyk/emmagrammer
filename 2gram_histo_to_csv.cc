#include <cstring>
#include <iostream>
#include <algorithm>
#include <functional>
#include <sstream>
#include "fileformat.hh"

class Entry {
  Header_type header;
  unsigned long histogram[256][256];

  void clear() {
    memset(histogram, 0, sizeof(histogram));
  }
public:
  Entry() {
    clear();
  }
  Entry(const Header_type &h) : header(h) {
    clear();
  }
  unsigned long &operator()(int x, int y) {
    return histogram[x][y];
  }
  unsigned long operator()(int x, int y) const {
    return histogram[x][y];
  }
  const Header_type &get_header() const { return header; }
};

Entry reader_fun(const char *fname) {
  std::ifstream in(fname);
  unsigned long count;
  char ch;
  std::string line;

  Entry entry(read_header(in));
  while(getline(in, line)) {
    try {
      std::istringstream sin(line);
      std::vector<uint8_t> two_gram(read_ngram_line(2, sin));
      sin >> ch >> std::hex >> count;
      entry(two_gram.at(0), two_gram.at(1)) = count;
      //std::cout << ch << two_gram.at(0) << ' ' << two_gram.at(1) << '\t' << count << std::endl;
    }
    catch(std::exception &excp) {
      std::cerr << "Error in line '" << line << "': " << excp.what() << std::endl;
      throw;
    }
  };
  return entry;
}

std::ostream &writer_fun(std::ostream &out, const Entry &entry) {
  std::cout << entry.get_header().at("fname");
  for(int y = 0; y < 256; ++y) {
    for(int x = 0; x < 256; ++x) {
      std::cout << '\t' << entry(x, y);
    }
  }
  std::cout << std::endl;
  return out;
};

int main(int argc, char **argv) {
  if(argc < 2) {
    std::cerr << "2gram_histo_to_csv <files...>\n";
    return 1;
  }
  std::vector<Entry> entries(argc - 1);
  std::transform(&argv[1], &argv[argc], entries.begin(), &reader_fun);
  std::for_each(entries.begin(), entries.end(), std::bind(writer_fun, std::ref(std::cout), std::placeholders::_1));
  return 0;
}
