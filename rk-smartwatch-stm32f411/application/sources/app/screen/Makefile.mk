include sources/app/screen/flappy_bird/Makefile.mk

INCLUDE_FLAGS += -I./sources/app/screen

VPATH += ./sources/app/screen

#CPP source files
SOURCE_CPP += tile_watchface.cpp
SOURCE_CPP += tile_weather.cpp
SOURCE_CPP += tile_menu.cpp
SOURCE_CPP += screen_startup.cpp
SOURCE_CPP += screen_main.cpp
SOURCE_CPP += screen_display.cpp
SOURCE_CPP += screen_time.cpp
SOURCE_CPP += screen_system.cpp
SOURCE_CPP += screen_charge.cpp
SOURCE_CPP += screen_about.cpp
