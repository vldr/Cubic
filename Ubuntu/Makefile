OUTPUT = Build/cubic
CXX = clang++
CXXFLAGS = -std=c++17 -IIncludes -O3

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
	$(CXX) $(CXXFLAGS) -lpthread -lSDL2main -lSDL2 -lGL -lGLEW -o $(OUTPUT) $(OBJS)

Objects/%.o: ../Source/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
