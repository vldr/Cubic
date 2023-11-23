OUTPUT = output/cubic
CXX = clang++
CXXFLAGS = -std=c++17 -Iincludes -O3
LINKFLAGS = -lpthread -lSDL2main -lSDL2 -lGL -lGLEW

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

objs/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
