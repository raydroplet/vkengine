CXX         = clang++
CXXFLAGS    = -c -g -std=c++20 $(WARNINGS)

WARNINGS    = -pedantic-errors -Wall -Wextra -Weffc++ -fsanitize=undefined -Wno-unused-parameter
LDFLAGS     = -fsanitize=undefined
LDLIBS      = -lglfw -lvulkan -ldl -lpthread

DEPENDECIES = $(wildcard *.hpp)
BINARY      = bin
DEFINES     =

OBJDIR      = build
SRCDIR      = src
INCDIR      = include

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

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPENDECIES)
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

run: $(BINARY)
	./bin
