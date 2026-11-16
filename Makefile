CXX ?= g++
CXXFLAGS ?= -O2 -g -Wall -Wno-unused-function -fPIC -fvisibility=hidden -msse2 -mfpmath=sse
CXXFLAGS += -Iinclude -Isrc $(shell pkg-config --cflags fftw3f)
LIBS = $(shell pkg-config --libs fftw3f) -lpthread -lm
GUI_LIBS = $(shell pkg-config --libs x11 cairo)
GUI_CFLAGS = $(shell pkg-config --cflags x11 cairo)

DSP = src/plugin.cc src/retuner.cc src/resampler.cc src/resampler-table.cc
DEPS = $(wildcard src/*.h src/*.cc)

all: build/ScalaTuner.clap

build/ScalaTuner.clap: $(DEPS) Makefile
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(GUI_CFLAGS) -shared -o $@ $(DSP) src/gui.cc $(LIBS) $(GUI_LIBS)

clean:
	rm -rf build
.PHONY: all clean
