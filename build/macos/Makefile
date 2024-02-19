OUTPUT = output/cubic
OUTPUT_BUNDLE = output/Cubic.app
CXX = clang++
CXXFLAGS = -arch x86_64 -arch arm64 -MMD -std=c++17 -Iincludes -ffast-math -O3
LINKFLAGS = -Llibs -lpthread -lSDL2main -lSDL2 -framework OpenGL -lGLEW

SRCS = $(wildcard ../../src/*.cpp)
OBJS = $(patsubst ../../src/%, objs/%, $(patsubst %.cpp, %.o, $(SRCS)))
DEPS = $(patsubst %.o, %.d, $(OBJS))

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
	install_name_tool -change /opt/homebrew/opt/glew/lib/libGLEW.2.2.dylib @executable_path/libGLEW.dylib $(OUTPUT)
	install_name_tool -change /usr/local/opt/glew/lib/libGLEW.2.2.dylib @executable_path/libGLEW.dylib $(OUTPUT)
	install_name_tool -change /opt/homebrew/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib $(OUTPUT)
	install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib $(OUTPUT)
	rm -rf $(OUTPUT_BUNDLE)
	mkdir -p "$(OUTPUT_BUNDLE)"/Contents/{MacOS,Resources}
	cp -rf resources/Info.plist "$(OUTPUT_BUNDLE)/Contents/"
	cp -rf resources/Icon.icns "$(OUTPUT_BUNDLE)/Contents/Resources/"
	cp -rf libs/libSDL2.dylib "$(OUTPUT_BUNDLE)/Contents/MacOS/"
	cp -rf libs/libGLEW.dylib "$(OUTPUT_BUNDLE)/Contents/MacOS/"
	mv $(OUTPUT) "$(OUTPUT_BUNDLE)/Contents/MacOS/"

objs/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

-include $(DEPS)