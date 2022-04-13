#! /usr/bin/make -f

OBJS = ngram-storage.o
LIBS = -lm
CFLAGS = -O3 -Wall -DNDEBUG
CXXFLAGS = -O3 -Wall -DNDEBUG -std=c++17 $(JSONCPP)
OBJSXX = fileformat.o
JSONCPP  = `pkg-config --cflags jsoncpp`

ALL_FILES = simichunks ngram-storage emmagrammer.py _emmagrammer.so emmagrammer simple-histogram ngramify histogramify 2gram_histo_to_csv


all: $(ALL_FILES)

2gram_histo_to_csv: 2gram_histo_to_csv.o $(OBJSXX)
	$(CXX) -o $@ $(CXXFLAGS) $+

histogramify: histogramify.o $(OBJSXX)
	$(CXX) -o $@ $(CXXFLAGS) $+

ngramify: ngramify.o
	$(CXX) -o $@ $(CXXFLAGS) $+

emmagrammer: emmagrammer.o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $+ $(LIBS)

ngram-storage: example.o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $+ $(LIBS)

_emmagrammer.so: emmagrammer.py $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared `python-config --includes` -o _emmagrammer.so ngram-storage.c emmagrammer_wrap.c

emmagrammer.py: emmagrammer.i
	swig -Wall -python emmagrammer.i

simple-histogram: simple-histogram.o histogram.o
	$(CXX) -o $@ $(CXXFLAGS) $+ $(LIBS)

simichunks: simichunks.o
	$(CXX) -o $@ $(JSONCPP) $(CXXFLAGS) $+ $(LIBS) `pkg-config --libs jsoncpp`

.PHONY: all clean

clean:
	rm -f $(ALL_FILES)
	rm -f *.o emmagrammer_wrap.c *.pyc

.PHONY: distclean
distclean:
	$(MAKE) clean
	rm -f *~

