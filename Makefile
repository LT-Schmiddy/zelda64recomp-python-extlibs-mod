BUILD_ROOT := build
MOD_BUILD_DIR := build/mod
TESTS_BUILD_DIR := build/tests
MOD_TOML := ./mod.toml
TESTS_TOML := ./tests.toml
EXTLIB_PREFIX := lib
ASSETS_EXTRACTED_DIR ?= assets_extracted
ASSETS_INCLUDE_DIR ?= assets_extracted/assets

ifeq ($(OS),Windows_NT)
PYTHON_EXEC ?= python
else
PYTHON_EXEC ?= python3
endif
PYTHON_FUNC_MODULE := make_python_functions

define get_python_func_no_build_info
$(shell $(PYTHON_EXEC) -c "import $(PYTHON_FUNC_MODULE); $(PYTHON_FUNC_MODULE).ModInfo(\"$(MOD_TOML)\", \"$(MOD_BUILD_DIR)\").$(1)($(2))")
endef

EXTLIB_NAME := $(call get_python_func_no_build_info,get_extlib_name,)

# Extlib Building Info:
EXTLIB_CMAKE_PRESET_GROUP := Debug
ZIG_WINDOWS_CONFIGURE_PRESET := $(call get_python_func_no_build_info,get_extlib_windows_configure_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)
ZIG_MACOS_CONFIGURE_PRESET := $(call get_python_func_no_build_info,get_extlib_macos_configure_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)
ZIG_LINUX_CONFIGURE_PRESET := $(call get_python_func_no_build_info,get_extlib_linux_configure_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)
NATIVE_CMAKE_CONFIGURE_PRESET ?= $(call get_python_func_no_build_info,get_extlib_native_configure_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)

ZIG_WINDOWS_BUILD_PRESET := $(call get_python_func_no_build_info,get_extlib_windows_build_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)
ZIG_MACOS_BUILD_PRESET := $(call get_python_func_no_build_info,get_extlib_macos_build_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)
ZIG_LINUX_BUILD_PRESET := $(call get_python_func_no_build_info,get_extlib_linux_build_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)
NATIVE_CMAKE_BUILD_PRESET ?= $(call get_python_func_no_build_info,get_extlib_native_build_preset,\"$(EXTLIB_CMAKE_PRESET_GROUP)\",)

ifeq ($(OS),Windows_NT)
MOD_TOOL_ZIG_TRIPLET ?= x86_64-windows
NATIVE_SUBDIR := lib
NATIVE_EXTENSION := dll
else ifneq ($(shell uname),Darwin)
MOD_TOOL_ZIG_TRIPLET ?= x86_64-linux
NATIVE_SUBDIR := lib
NATIVE_EXTENSION := so
else
MOD_TOOL_ZIG_TRIPLET ?= aarch64-macos
NATIVE_SUBDIR := lib
NATIVE_EXTENSION := dylib
endif

# MOD_TOOL_ZIG_TRIPLET ?= $(MOD_TOOL_ZIG_TRIPLET)

define extlib_build_file
$(BUILD_ROOT)/$(1)/$(2)/$(EXTLIB_PREFIX)$(EXTLIB_NAME).$(3)
endef

define native_extlib_build_file
$(BUILD_ROOT)/$(1)/$(2)/$(EXTLIB_NAME).$(3)
endef

EXTLIB_BUILD_WIN := $(call extlib_build_file,$(ZIG_WINDOWS_CONFIGURE_PRESET),lib,dll)
EXTLIB_BUILD_MACOS := $(call extlib_build_file,$(ZIG_MACOS_CONFIGURE_PRESET),lib,dylib)
EXTLIB_BUILD_LINUX := $(call extlib_build_file,$(ZIG_LINUX_CONFIGURE_PRESET),lib,so)
EXTLIB_BUILD_NATIVE := $(call native_extlib_build_file,$(NATIVE_CMAKE_CONFIGURE_PRESET),$(NATIVE_SUBDIR),$(NATIVE_EXTENSION))

# Python Build Info:
define call_python_func
	$(PYTHON_EXEC) -c "import $(PYTHON_FUNC_MODULE); $(PYTHON_FUNC_MODULE).ModInfo(\"$(MOD_TOML)\", \"$(MOD_BUILD_DIR)\").set_tests_info(\"$(TESTS_TOML)\", \"$(TESTS_BUILD_DIR)\").set_extlib_info(\"$(EXTLIB_BUILD_WIN)\", \"$(EXTLIB_BUILD_MACOS)\", \"$(EXTLIB_BUILD_LINUX)\", \"$(EXTLIB_BUILD_NATIVE)\").$(1)($(2))"
endef

define get_python_func
$(shell $(PYTHON_EXEC) -c "import $(PYTHON_FUNC_MODULE); $(PYTHON_FUNC_MODULE).ModInfo(\"$(MOD_TOML)\", \"$(MOD_BUILD_DIR)\").set_tests_info(\"$(TESTS_TOML)\", \"$(TESTS_BUILD_DIR)\").set_extlib_info(\"$(EXTLIB_BUILD_WIN)\", \"$(EXTLIB_BUILD_MACOS)\", \"$(EXTLIB_BUILD_LINUX)\", \"$(EXTLIB_BUILD_NATIVE)\").$(1)($(2))")
endef

define get_python_val
$(shell $(PYTHON_EXEC) -c "import $(PYTHON_FUNC_MODULE); print($(PYTHON_FUNC_MODULE).ModInfo(\"$(MOD_TOML)\", \"$(MOD_BUILD_DIR)\").set_tests_info(\"$(TESTS_TOML)\", \"$(TESTS_BUILD_DIR)\").set_extlib_info(\"$(EXTLIB_BUILD_WIN)\", \"$(EXTLIB_BUILD_MACOS)\", \"$(EXTLIB_BUILD_LINUX)\", \"$(EXTLIB_BUILD_NATIVE)\").$(1))")
endef

# Get the mod code compilers from a config.
CC      := $(call get_python_func,get_mod_compiler,)
LD      := $(call get_python_func,get_mod_linker,)


# Recomp Tools Building Info:
N64RECOMP_DIR := N64Recomp
N64RECOMP_BUILD_DIR := $(N64RECOMP_DIR)/build
RECOMP_MOD_TOOL := $(N64RECOMP_BUILD_DIR)/RecompModTool
OFFLINE_MOD_TOOL := $(N64RECOMP_BUILD_DIR)/OfflineModRecomp

# Mod Building Info:
MOD_FILE := $(call get_python_func,get_mod_file,)
$(info MOD_FILE = $(MOD_FILE))
MOD_ELF  := $(call get_python_func,get_mod_elf,)
$(info MOD_ELF = $(MOD_ELF))

MOD_SYMS := $(MOD_BUILD_DIR)/mod_syms.bin
MOD_BINARY := $(MOD_BUILD_DIR)/mod_binary.bin
ZELDA_SYMS := Zelda64RecompSyms/mm.us.rev1.syms.toml
OFFLINE_C_OUTPUT := $(MOD_BUILD_DIR)/mod_offline.c
LDSCRIPT := mod.ld
CFLAGS   := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
			-fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations -fno-builtin-memset \
			-Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable \
			-Wno-missing-braces -Wno-unsupported-floating-point-opt -Werror=section
CPPFLAGS := -nostdinc -D_LANGUAGE_C -DMIPS -DF3DEX_GBI_2 -DF3DEX_GBI_PL -DGBI_DOWHILE -I include -I include/mod -I include/mod/dummy_headers \
			-I include/shared -I mm-decomp/include -I mm-decomp/src -I mm-decomp/extracted/n64-us -I mm-decomp/include/libc \
			-I include_in_dependents -I assets_extracted -I assets_extracted/assets -I assets_extracted/assets/assets
LDFLAGS  := -nostdlib -T $(LDSCRIPT) -Map $(MOD_BUILD_DIR)/mod.map --unresolved-symbols=ignore-all --emit-relocs -e 0 --no-nmagic

MOD_C_SRCS := $(wildcard src/mod/*.c)
MOD_C_OBJS := $(addprefix $(MOD_BUILD_DIR)/, $(MOD_C_SRCS:.c=.o))
MOD_C_DEPS := $(addprefix $(MOD_BUILD_DIR)/, $(MOD_C_SRCS:.c=.d))

# Tests Building Info:
TESTS_FILE := $(call get_python_func,get_tests_file,)
$(info TESTS_FILE = $(TESTS_FILE))
TESTS_ELF  := $(call get_python_func,get_tests_elf,)
$(info TESTS_ELF = $(TESTS_ELF))

TESTS_C_SRCS := $(wildcard src/tests/*.c)
TESTS_C_OBJS := $(addprefix $(TESTS_BUILD_DIR)/, $(TESTS_C_SRCS:.c=.o))
TESTS_C_DEPS := $(addprefix $(TESTS_BUILD_DIR)/, $(TESTS_C_SRCS:.c=.d))

# General Recipes:
# If no extlib is to be built, then don't include it in recipe 'all'
ifeq ($(EXTLIB_NAME),None)
all: mod_nrm tests_nrm runtime
else
all: mod_nrm tests_nrm extlib-all runtime
endif

create_user_build_config:
	$(call call_python_func,create_user_build_config,)

thunderstore:
	$(PYTHON_EXEC) ./create_thunderstore_package.py

native: mod_nrm tests_nrm extlib-native runtime_native

windows: mod_nrm extlib-win runtime

macos: mod_nrm extlib-macos runtime

linux: mod_nrm extlib-linux runtime

runtime:
	$(call call_python_func,copy_to_runtime_dir,)

runtime_native:
	$(call call_python_func,copy_to_runtime_dir_native,)

# Mod Recipes:
mod_nrm: $(MOD_FILE)

$(MOD_FILE): $(RECOMP_MOD_TOOL) $(MOD_ELF) 
	$(RECOMP_MOD_TOOL) $(MOD_TOML) $(MOD_BUILD_DIR)

offline: mod_nrm
	$(OFFLINE_MOD_TOOL) $(MOD_SYMS) $(MOD_BINARY) $(ZELDA_SYMS) $(OFFLINE_C_OUTPUT)

mod_elf: $(MOD_ELF) 

$(MOD_ELF): $(MOD_C_OBJS) $(LDSCRIPT) | $(MOD_BUILD_DIR) $(ASSETS_INCLUDE_DIR)
	$(LD) $(MOD_C_OBJS) $(LDFLAGS) -o $@


$(MOD_C_OBJS): $(MOD_BUILD_DIR)/%.o : %.c | $(ASSETS_INCLUDE_DIR) $(MOD_BUILD_DIR) $(MOD_BUILD_DIR)/src $(MOD_BUILD_DIR)/src/mod
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -DRECOMP_PY_BUILD_MODE -MMD -MF $(@:.o=.d) -c -o $@


# Test Recipes
tests_nrm: $(TESTS_FILE)

$(TESTS_FILE): $(RECOMP_MOD_TOOL) $(TESTS_ELF) 
	$(RECOMP_MOD_TOOL) $(TESTS_TOML) $(TESTS_BUILD_DIR)

tests_elf: $(TESTS_ELF) 

$(TESTS_ELF): $(TESTS_C_OBJS) $(LDSCRIPT) | $(TESTS_BUILD_DIR) $(ASSETS_INCLUDE_DIR)
	$(LD) $(TESTS_C_OBJS) $(LDFLAGS) -o $@

$(TESTS_C_OBJS): $(TESTS_BUILD_DIR)/%.o : %.c | $(ASSETS_INCLUDE_DIR) $(TESTS_BUILD_DIR) $(TESTS_BUILD_DIR)/src $(TESTS_BUILD_DIR)/src/tests
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@

# Recomp Tools Recipes:
$(RECOMP_MOD_TOOL): $(N64RECOMP_BUILD_DIR) 
	cmake -DCMAKE_TOOLCHAIN_FILE="../zig_toolchain.cmake" -DZIG_TARGET="$(MOD_TOOL_ZIG_TRIPLET)" -G Ninja \
		-DCMAKE_BUILD_TYPE=Release -S $(N64RECOMP_DIR) -B $(N64RECOMP_BUILD_DIR) 
	cmake --build $(N64RECOMP_BUILD_DIR)

# Extlib Recipes:
extlib-all: extlib-win extlib-macos extlib-linux

extlib-win:
	cmake --preset=$(ZIG_WINDOWS_CONFIGURE_PRESET) -DLIB_NAME=$(EXTLIB_NAME) .
	cmake --build --preset=$(ZIG_WINDOWS_BUILD_PRESET)

extlib-macos:
	cmake --preset=$(ZIG_MACOS_CONFIGURE_PRESET) -DLIB_NAME=$(EXTLIB_NAME) .
	cmake --build --preset=$(ZIG_MACOS_BUILD_PRESET)

extlib-linux:
	cmake --preset=$(ZIG_LINUX_CONFIGURE_PRESET) -DLIB_NAME=$(EXTLIB_NAME) .
	cmake --build --preset=$(ZIG_LINUX_BUILD_PRESET)

extlib-native:
	cmake --preset=$(NATIVE_CMAKE_CONFIGURE_PRESET) -DLIB_NAME=$(EXTLIB_NAME) .
	cmake --build --preset=$(NATIVE_CMAKE_BUILD_PRESET)

# Misc Recipes:
$(ASSETS_INCLUDE_DIR):
	$(call call_python_func,create_asset_archive,\"$(ASSETS_INCLUDE_DIR)\")

$(N64RECOMP_BUILD_DIR) $(MOD_BUILD_DIR) $(MOD_BUILD_DIR)/src $(MOD_BUILD_DIR)/src/mod $(TESTS_BUILD_DIR) $(TESTS_BUILD_DIR)/src $(TESTS_BUILD_DIR)/src/tests :
ifeq ($(OS),Windows_NT)
	mkdir $(subst /,\,$@)
else
	mkdir -p $@
endif

distclean:
ifeq ($(OS),Windows_NT)
	- rmdir "$(MOD_BUILD_DIR)" /s /q
	- rmdir "$(N64RECOMP_BUILD_DIR)" /s /q
	- rmdir "$(ASSETS_EXTRACTED_DIR)" /s /q
else
	- rm -rf $(MOD_BUILD_DIR)
	- rm -rf $(N64RECOMP_BUILD_DIR)
	- rm -rf $(ASSETS_EXTRACTED_DIR)
endif

clean:
ifeq ($(OS),Windows_NT)
	- rmdir "$(MOD_BUILD_DIR)" /s /q
else
	- rm -rf $(MOD_BUILD_DIR)
endif

-include $(MOD_C_DEPS)
-include $(TESTS_C_DEPS)

.PHONY: all native windows macos linux runtime mod_nrm offline extlib-all extlib-win extlib-macos extlib-linux extlib-native clean clean-build
