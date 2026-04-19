# CodeMaster Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Ibackend
SRCDIR = backend
OBJDIR = obj
TARGET = master_bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) 
	rm -f output/*.dot output/*.csv

run: all
	./$(TARGET) testfiles/test_quality.c

.PHONY: all clean run
