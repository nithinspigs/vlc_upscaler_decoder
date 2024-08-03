# Variables
MAKE = make
CC = gcc
CXX = g++
VLC_SRC_PATH = ./include/vlc/plugins/
VLC_LIBS_PATH = /Applications/VLC.app/Contents/MacOS/lib
CV_SRC_PATH = /usr/local/include/opencv4
CFLAGS = -std=gnu99 -I$(VLC_SRC_PATH) -g -Wall -fPIC -D__PLUGIN__ -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_THREAD_SAFE
CXXFLAGS = -std=c++11 -I$(CV_SRC_PATH) -fPIC
LDFLAGS = -L$(VLC_LIBS_PATH)
LDLIBS = -lvlccore

# Rules

libvlc_upscaler_decoder_plugin.dylib: vlc_upscaler_decoder.o cvDecode/libCvDecode.a
	$(CXX) $^ $(LDFLAGS) $(LDLIBS) -dynamiclib -o $@

vlc_upscaler_decoder.o: vlc_upscaler_decoder.c
	$(CC) $(CFLAGS) -c $^ -o $@

cvDecode/libCvDecode.a: cvDecode/cvDecodeClass.o cvDecode/cvDecodeWrapper.o
	ar rcs $@ $^

cvDecode/cvDecodeClass.o: cvDecode/cvDecodeClass.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

cvDecode/cvDecodeWrapper.o: cvDecode/cvDecodeWrapper.cpp 
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -f *.o *.dylib cvDecode/*.o cvDecode/*.a

.PHONY: all
all: clean libvlc_upscaler_decoder_plugin.dylib
