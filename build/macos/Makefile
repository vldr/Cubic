OUTPUT = output/cubic
CXX = clang++
CXXFLAGS = -arch x86_64 -arch arm64 -std=c++17 -Iincludes -O3 -Wno-deprecated-declarations
LINKFLAGS = -Llibs -lpthread -lSDL2main -lSDL2 -framework OpenGL -lGLEW

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
	install_name_tool -change /opt/homebrew/opt/glew/lib/libGLEW.2.2.dylib @executable_path/libGLEW.dylib output/cubic
	install_name_tool -change /usr/local/opt/glew/lib/libGLEW.2.2.dylib @executable_path/libGLEW.dylib output/cubic
	install_name_tool -change /opt/homebrew/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib output/cubic
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib output/cubic
	cp -rf libs/libSDL2.dylib output/libSDL2.dylib
	cp -rf libs/libGLEW.dylib output/libGLEW.dylib


objs/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
