# Variables
MAKE = make
CC = gcc
CXX = g++
VLC_SRC_PATH = /Users/nithin/Personal/git-repos/vlc/
VLC_LIBS_PATH = /Applications/VLC.app/Contents/MacOS/lib/
CV_SRC_PATH = /usr/local/include/opencv4/
CV_LIBS_PATH = ./cvDecode/
CFLAGS = -std=gnu99 -I$(VLC_SRC_PATH)/include/ -g -Wall -fPIC
CXXFLAGS = -std=c++11 -I$(CV_SRC_PATH) -fPIC
LDFLAGS = -L$(VLC_LIBS_PATH) -L$(CV_LIBS_PATH)
LDLIBS = -lvlc -lvlccore -lCvDecode
SUBDIRS = ./cvDecode/

# Rules

libvlc_upscaler_decoder.dylib: vlc_upscaler_decoder.o libCvDecode.a
	$(CXX) vlc_upscaler_decoder.o $(LDFLAGS) $(LDLIBS) -shared -o libvlc_upscaler_decoder.dylib

vlc_upscaler_decoder.o: vlc_upscaler_decoder.c
	$(CC) $(CFLAGS) vlc_upscaler_decoder.c -c -o vlc_upscaler_decoder.o

libCvDecode.a: cvDecodeClass.o cvDecodeWrapper.o
	ar rcs cvDecode/libCvDecode.a cvDecode/cvDecodeClass.o cvDecode/cvDecodeWrapper.o

cvDecodeClass.o: cvDecode/cvDecodeClass.cpp
	$(CXX) $(CXXFLAGS) -c cvDecode/cvDecodeClass.cpp -o cvDecode/cvDecodeClass.o

cvDecodeWrapper.o: cvDecode/cvDecodeWrapper.cpp 
	$(CXX) $(CXXFLAGS) -c cvDecode/cvDecodeWrapper.cpp -o cvDecode/cvDecodeWrapper.o

.PHONY: clean
clean:
	rm -f *.o *.dylib cvDecode/*.o cvDecode/*.a

.PHONY: all
all: clean libvlc_upscaler_decoder.dylib
	
