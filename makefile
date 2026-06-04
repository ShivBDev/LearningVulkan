BUILD_DIR := bin
ASSEMBLY := learning_vulkan
SRC_DIR := src

COMPILER := clang++
COMPILER_FLAGS := -fno-signed-char -std=c++2c -gdwarf-5
INCLUDE_FLAGS := \
	-I./src \
	-I$(VULKAN_SDK)/include \
	-I$(VULKAN_SDK)/lib \
	-I/opt/homebrew/include
LINKER_FLAGS := -lvulkan -lglfw3 -L/opt/homebrew/lib -framework Cocoa -framework IOKit -framework CoreVideo
SOURCE := $(shell find $(SRC_DIR) -type f -name "*.cpp")

default:
	$(MAKE) clean
	$(MAKE) build

.PHONY: clean build
clean:
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR)
build:
	$(COMPILER) $(COMPILER_FLAGS) $(SOURCE) $(INCLUDE_FLAGS) $(LINKER_FLAGS) -o $(BUILD_DIR)/$(ASSEMBLY)




$(SOURCE): 