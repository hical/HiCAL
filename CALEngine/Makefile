CXX = g++
CXXFLAGS = -O3 -Wall -pthread --std=c++14

OBJ_DIR ?= obj
SRC_DIRS ?= src

# Modify BIN_SRCS to add targets
BIN_SRCS := $(SRC_DIRS)/bmi_fcgi.cc $(SRC_DIRS)/bmi_cli.cc $(SRC_DIRS)/corpus_parser.cc
BIN_OBJS := $(BIN_SRCS:%=$(OBJ_DIR)/%.o)
BIN_TARGETS := $(notdir $(basename $(BIN_SRCS)))

SRCS := $(shell find $(SRC_DIRS) -name *.cc)
SRCS := $(filter-out $(BIN_SRCS),$(SRCS))
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)


DEPS := $(OBJS:.o=.d)
DEP_FLAGS = -MMD -MP

$(BIN_TARGETS): % : $(OBJ_DIR)/$(SRC_DIRS)/%.cc.o $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(OBJ_DIR)/$(SRC_DIRS)/$@.cc.o -o $@

$(OBJ_DIR)/%.cc.o: %.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) $(DEP_FLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_TARGETS)
	rm -r $(OBJ_DIR)

# print-%  : ; @echo $* = $($*)
-include $(DEPS)

MKDIR_P ?= mkdir -p
