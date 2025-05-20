# Compiler
CC = gcc


# Source file
SRC = usp.c

# Object file
OBJ = $(SRC:.txt=.o)

# Output executable
EXEC = USP

# Default target
all: $(EXEC)

# Rule to create the executable
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# Rule to create the object file
%.o: %.txt
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ) $(EXEC)

# Phony targets
.PHONY: all clean