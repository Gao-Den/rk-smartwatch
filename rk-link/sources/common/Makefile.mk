include sources/common/container/Makefile.mk

INCLUDE_FLAGS += -I./sources/common

VPATH += ./sources/common

#C source files
SOURCE_C += xprintf.c
SOURCE_C += cmd_line.c
