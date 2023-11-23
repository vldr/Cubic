OUTPUT = Build/cubic
CXX = clang++
CXXFLAGS = -arch x86_64 -arch arm64 -std=c++17 -Wno-deprecated-declarations -IIncludes -O3

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
	$(CXX) $(CXXFLAGS) -LLibraries -lpthread -lSDL2main -lSDL2 -framework OpenGL -lGLEW -o $(OUTPUT) $(OBJS)
	install_name_tool -change /opt/homebrew/opt/glew/lib/libGLEW.2.2.dylib @executable_path/libGLEW.dylib Build/cubic
	install_name_tool -change /usr/local/opt/glew/lib/libGLEW.2.2.dylib @executable_path/libGLEW.dylib Build/cubic
	install_name_tool -change /opt/homebrew/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib Build/cubic
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib Build/cubic
	cp -rf Libraries/libSDL2.dylib Build/libSDL2.dylib
	cp -rf Libraries/libGLEW.dylib Build/libGLEW.dylib


Objects/%.o: ../Source/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
