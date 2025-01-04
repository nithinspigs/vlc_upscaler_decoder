# Variables

CC = gcc
CXX = g++

VLC_SRC_PATH = /Users/nithin/Personal/git-repos/vlc-master/
VLC_LIBS_PATH = /Applications/VLC.app/Contents/MacOS/lib/

FFMPEG_SRC_PATH = /opt/homebrew/Cellar/ffmpeg@6/6.1.2/include/
FFMPEG_LIBS_PATH = /opt/homebrew/Cellar/ffmpeg@6/6.1.2/lib/

VLC_HEADERS_PATH = ./include/vlc/plugins/
VLC_PLUGIN_LIBS_PATH = /Applications/VLC.app/Contents/MacOS/plugins/

CV_SRC_PATH = /opt/homebrew/include/opencv4/
CV_LIBS_PATH = /opt/homebrew/Cellar/opencv/4.10.0_2/lib/

CFLAGS = -std=gnu99 -I$(VLC_HEADERS_PATH) -I$(FFMPEG_SRC_PATH) -g -Wall -fPIC -D__PLUGIN__ -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_THREAD_SAFE
CXXFLAGS = -std=c++11 -I$(CV_SRC_PATH) -g -Wall -fPIC

LDFLAGS = -L$(VLC_LIBS_PATH) -L$(FFMPEG_LIBS_PATH) -L$(VLC_PLUGIN_LIBS_PATH) -L$(CV_LIBS_PATH)
LDLIBS = -lvlccore -lavcodec -lavcodec_plugin -lopencv_core -lopencv_imgcodecs

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
