/*! \brief simichunks: find similar chunks
 *
 * Find similar (n-gram) chunks in several files.
 */
#include <getopt.h>
#include <cstring>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <set>
#include <memory>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <json/json.h>


typedef std::vector<std::string> ChunkSet;


struct CLIParams {
  unsigned int n;
  int optindex;
  std::vector<const char *> filenames;
  std::string chunkstorage;
  std::string subtract_chunks;
};

static CLIParams cli_parse(int argc, char **argv) {
  int c;
  int names_size;
  static struct option long_options[] = {
    { "n-gram-size", required_argument, 0, 'n' },
    { "chunk-storage", required_argument, 0, 'S' },
    { "subtract-chunks", required_argument, 0, 's' },
    { 0, 0, 0, 0 }
  };
  CLIParams cli_pars = {
    5
  };

  while(true) {
    c = getopt_long(argc, argv, "n:S:s:", long_options, &cli_pars.optindex);
    if(c == -1) {
      break;
    }
    switch(c) {
    case 'n':
      cli_pars.n = boost::lexical_cast<unsigned int>(optarg);
      break;
    case 'S':
      cli_pars.chunkstorage = optarg;
      break;
    case 's':
      cli_pars.subtract_chunks = optarg;
      break;
    default:
      fprintf(stderr, "?? getopt returned character code $%02x ??\n", c);
    }
  }
  cli_pars.optindex = optind;
  names_size = argc - cli_pars.optindex;
  if(names_size > 0) {
    cli_pars.filenames.resize(names_size);
    std::copy(argv + cli_pars.optindex, argv + argc, cli_pars.filenames.begin());
  }
  return cli_pars;
}

class Chunker {
public:
  class const_iterator {
  public:
    // Apparently we need all of the following types to be a correct
    // forward Iterator. Too much boilerplate...
    typedef const_iterator self_type;
    typedef std::string value_type;
    typedef std::string& reference;
    typedef std::string const_reference;
    typedef std::string* pointer;
    typedef long difference_type;
    typedef std::forward_iterator_tag iterator_category;
    const_iterator(const Chunker &chunker_, unsigned long pos_) : chunker(chunker_), pos(pos_) { }
    self_type operator++() { self_type i = *this; pos++; return i; }
    self_type operator++(int junk) { pos++; return *this; }
    // This is *not* a const reference as we use a local
    // temporary. STL algorithms seem to be fine with it.
    std::string operator*() const {
      return chunker.data->substr(pos, chunker.n_gram);
    }
    //const pointer operator->() { return &(chunker.data->substr(pos, n_gram)); }
    //bool operator==(const self_type& rhs) const { return pos == rhs.pos; }
    bool operator!=(const self_type& rhs) const { return pos != rhs.pos; }
  private:
    std::string argl;
    const Chunker &chunker;
    unsigned long pos;
  };

private:
  unsigned long pos;
  std::shared_ptr<std::string> data;
  unsigned int n_gram;
public:
  Chunker(const char *fname, unsigned int n) : pos(0), data(NULL), n_gram(n) {
    //Using C-style input output as C++ really sucks on binary files!
    FILE *fin;
    struct stat buf;
    std::string error;
    size_t bread; //number of bytes read

    data = std::shared_ptr<std::string>(new std::string);
    if(stat(fname, &buf) == -1) {
      error = std::strerror(errno);
      throw std::runtime_error(error);
    } else {
      if(!S_ISREG(buf.st_mode)) {
	std::ostringstream out;
	out << '\'' << fname << "' is not a regular file";
	throw std::runtime_error(out.str());
      }
    }
    if(!(fin = fopen(fname, "rb"))) {
      error = std::strerror(errno);
      throw std::runtime_error(error);
    }
    data->resize(buf.st_size);
    bread = fread(&(*data)[0], 1, buf.st_size, fin);
    if(bread != static_cast<size_t>(buf.st_size)) {
      throw std::runtime_error(str(boost::format("not all data (%06lx != %06lx) could be read") % bread % buf.st_size));
    }
    fclose(fin);
  }
  const_iterator begin() const { return const_iterator(*this, 0); }
  const_iterator end() const {
    // Chunker ret(*this);
    // ret.pos = data->size() - n_gram + 1;
    // return ret;
    return const_iterator(*this, data->size() - n_gram + 1);
  }
  Chunker operator++() {
    ++pos;
    return *this;
  }
  // Chunker operator++(int) {
  //   Chunker ret(*this);
  //   ++pos;
  //   return ret;
  // }  
  bool operator!=(const Chunker &x) const {
    return pos < x.pos;
  }
    
  std::string operator*() const {
    return data->substr(pos, n_gram);
  }
};

/*! \brief calculate all the chunks in a file
 *
 * This function will return a sorted list of unique chunks.
 * 
 * \param fname filename of file to open
 * \param n chunk length
 */
ChunkSet calculate_chunks(const char *fname, int n) {
  Chunker chunker(fname, n);
  ChunkSet chunks(chunker.begin(), chunker.end());

  std::sort(chunks.begin(), chunks.end());
  auto last = std::unique(chunks.begin(), chunks.end());
  chunks.erase(last, chunks.end());
  return chunks;
}


/*! \brief Simple chunk-set reader class (functor)
 *
 * This class reads chunks from a json file. The set is checked for
 * the correct chunk length and normalised.
 */
class ChunkSetReader {
  ChunkSet chunkset; //!< here the chunkset is stored
public:
  /*! \brief Constructor from stream
   *
   * \param n chunk length
   * \param input input stream
   */
  ChunkSetReader(unsigned int n, std::istream &input) {
    Json::Value root;
    input >> root;
    if(root["N"].asUInt() != n) {
      throw std::logic_error("storage has different chunk size");
    }
    //Create set reference (for set stored json).
    const Json::Value &json_chunks(root["chunks"]);
    chunkset.resize(json_chunks.size());
    //Now copy the strings while interpreting escape characters.
    std::transform(json_chunks.begin(), json_chunks.end(), chunkset.begin(), [](const Json::Value &x) { return x.asString(); });
    //Normalisation procedure:
    //Sort chunks in the chunkset
    std::sort(chunkset.begin(), chunkset.end());
    //Use standard algorithm to remove non-unique members
    auto last = std::unique(chunkset.begin(), chunkset.end());
    chunkset.erase(last, chunkset.end());
    //Sanity checks.
    std::for_each(chunkset.begin(), chunkset.end(), [n] (const std::string &x) {
	if(x.length() != n) {
	  throw std::invalid_argument(str(boost::format("chunk '%s' has length != %u") % x % n));
	}
      });
  }
  /*! \brief Get chunkset
   *
   * Get a reference to the internal chunkset
   */
  ChunkSet &get_chunkset() { return chunkset; }
};


/*! \brief Get initial chunks from first filename and/or chunk storage.
 *
 * \param n chunk size 
 * \param fname file to get chunks from
 * \param cs_storage chunk storage filename
 */
ChunkSet get_initial_chunks(unsigned int n, const char *filename, const std::string &cs_name) {
  std::cout << "Calculating all chunks of the first file: " << filename << std::endl;
  ChunkSet chunkset(calculate_chunks(filename, n));
  if(!cs_name.empty()) {
    std::cout << "Loading chunks from storage: " << cs_name << std::endl;
    std::ifstream input(cs_name);
    if(input) {
      ChunkSetReader set_two(n, input);
      //Do an intersection of the two sets.
      ChunkSet intersection;
      std::set_intersection(chunkset.begin(), chunkset.end(), set_two.get_chunkset().begin(), set_two.get_chunkset().end(), std::back_inserter(intersection));
      std::cout << "\tUnique Chunks in Intersection: " << intersection.size() << std::endl;
      //And now replace the chunk set with the intersection.
      std::swap(intersection, chunkset);
    } else {
      //If it fails then let it fail (for now).
      std::cerr << "\tCan not load from storage.\n";
    }
  }
  return chunkset;
}


int main(int argc, char **argv) {
  // Json::Value json;
  // json["oooo"] = "asdfasdfs";
  // std::cout << json << '\n';
  // return 0;
  CLIParams cli_params(cli_parse(argc, argv));
  ChunkSet chunkset(get_initial_chunks(cli_params.n, cli_params.filenames.at(0), cli_params.chunkstorage));
  std::cout << "Number of unique chunks found: " << chunkset.size() << std::endl;
  std::for_each(++cli_params.filenames.begin(), cli_params.filenames.end(), [&chunkset, cli_params] (const char *fname) {
      try {
	ChunkSet intersection;
      
	std::cout << "Calculating chunks for: " << fname << std::endl;
	ChunkSet set_two(calculate_chunks(fname, cli_params.n));
	std::cout << "\t Unique Chunks: " << set_two.size() << std::endl;
	std::set_intersection(chunkset.begin(), chunkset.end(), set_two.begin(), set_two.end(), std::back_inserter(intersection));
	std::cout << "\t Total intersection size: " << intersection.size() << std::endl;
	std::swap(intersection, chunkset);
      }
      catch(const std::exception &excp) {
	std::cerr << "Exception: " << excp.what() << '.' << std::endl;
      }
    });
  if(!cli_params.subtract_chunks.empty()) {
    ChunkSet difference;
    std::cout << boost::format("Subtracting chunks from set file '%s'.\n") % cli_params.subtract_chunks;
    std::ifstream input(cli_params.subtract_chunks);
    ChunkSetReader set_two(cli_params.n, input);
    std::set_difference(chunkset.begin(), chunkset.end(), set_two.get_chunkset().begin(), set_two.get_chunkset().end(), std::back_inserter(difference));
    std::swap(difference, chunkset);
  }
  std::cout << "Chunks (total " << chunkset.size() << ") found in all files:\n";
  if(!chunkset.empty()) {
    for(auto x : chunkset) {
      //wchar_t wc = L'␀';
      std::cout << "⇾ ";
      std::for_each(x.begin(), x.end(), [] (char c) {
	  std::cout << boost::format(" %02X") % (static_cast<int>(c) & 0xFF);
	});
      std::cout << "  | ";
      std::for_each(x.begin(), x.end(), [] (char c) {
	  if(c == 0) {
	    std::cout << "․";
	  } else if(c < ' ') {
	    std::cout << "▒";
	  } else if(c >= 0x7f) {
	    std::cout << "░";
	  } else {
	    std::cout << c;
	  }
	});
      std::cout << " |" << std::endl;
    }
  } else {
    std::cout << "No chunks found!\n";
  }
  if(!cli_params.chunkstorage.empty()) {
    std::cout << "Storing chunks.\n";
    Json::Value root;
    Json::Value all_chunks(Json::arrayValue);
    for(auto x : chunkset) {
	all_chunks.append(x);
    }
    root["N"] = cli_params.n;
    root["chunks"] = all_chunks;
    std::ofstream output(cli_params.chunkstorage);
    output << root << std::endl;
    // {
    //   std::ifstream in(cli_params.chunkstorage);
    //   std::wofstream wout("/tmp/wout");
    //   std::locale utf8_locale("en_US.UTF8");
    //   wout.imbue(utf8_locale); 

    //   char x;
    //   if(!in) throw "ERROR";
    //   while(in.get(x)) {
    // 	wchar_t wch = L'☻';
    // 	wch = static_cast<unsigned char>(x);
    // 	wout << wch;
    //   }
    //   wout << std::endl;
    // }
  }
  return 0;
}
