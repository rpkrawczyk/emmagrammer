#include "histogram.h"
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <vector>

using namespace std;

string read_stream(istream &in) {
  string s;
  char ch;

  while(in.get(ch)) s += ch;
  return s;
}

string read_file(const char *fname) {
  ifstream ifs(fname);
  return read_stream(ifs);
}

int main(int argc, char **argv) {
  int i;
  
  if(argc != 2) {
    cerr << "usage: simple-histogram <file>\n";
    return 1;
  }
  try {
    string data(read_file(argv[1]));
    vector<double> hist(256);
    double max = simple_histogram(reinterpret_cast<unsigned char*>(&data[0]), data.size(), &hist[0]);
    cerr << "max = " << max << endl;
    for(i = 0; i < 256; ++i) {
      printf(" %20.16E", hist[i]);
      //cerr << i << ' ' << hist[i] << endl;
    }
    cout << endl;
  }
  catch(const std::exception &excp) {
    cerr << "Error! Exception: " << excp.what() << endl;
    return -1;
  }
  return 0;
}
