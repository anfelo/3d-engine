APP_NAME = 3DEngine
BUILD_DIR = ./bin
RESOURCES_DIR = resources
C_FILES = ./src/*.c ./src/*.cpp
CFLAGS = -Wall -g -O0 -std=c++17

APP_DEFINES:=
APP_INCLUDES:= -I/usr/local/include -L/usr/local/lib -lglfw -lassimp -framework GLUT -framework OpenGL -Wl,-rpath,/usr/local/lib

all: build copy_resources

build:
	mkdir -p $(BUILD_DIR)
	clang++ $(CFLAGS) $(C_FILES) -o $(BUILD_DIR)/$(APP_NAME) $(APP_INCLUDES)

copy_resources:
	cp -r $(RESOURCES_DIR) $(BUILD_DIR)/

clean:
	rm -rf $(BUILD_DIR)
