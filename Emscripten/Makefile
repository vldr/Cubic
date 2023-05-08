OUTPUT = Build/index.html
CXX = emcc
CXXFLAGS = -Wall --closure 1 -Wno-unused-command-line-argument -sEVAL_CTORS -flto -fno-rtti -fno-exceptions -IIncludes \
		   -ffast-math -sENVIRONMENT=web -sGL_UNSAFE_OPTS -sMAX_WEBGL_VERSION=2 -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 -sWASM=1 -O3

SRCS = $(wildcard ../Source/*.cpp)
OBJS = $(patsubst ../Source/%, Objects/%, $(patsubst %.cpp, %.o, $(SRCS)))

ifeq ($(strip $(shell which $(CXX))),)
$(error $(CXX) is not installed)
endif
 
all: Objects/ Build/ build 

Objects/:
	mkdir Objects
 
Build/:
	mkdir Build

build: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) --shell-file Shell.html --embed-file ../Assets/ $(OBJS)

Objects/%.o: ../Source/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
