SHELL = /usr/bin/env bash

CXX 	 := clang++
CXXFLAGS  = -c -g -std=c++20 $(WARNINGS) -I$(INCDIR)

WARNINGS := -pedantic-errors -Wall -Wextra -Weffc++ -fsanitize=undefined -Wno-unused-parameter
LDFLAGS  := -fsanitize=undefined
LDLIBS   := -lglfw -lvulkan -ldl -lpthread

BINARY   := bin
DEFINES  := -DGLM_ENABLE_EXPERIMENTAL

INCDIR         := include
OBJDIR         := build
SRCDIR         := src
SHADER_DIR     := shaders
SHADER_OBJ_DIR := $(OBJDIR)/shaders

TEST_SRCDIR    := tests/src
TEST_INCDIR    := tests/include
TEST_OBJDIR    := tests/build
TEST_BINDIR    := tests/bin
TEST_BINARIES   = $(patsubst $(TEST_SRCDIR)/%.cpp,$(TEST_BINDIR)/%,$(TEST_CPPS))

CPPS         := $(shell find $(SRCDIR) -type f -name *.cpp)
TEST_CPPS    := $(shell find $(TEST_SRCDIR) -type f -name *.cpp)
FRAG_SHADERS := $(shell find $(SHADER_DIR) -type f -name '*.frag')
VERT_SHADERS := $(shell find $(SHADER_DIR) -type f -name '*.vert')

OBJS              := $(CPPS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TEST_OBJS         := $(TEST_CPPS:$(TEST_SRCDIR)/%.cpp=$(TEST_OBJDIR)/%.o)
FRAG_SHADERS_OBJS := $(addprefix $(OBJDIR)/, $(addsuffix .spv, $(FRAG_SHADERS)))
VERT_SHADERS_OBJS := $(addprefix $(OBJDIR)/, $(addsuffix .spv, $(VERT_SHADERS)))
SHADER_OBJS       := $(FRAG_SHADERS_OBJS) $(VERT_SHADERS_OBJS)

GF := "\e[033;32m"
CF := "\e[033;36m"
RF := "\e[033;31m"
YF := "\e[033;33m"
PF := "\e[033;35m"
NF := "\e[033;39m"

export TEST_TARGETS=
TESTING=

.PHONY: clean run all files new
all: $(SHADER_OBJS) $(TEST_BINARIES) $(BINARY) test

# shaders #
$(VERT_SHADERS_OBJS): $(SHADER_OBJ_DIR)/%.spv: $(SHADER_DIR)/%
	@echo -e $(YF)[COMPILING .vert]$(NF)$(CF) $(subst $(SHADER_DIR)/,,$<)$(NF)
	@glslc $< -o $@

$(FRAG_SHADERS_OBJS): $(SHADER_OBJ_DIR)/%.spv: $(SHADER_DIR)/%
	@echo -e $(YF)[COMPILING .frag]$(NF)$(CF) $(subst $(SHADER_DIR)/,,$<)$(NF)
	@glslc $< -o $@

# tests #
$(TEST_BINARIES): $(TEST_OBJS)
	@echo -e $(GF)[LINKING TEST]$(NF)
	@$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(TEST_OBJS): $(TEST_OBJDIR)/%.o: $(TEST_SRCDIR)/%.cpp $(SRCDIR)/%.cpp $(INCDIR)/%.hpp $(BINARY)
	@echo -e $(GF)[COMPILING TEST]$(NF)$(CF) $(subst $(TEST_SRCDIR)/,,$<)$(NF)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $< -o $@

# cpp #
$(BINARY): $(OBJS)
	@echo -e $(GF)[LINKING]$(NF)
	@$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.hpp
	@echo -e $(GF)[COMPILING]$(NF)$(CF) $(subst $(SRCDIR)/,,$<)$(NF)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $< -o $@
	$(eval TEST_TARGETS := $(TEST_TARGETS) $(TEST_BINDIR)/$*)

# utilities
test: $(TEST_BINARIES) $(BINARY)
	$(eval TESTING := $(foreach bin,$(TEST_TARGETS),$(wildcard $(bin))))
	$(foreach bin,$(TESTING), @echo -e $(GF)[TESTING]$(NF)$(CF) $(subst $(TEST_BINDIR)/,,$(bin))$(NF) && ./$(bin))

clean:
	@rm -f $(OBJS) $(BINARY) $(SHADER_OBJS) $(TEST_OBJS) $(TEST_BINARIES)

run: $(BINARY) $(SHADER_OBJS)
	./bin

files:
	@read -p 'Enter [File name] [Folder]: ' name folder; \
	if [ -n "$$name" ] && [ -n "$$folder" ]; then \
	  mkdir -p $(INCDIR)/$$folder; \
	  mkdir -p $(SRCDIR)/$$folder; \
	  touch $(INCDIR)/$$folder/"$$name".hpp; \
	  touch $(SRCDIR)/$$folder/"$$name".cpp; \
	  echo -e "#pragma once\n\n//" > $(INCDIR)/$$folder/"$$name".hpp; \
	  echo -e "#include "\"$$folder/$$name".hpp\"\n\n//" > $(SRCDIR)/$$folder/"$$name".cpp; \
	elif [ -n "$$name" ] && [ -z "$$folder"]; then \
	  #touch $(INCDIR)/"$$name".hpp; \
	  #touch $(SRCDIR)/"$$name".cpp; \
	  echo -e "#pragma once\n\n//" > $(INCDIR)/"$$name".hpp; \
	  echo -e "#include "\"$$name".hpp\"\n\n//" > $(SRCDIR)/"$$name".cpp; \
	else \
	  echo "Invalid input"; \
	fi;
