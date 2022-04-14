CXX 	 = clang++
CXXFLAGS = -c -g -std=c++20 $(WARNINGS) -I$(INCDIR)

WARNINGS = -pedantic-errors -Wall -Wextra -Weffc++ -fsanitize=undefined -Wno-unused-parameter
LDFLAGS  = -fsanitize=undefined
LDLIBS   = -lglfw -lvulkan -ldl -lpthread

DEPENDENCIES = $(shell find $(SRCDIR) -name *.hpp)
BINARY       = bin
DEFINES      = -DGLM_ENABLE_EXPERIMENTAL

OBJDIR = build
SRCDIR = src
INCDIR = include

#$(shell find -type f -name "*.cpp")
CPPS:= $(wildcard $(SRCDIR)/*.cpp)
OBJS:= $(CPPS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

GF = "\e[033;32m"
CF = "\e[033;36m"
RF = "\e[033;31m"
NF = "\e[033;39m"

####
#(wildcard $(ShaderDir)/*.frag)
#$(wildcard $(ShaderDir)/*.vert)

SHADER_DIR=shaders

FRAG_SHADERS:= $(shell find $(SHADER_DIR) -type f -name "*.frag")
FRAG_SHADERS_OBJS:= $(addsuffix .spv, $(addprefix $(OBJDIR)/, $(FRAG_SHADERS)))

VERT_SHADERS:= $(shell find $(SHADER_DIR) -type f -name "*.vert")
VERT_SHADERS_OBJS:= $(addsuffix .spv, $(addprefix $(OBJDIR)/, $(VERT_SHADERS)))
SHADER_OBJS:= $(FRAG_SHADERS_OBJS) $(VERT_SHADERS_OBJS)
####


.PHONY: clean run all
all: $(BINARY) $(SHADER_OBJS)

$(BINARY): $(OBJS)
	@echo -e $(GF)[LINKING]$(NF)
	@$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.hpp
	@echo -e $(GF)[COMPILING]$(NF)$(CF) $(subst $(SRCDIR)/,,$<)$(NF)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $< -o $@


#####
$(VERT_SHADERS_OBJS): $(VERT_SHADERS)
	glslc $< -o $@

$(FRAG_SHADERS_OBJS): $(FRAG_SHADERS)
	glslc $< -o $@
#####


clean:
	@rm -f $(OBJS) $(BINARY)

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

