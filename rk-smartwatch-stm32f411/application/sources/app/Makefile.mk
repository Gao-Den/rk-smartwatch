include sources/app/screen/Makefile.mk

INCLUDE_FLAGS += -I./sources/app

VPATH += ./sources/app

#CPP source files
SOURCE_CPP += app.cpp
SOURCE_CPP += app_flash.cpp
SOURCE_CPP += bsp.cpp
SOURCE_CPP += shell.cpp
SOURCE_CPP += task_list.cpp
SOURCE_CPP += task_system.cpp
SOURCE_CPP += task_fw.cpp
SOURCE_CPP += task_display.cpp
SOURCE_CPP += task_console.cpp
SOURCE_CPP += task_polling.cpp
SOURCE_CPP += task_dbg.cpp
