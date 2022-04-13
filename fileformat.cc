#include "fileformat.hh"

using namespace std;

map<string,string> read_header(istream &in) {
  string line;
  map<string,string> keyval;
  
  while(getline(in, line)) {
    if(line.size() == 0) break;
    if(line[0] == '#') {
      string::size_type pos = line.find(": ");
      if(pos != string::npos) {
	keyval[line.substr(1, pos - 1)] = line.substr(pos + 2);
      }
    }
  }
  return keyval;
}

std::vector<uint8_t> read_ngram_line(unsigned int n, std::istream &in) {
  vector<uint8_t> ngrams;
  int x;

  while(n-- > 0) {
    //in >> skipws >> hex >> noshowbase >> x;
    in >> std::hex >> x;
    if(!in) throw runtime_error("read n-gram failed");
    ngrams.push_back(x & 0xFF);
  }
  return ngrams;
}

