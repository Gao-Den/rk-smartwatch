include sources/sys/peripheral/Makefile.mk
include sources/sys/startup/Makefile.mk

INCLUDE_FLAGS += -I./sources/sys

VPATH += ./sources/sys

# C source files
SOURCE_C += sys_startup.c
SOURCE_C += sys_irq.c
SOURCE_C += sys_cfg.c
