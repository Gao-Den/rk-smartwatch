###############################################################################
# author: GaoDen
# date: 10/09/2025
###############################################################################

NAME_MODULE	= rk-flash
OBJ_DIR = build

# the compiler to use
CC = gcc
CXX = g++

include sources/lite-thread/Makefile.mk
include sources/app/Makefile.mk
include sources/common/Makefile.mk
include sources/sys/Makefile.mk

# define options
APP_MODE += -DAPP_DEBUG
DEFINE_OPTIONS += $(APP_MODE)

# build options
OPTIMIZE_OPTION += -Os -g
WARNING_OPTION	+= -Wall -Wno-missing-field-initializers

# library include path (header ".h")
INCLUDE_FLAGS += -I/usr/local/include
INCLUDE_FLAGS += -I/usr/include

# library source path (sources ".so, .a")
LDFLAGS += -L/usr/local/lib 
LDFLAGS += -Wl,-Map=$(OBJ_DIR)/$(NAME_MODULE).map
LDFLAGS += -static -static-libgcc -static-libstdc++

# library linking options (specify the library to link)
LDLIBS += -lpthread -lm

#general flag
GENERAL_FLAG += $(OPTIMIZE_OPTION)	\
				$(WARNING_OPTION) \
				$(INCLUDE_FLAGS)	\
				$(DEFINE_OPTIONS)	\

# build flags
CFLAGS += $(GENERAL_FLAG)	\
			-std=c99	\
			-Wno-enum-conversion	\
			-Wno-redundant-decls	\

CXXFLAGS += $(GENERAL_FLAG)	\
			-std=c++11	\
			-fno-rtti	\
			-fno-exceptions	\
			-fno-use-cxa-atexit	\

# object files
OBJECTS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SOURCE_C))
OBJECTS += $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SOURCE_CPP))

.PHONY: all
all: create $(OBJ_DIR)/$(NAME_MODULE)

create:
	@echo "[Compiling sources]"
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	@echo "~ CC $<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	@echo "~ CPP $<"
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)/$(NAME_MODULE): $(OBJECTS)
	@echo "\n[Linking objects]"
	@$(foreach obj,$(OBJECTS),echo "$(notdir $(obj))";)
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)
	@echo "\n[Build completed]"
	@echo "Target: $@"
	@echo "Size: $$(du -h $@ | cut -f1)"
	@echo "Strip size: $$(strip --strip-unneeded $@ && du -h $@ | cut -f1)"
	@echo "MD5: $$(md5sum $@ | cut -d' ' -f1)"
	@echo "SHA1: $$(sha1sum $@ | cut -d' ' -f1)"
	@echo "SHA256: $$(sha256sum $@ | cut -d' ' -f1)"
	@echo "SHA512: $$(sha512sum $@ | cut -d' ' -f1)"
	@echo "Build finished at: $$(date)"
	@echo "----------------------------------------"

.PHONY: clean
clean:
	@rm -rf build
