# Makefile

# root directory of asset files
ASSETS_ROOT_DIR=assets

# root directory of source files
SOURCE_ROOT_DIR=src

# root directory for build files
BUILD_ROOT_DIR=build

# directory for compiled object files
OBJECT_ROOT_DIR=$(BUILD_ROOT_DIR)/obj

# directory for the runtime output
RUNTIME_ROOT_DIR=bin

# output configuration
EXECUTABLE_OUT=$(RUNTIME_ROOT_DIR)/gamebin

# compiler configuration
CC=clang++

# SFML library flags
SFML_CFLAGS=$(shell pkg-config sfml-graphics sfml-window sfml-system --cflags)
SFML_LDFLAGS=$(shell pkg-config sfml-graphics sfml-window sfml-system sfml-audio --libs) -framework OpenAL

GAME_SOURCES=$(shell find $(SOURCE_ROOT_DIR) -name "*.cpp")
GAME_OBJECTS=$(patsubst $(SOURCE_ROOT_DIR)/%.cpp,$(OBJECT_ROOT_DIR)/%.o,$(GAME_SOURCES))

CFLAGS=-DSTATIC_SFML $(SFML_CFLAGS) $(addprefix -I,$(wildcard $(SOURCE_ROOT_DIR)/**)) -Wall -Wextra -Wpedantic -std=c++17 -O3 -g
LDFLAGS=$(SFML_LDFLAGS)

# phony targets
.PHONY: all dirs assets game clean launch

# special case - first target in Makefile is executed when "make" is run without a target
all: dirs assets game

# build and run the game
launch: all
	@echo "🚀  \x1b[32mLaunching: $(EXECUTABLE_OUT)\x1b[0m"
	@$(EXECUTABLE_OUT)

# make the directories if they do not exist
dirs:
	@mkdir -p $(BUILD_ROOT_DIR) $(RUNTIME_ROOT_DIR)/$(ASSETS_ROOT_DIR)

# copy the assets from the assets directory into the runtime output directory
assets:
	@cp -r $(ASSETS_ROOT_DIR)/* $(RUNTIME_ROOT_DIR)/$(ASSETS_ROOT_DIR)

# make the game executable
game: $(EXECUTABLE_OUT) assets

# clean all output
clean:
	@echo "\x1b[32mCleaning: $(BUILD_ROOT_DIR)\x1b[0m"
	@echo "\x1b[32mCleaning: $(RUNTIME_ROOT_DIR)\x1b[0m"
	@rm -rf $(BUILD_ROOT_DIR) $(DIST_ROOT_DIR) $(RUNTIME_ROOT_DIR)

# producer targets

# produce the main executable binary
$(EXECUTABLE_OUT): $(GAME_OBJECTS)
	@echo "\x1b[32mProducing: $(EXECUTABLE_OUT)\x1b[0m"
	@mkdir -p $(@D)
	@$(CC) $(GAME_OBJECTS) $(LDFLAGS) -o $@
	@$(CC) $(LDFLAGS) $^ -o $@

# compile the source files into objects
$(OBJECT_ROOT_DIR)/%.o: $(SOURCE_ROOT_DIR)/%.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(GAME_OBJECTS:.o=.d)