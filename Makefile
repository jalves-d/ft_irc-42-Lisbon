# Compiler and compilation flags
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Target executable name
NAME = ircserv

# Source and object directories
SRCDIR = srcrs
OBJDIR = objs

# Find all source files
SRCS = $(shell find $(SRCDIR) -name '*.cpp')
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Phony targets
.PHONY: all clean fclean re

# Default target
all: $(OBJDIR) $(NAME)

# Rule to create the object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build the executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Clean up object files
clean:
	rm -rf $(OBJDIR)

# Clean up object files and the executable
fclean: clean
	rm -f $(NAME)

# Rebuild the project
re: fclean all
