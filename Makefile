PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
TARGET := zf
TEST_TARGET := test_all

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

CXX := g++
CFLAGS := -Werror -Wall -Wextra -I$(SRC_DIR)
DEBUG_FLAGS := -g -O0
OPTFLAGS := -O3 -ffast-math -march=native -flto -funroll-loops -fomit-frame-pointer -fno-exceptions -fno-rtti -pipe -falign-loops=16
LDFLAGS := -lm

CPP_SRCS_NO_MAIN := $(shell find -L ${SRC_DIR} -name '*.cpp' ! -name 'main.cpp')
CPP_SRCS := $(CPP_SRCS_NO_MAIN) $(SRC_DIR)/main.cpp

OBJS_NO_MAIN := $(addprefix $(BUILD_DIR)/, $(CPP_SRCS_NO_MAIN:.cpp=.cpp.o))
OBJS = $(addprefix $(BUILD_DIR)/, $(CPP_SRCS:.cpp=.cpp.o))

TEST_SRCS := $(TEST_DIR)/main.cpp

all: $(BUILD_DIR)/$(TARGET)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

release: CFLAGS += $(OPTFLAGS)
release: all

tests: $(BUILD_DIR)/$(TEST_TARGET)
	./$(BUILD_DIR)/$(TEST_TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p "$(dir $@)"
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

$(BUILD_DIR)/$(TEST_TARGET): clean $(OBJS_NO_MAIN) $(TEST_SRCS)
	@mkdir -p "$(dir $@)"
	$(CXX) $(CFLAGS) -o $@ $(TEST_SRCS) $(OBJS_NO_MAIN) $(LDFLAGS)

install: clean release
	install -m 0755 $(BUILD_DIR)/$(TARGET) $(BINDIR)

uninstall: $(BINDIR)/$(TARGET)
	rm -f $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean debug release tests install uninstall
