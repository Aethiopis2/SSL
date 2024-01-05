# A linux makefile; this file will build all the neccessary files
# By Rediet Worku
#
# Date Created: 26th of January 2022, Wedensday

# change the CC to the compiler in desire; i.e. clang
# CFLAGS is the compiler options (add a -g flag to enable debugger options basically analogus to debug mode)
CC	:= g++
CFLAGS	:= -Wall -Werror -std=c++14 -g

# define any directories containing header files
INCLUDES = -Iinclude

#define any library path
LIBS = 

#define the C++ source files
SRCS = src/http_client.cpp

#define the C/C++ object files; replace every occurance of .c in SRCS with .o
OBJS = $(SRCS:.c=.o)

#define executables and shared libraries (we won't be using complier settings to link
#	.so files during compile time)
MAIN = test/http_client


# the following section is generic; it can be used to build for any system
# just by changing the dependencies in the above section
.PHONY: depend clean

all: $(MAIN)
	@echo Project has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)


# suffix replacement rules
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELTE THIS LINE -- used by make depend