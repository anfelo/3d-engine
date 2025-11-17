APP_NAME = 3DEngine
BUILD_DIR = ./bin
RESOURCES_DIR = resources
C_FILES = ./src/*.c ./src/*.cpp ./src/imgui/*.cpp
CFLAGS = -Wall -g -O0 -std=c++17

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	INCLUDES = -I/usr/include -I/usr/local/include
	LDFLAGS = -L/usr/lib -L/usr/local/lib
	LIBS = -lglfw -lGL -ldl -lassimp -Wl,-rpath,/usr/local/lib
endif

ifeq ($(UNAME_S), Darwin)
	INCLUDES = -I/usr/include -I/usr/local/include
	LDFLAGS = -L/usr/lib -L/usr/local/lib
	LIBS = -lglfw -lassimp -framework GLUT -framework OpenGL -Wl,-rpath,/usr/local/lib
endif

all: build copy_resources

build:
	mkdir -p $(BUILD_DIR)
	clang++ $(CFLAGS) $(C_FILES) -o $(BUILD_DIR)/$(APP_NAME) $(INCLUDES) $(LDFLAGS) $(LIBS)

copy_resources:
	cp -r $(RESOURCES_DIR) $(BUILD_DIR)/

clean:
	rm -rf $(BUILD_DIR)
