# Template makefile for sub directories

# this should be the first line
include $(CONFIG_FILE)

# List of .c files to be compiled
SRCS = $(filter-out $(wildcard aes*.c), $(wildcard *.c))

# List of sub-directories to be compiled
SUBDIRS =

# Additional CFLAGS for current directory.
CFLAGS =
override COMMON_CFLAGS := $(filter-out -Wall -Werror, $(COMMON_CFLAGS))

# Auxialiary object files to be linked to the final executable.
# Local rules must be defined for these object files.
# Only needed for some special cases (for example, to compile the
# dynamically generated .c files)
OBJS =

# Keep this line following SRCS/SUBDIRS/CFLAGS/OBJS
include $(COMMON_RULES)

# local rules for auxialiary object files.
