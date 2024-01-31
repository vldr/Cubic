OUTPUT = output/index.html
CXX = emcc
CXXFLAGS = -std=c++17 -Iincludes -flto -fno-rtti -fno-exceptions -ffast-math -O3
LINKFLAGS = --closure 1 -Llibs -lidbfs.js -lSDL2 -lwebsocket.js -sALLOW_MEMORY_GROWTH=1 -sGL_UNSAFE_OPTS \
			-sWASM=1 -sEVAL_CTORS -sENVIRONMENT=web -sMAX_WEBGL_VERSION=2 --shell-file Shell.html

SRCS = $(wildcard ../../src/*.cpp)
OBJS = $(patsubst ../../src/%, objs/%, $(patsubst %.cpp, %.o, $(SRCS)))

ifeq ($(strip $(shell which $(CXX))),)
$(error $(CXX) is not installed)
endif
 
all: objs/ output/ build 

objs/:
	mkdir objs
 
output/:
	mkdir output

build: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LINKFLAGS) -o $(OUTPUT) $(OBJS)
	cp favicon.ico output/favicon.ico

objs/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
