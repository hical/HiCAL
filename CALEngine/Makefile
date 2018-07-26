CXX = g++
CXXFLAGS = -Wall -pthread --std=c++14 -lfcgi -lfcgi++ -larchive
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g3 -O0 -DDEBUG
else
    CXXFLAGS += -O3
endif

OBJ_DIR ?= obj
SRC_DIRS ?= src
TEST_DIRS ?= tests

# Modify BIN_SRCS to add targets
BIN_SRCS := $(SRC_DIRS)/bmi_fcgi.cc $(SRC_DIRS)/bmi_cli.cc $(SRC_DIRS)/corpus_parser.cc
BIN_OBJS := $(BIN_SRCS:%=$(OBJ_DIR)/%.o)
BIN_TARGETS := $(notdir $(basename $(BIN_SRCS)))

TEST_SRCS := $(shell find $(TEST_DIRS) -name *.cc)
TEST_TARGETS := $(notdir $(basename $(TEST_SRCS)))

SRCS := $(shell find $(SRC_DIRS) -name '*.cc')
SRCS := $(filter-out $(BIN_SRCS),$(SRCS))
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)

DEPS := $(OBJS:.o=.d)
DEP_FLAGS = -MMD -MP


all: $(BIN_TARGETS)

$(BIN_TARGETS): % : $(OBJ_DIR)/$(SRC_DIRS)/%.cc.o $(OBJS)
	$(CXX)  $(OBJS) $(OBJ_DIR)/$(SRC_DIRS)/$@.cc.o -o $@ $(CXXFLAGS)

$(TEST_TARGETS): % : $(OBJ_DIR)/$(TEST_DIRS)/%.cc.o $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(OBJ_DIR)/$(TEST_DIRS)/$@.cc.o -o $(TEST_DIRS)/$@
	cd $(TEST_DIRS) && (./$@; cd ..)

$(OBJ_DIR)/%.cc.o: %.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) $(DEP_FLAGS) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGETS)

clean:
	rm -f $(BIN_TARGETS)
	rm -r $(OBJ_DIR)

print-%  : ; @echo $* = $($*)

-include $(DEPS)

MKDIR_P ?= mkdir -p
