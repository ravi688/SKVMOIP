#-------------------------------------------
#		Requirements
#-------------------------------------------
#	1. mcpp: http://mcpp.sourceforge.net/
#	2. dot or graphviz: https://graphviz.org/
#	3. uncrustify: https://github.com/uncrustify/uncrustify
#	4. ar
#	5. gcc
#-------------------------------------------

#-------------------------------------------
#		Project Configuration
#-------------------------------------------
PROJECT_NAME = SKVMOIP
STATIC_LIB_NAME = skvmoip.a
DYNAMIC_LIB_NAME = skvmoip.dll
EXECUTABLE_NAME = main
MAIN_SOURCE_LANG = cpp
MAIN_SOURCES=main.cpp main.client.cpp main.server.cpp
EXCLUDE_SOURCES=
EXTERNAL_INCLUDES = -I./external-dependencies/
EXTERNAL_LIBS = -lws2_32 -lole32 -loleaut32 -lmfreadwrite -lmfplat -lmf -lmfuuid -lgdi32 -lwmcodecdspuuid
BUILD_DEFINES=

EXTERNAL_SERVER_INCLUDES=
EXTERNAL_SERVER_LIBS=-L./external-dependencies/x264  -lx264 -lx264

EXTERNAL_GTK_INCLUDES=$(shell pkg-config gtk+-3.0 --cflags)
EXTERNAL_GTK_LIBS=$(shell pkg-config gtk+-3.0 --libs)
EXTERNAL_CLIENT_INCLUDES=$(EXTERNAL_GTK_INCLUDES) \
						-I./external-dependencies/NvidiaCodec/include/ \
						-I"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.3/include/"
EXTERNAL_CLIENT_LIBS=$(EXTERNAL_GTK_LIBS) \
					-L"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.3/lib/x64" -l:cuda.lib \
					-L./external-dependencies/NvidiaCodec/ -l:nvcuvid.lib -l:nvencodeapi.lib

SERVER_SOURCES=source/Encoder.cpp \
				source/main.server.cpp
CLIENT_SOURCES=source/third_party/NvDecoder.cpp \
				source/Decoder.cpp \
				source/main.client.cpp \
				source/Window.cpp \
				source/Win32/Win32DrawSurface.cpp \
				source/Win32/Win32RawInput.cpp \
				source/Win32/Win32Window.cpp \
				source/HDMIDecoderNetStream.cpp \
				source/HIDUsageID.cpp
TEST_SOURCES=source/main.cpp
GUI_SOURCES=$(wildcard source/GUI/*.cpp)
GUITEST_SOURCES=source/main.guitest.cpp

ifeq ($(BUILD),server)
	BUILD_DEFINES+=-DBUILD_SERVER
	EXCLUDE_SOURCES+=$(CLIENT_SOURCES) $(TEST_SOURCES) $(GUITEST_SOURCES) $(GUI_SOURCES)
	EXTERNAL_INCLUDES+=$(EXTERNAL_SERVER_INCLUDES)
	EXTERNAL_LIBS+=$(EXTERNAL_SERVER_LIBS)
endif
ifeq ($(BUILD),client)
	BUILD_DEFINES+=-DBUILD_CLIENT
	EXCLUDE_SOURCES+=$(SERVER_SOURCES) $(TEST_SOURCES) $(GUITEST_SOURCES)
	EXTERNAL_INCLUDES+=$(EXTERNAL_CLIENT_INCLUDES)
	EXTERNAL_LIBS+=$(EXTERNAL_CLIENT_LIBS)
endif
ifeq ($(BUILD),guitest)
	BUILD_DEFINES+=-DBUILD_GUITEST
	EXCLUDE_SOURCES+=$(SERVER_SOURCES) $(CLIENT_SOURCES) $(TEST_SOURCES)
	EXTERNAL_INCLUDES+=$(EXTERNAL_GTK_INCLUDES)
	EXTERNAL_LIBS+=$(EXTERNAL_GTK_LIBS)
endif
ifeq ($(BUILD),)
	BUILD_DEFINES+=-DBUILD_TEST
	EXTERNAL_INCLUDES+=$(EXTERNAL_SERVER_INCLUDES) $(EXTERNAL_CLIENT_INCLUDES)
	EXTERNAL_LIBS+=$(EXTERNAL_SERVER_LIBS) $(EXTERNAL_CLIENT_LIBS)
endif

ifneq ($(OUT),)
	EXECUTABLE_NAME = $(OUT)
endif

DEPENDENCIES = Common Common/dependencies/BufferLib Common/dependencies/BufferLib/dependencies/CallTrace
DEPENDENCY_LIBS = Common/lib/common.a Common/dependencies/BufferLib/lib/bufferlib.a Common/dependencies/BufferLib/dependencies/CallTrace/lib/calltrace.a
DEPENDENCIES_DIR = ./dependencies
SHARED_DEPENDENCIES =
SHARED_DEPENDENCY_LIBS =
SHARED_DEPENDENCIES_DIR = ./shared-dependencies
#-------------------------------------------

#-------------------------------------------
#		Project Initialization and Uitilty commands
#-------------------------------------------
__DEPENDENCIES = $(addprefix $(DEPENDENCIES_DIR)/, $(DEPENDENCIES))
__DEPENDENCY_LIBS = $(addprefix $(DEPENDENCIES_DIR)/, $(DEPENDENCY_LIBS))
__SHARED_DEPENDENCIES = $(addprefix $(SHARED_DEPENDENCIES_DIR)/, $(SHARED_DEPENDENCIES))
__SHARED_DEPENDENCY_LIBS = $(addprefix $(SHARED_DEPENDENCIES_DIR)/, $(SHARED_DEPENDENCY_LIBS))
ifdef COMSPEC
__EXECUTABLE_NAME = $(addsuffix .exe, $(basename $(EXECUTABLE_NAME)))
else
__EXECUTABLE_NAME = $(basename $(EXECUTABLE_NAME))
endif
.PHONY: all
.PHONY: init
all: dgraph release

%.gv:
	echo digraph $(PROJECT_NAME) { $(PROJECT_NAME); } > $@
	@echo [Log] $@ created successfully!

$(DEPENDENCIES_DIR) $(SHARED_DEPENDENCIES_DIR):
	mkdir $(subst /,\,$@)
	@echo [Log] $@ created successfully!


init: $(PROJECT_NAME).gv $(DEPENDENCIES_DIR) $(SHARED_DEPENDENCIES_DIR)
	@echo [Log] $(PROJECT_NAME) init successfully!
#-------------------------------------------


#-------------------------------------------
#		Dependency Graph Generation
#-------------------------------------------
DGRAPH_TARGET = ./dependency_graph/$(PROJECT_NAME).png
DGRAPH_TARGET_DIR = dependency_graph
DGRAPH_SCRIPT = $(PROJECT_NAME).gv
DGRAPH_INCLUDES = $(addprefix -I, $(__DEPENDENCIES) $(__SHARED_DEPENDENCIES))
DGRAPH_COMPILER = dot
DGRAPH_FLAGS = -Tpng

DGRAPH_PREPROCESSOR = mcpp
DGRAPH_PREPROCESSOR_FLAGS = -P


DGRAPH_PREPROCESSED_SCRIPT = $(addsuffix .i, $(DGRAPH_SCRIPT))

.PHONY: dgraph
.PHONY: dgraph-clean

%.gv.i:
	$(DGRAPH_PREPROCESSOR) $(DGRAPH_PREPROCESSOR_FLAGS) $(DGRAPH_INCLUDES) $(basename $@) -o $@

$(DGRAPH_TARGET_DIR):
	mkdir $@

dgraph: $(DGRAPH_PREPROCESSED_SCRIPT) | $(DGRAPH_TARGET_DIR)
	$(DGRAPH_COMPILER) $(DGRAPH_FLAGS) $(DGRAPH_PREPROCESSED_SCRIPT) > $(DGRAPH_TARGET)

dgraph-clean:
	del dependency_graph\$(PROJECT_NAME).png
	rmdir dependency_graph
	del $(PROJECT_NAME).gv.i
	@echo [Log] Dependency graphs cleaned successfully!
#-------------------------------------------

#-------------------------------------------
#
#-------------------------------------------



#-------------------------------------------
#		Binary Generation
#-------------------------------------------
TARGET_LIB_DIR = ./lib
TARGET_STATIC_LIB = $(join $(TARGET_LIB_DIR)/, $(STATIC_LIB_NAME))
TARGET_DYNAMIC_LIB = $(join $(TARGET_LIB_DIR)/, $(DYNAMIC_LIB_NAME))
TARGET = $(__EXECUTABLE_NAME)

#Dependencies
DEPENDENCY_INCLUDES = $(addsuffix /include, $(__DEPENDENCIES))
SHARED_DEPENDENCY_INCLUDES = $(addsuffix /include, $(__SHARED_DEPENDENCIES))

MAIN_OBJECT=$(addsuffix .o, $(wildcard $(MAIN_SOURCES)))
INCLUDES= -I./include $(EXTERNAL_INCLUDES) $(addprefix -I, $(DEPENDENCY_INCLUDES) $(SHARED_DEPENDENCY_INCLUDES))
C_SOURCES=$(wildcard source/*.c source/*/*.c)
CPP_SOURCES=$(wildcard source/*.cpp source/*/*.cpp)
SOURCES= $(filter-out $(EXCLUDE_SOURCES), $(C_SOURCES) $(CPP_SOURCES))
OBJECTS= $(addsuffix .o, $(SOURCES))
LIBS = $(EXTERNAL_LIBS)

#Flags and Defines
DEBUG_DEFINES =  -DGLOBAL_DEBUG -DDEBUG -DLOG_DEBUG
RELEASE_DEFINES =  -DGLOBAL_RELEASE -DRELEASE -DLOG_RELEASE
DEFINES = $(BUILD_DEFINES)

COMPILER_FLAGS= -m64
C_COMPILER_FLAGS = $(COMPILER_FLAGS)
CPP_COMPILER_FLAGS = $(COMPILER_FLAGS)
LINKER_FLAGS= -m64
LINKER=g++
DYNAMIC_LIBRARY_COMPILATION_FLAG = -shared
DYNAMIC_IMPORT_LIBRARY_FLAG = -Wl,--out-implib,
C_COMPILER = gcc
CPP_COMPILER = g++
ARCHIVER_FLAGS = -rc
ARCHIVER = ar

ifeq ($(STACK_PROTECT),1)
	COMPILER_FLAGS += -fstack-protector
	LINKER_FLAGS += -fstack-protector
endif

ifeq ($(ADDRESS_SANITIZE),1)
	COMPILER_FLAGS += -fsanitize=address -static-libasan
	LINKER_FLAGS += -fsanitize=address -static-libasan
endif

ifeq ($(SHADOW_CALL_STACK_SANITIZE),1)
	COMPILER_FLAGS += -fsanitize=shadow-call-stack
	LINKER_FLAGS += -fsanitize=shadow-call-stack
endif

ifeq ($(LEAK_SANITIZE),1)
	# no need to add it to COMPILER_FLAGS, as this flag only causes a library linking which overrides the malloc
	LINKER_FLAGS += -fsanitize=leak
endif

ifeq ($(PROF),1)
	COMPILER_FLAGS += -p
	LINKER_FLAGS += -p
endif

ifeq ($(GPROF),1)
	COMPILER_FLAGS += -pg
	LINKER_FLAGS += -pg
endif

ifeq ($(NOOPT),1)
	COMPILER_FLAGS += -O0
	LINKER_FLAGS += -O0
endif

ifeq ($(WARN),1)
	COMPILER_FLAGS += -Wall -Wextra -pedantic
endif

DEBUG_COMPILER_FLAGS= -g #-fsanitize=integer-divide-by-zero // why it is not working on windows 64 bit?
RELEASE_COMPILER_FLAGS= -O3
DEBUG_LINKER_FLAGS= -g #-fsanitize=integer-divide-by-zero  // why it is not working on windows 64 bit?
RELEASE_LINKER_FLAGS= -flto

TARGET_DYNAMIC_IMPORT_LIB = $(addprefix $(dir $(TARGET_DYNAMIC_LIB)), $(addprefix lib, $(notdir $(TARGET_DYNAMIC_LIB).a)))

.PHONY: lib-static
.PHONY: lib-static-debug
.PHONY: lib-static-release
.PHONY: lib-dynamic
.PHONY: lib-dynamic-debug
.PHONY: lib-dynamic-release
.PHONY: lib-static-dynamic
.PHONY: lib-static-dynamic-debug
.PHONY: lib-static-dynamic-release
.PHONY: release
.PHONY: debug
.PHONY: $(TARGET)
.PHONY: bin-clean
.PHONY: PRINT_MESSAGE1

all: release
lib-static: lib-static-release
lib-static-debug: DEFINES += $(DEBUG_DEFINES) -DBUILD_STATIC_LIBRARY
lib-static-debug: __STATIC_LIB_COMMAND = lib-static-debug
lib-static-debug: COMPILER_FLAGS += $(DEBUG_COMPILER_FLAGS)
lib-static-debug: LINKER_FLAGS += $(DEBUG_LINKER_FLAGS)
lib-static-debug: $(TARGET_STATIC_LIB)
lib-static-release: DEFINES += $(RELEASE_DEFINES) -DBUILD_STATIC_LIBRARY
lib-static-release: __STATIC_LIB_COMMAND = lib-static-release
lib-static-release: COMPILER_FLAGS += $(RELEASE_COMPILER_FLAGS)
lib-static-release: LINKER_FLAGS += $(RELEASE_LINKER_FLAGS)
lib-static-release: $(TARGET_STATIC_LIB)

lib-dynamic: lib-dynamic-release
lib-dynamic-debug: DEFINES += $(DEBUG_DEFINES) -DBUILD_DYNAMIC_LIBRARY
lib-dynamic-debug: __STATIC_LIB_COMMAND = lib-static-dynamic-debug
lib-dynamic-debug: COMPILER_FLAGS += $(DEBUG_COMPILER_FLAGS) -fPIC
lib-dynamic-debug: LINKER_FLAGS += $(DEBUG_LINKER_FLAGS) -fPIC
lib-dynamic-debug: $(TARGET_DYNAMIC_LIB)
lib-dynamic-release: DEFINES += $(RELEASE_DEFINES) -DBUILD_DYNAMIC_LIBRARY
lib-dynamic-release: __STATIC_LIB_COMMAND = lib-static-dynamic-release
lib-dynamic-release: COMPILER_FLAGS += $(RELEASE_COMPILER_FLAGS) -fPIC
lib-dynamic-release: LINKER_FLAGS += $(RELEASE_LINKER_FLAGS) -fPIC
lib-dynamic-release: $(TARGET_DYNAMIC_LIB)

.PHONY: lib-dynamic-packed-debug
lib-dynamic-packed-debug: DEFINES += $(DEBUG_DEFINES) -DBUILD_DYNAMIC_LIBRARY
lib-dynamic-packed-debug: __STATIC_LIB_COMMAND = lib-static-dynamic-debug
lib-dynamic-packed-debug: COMPILER_FLAGS += $(DEBUG_COMPILER_FLAGS) -fPIC
lib-dynamic-packed-debug: LINKER_FLAGS += $(DEBUG_LINKER_FLAGS) -fPIC
lib-dynamic-packed-debug: $(TARGET_DYNAMIC_PACKED_LIB)

release: DEFINES += $(RELEASE_DEFINES) -DBUILD_EXECUTABLE
release: __STATIC_LIB_COMMAND = lib-static-release
release: $(TARGET)
debug: DEFINES += $(DEBUG_DEFINES) -DBUILD_EXECUTABLE
debug: __STATIC_LIB_COMMAND = lib-static-debug
debug: COMPILER_FLAGS += $(DEBUG_COMPILER_FLAGS)
debug: LINKER_FLAGS += $(DEBUG_LINKER_FLAGS)
debug: $(TARGET)


%.c.o : %.c
	@echo [Log] Compiling $< to $@
	$(C_COMPILER) $(C_COMPILER_FLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

%.cpp.o: %.cpp
	@echo [Log] Compiling $< to $@
	$(CPP_COMPILER) $(CPP_COMPILER_FLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

%.a:
	@echo [Log] Building $@ ...
	$(MAKE) --directory=$(subst lib/, ,$(dir $@)) $(__STATIC_LIB_COMMAND)
	@echo [Log] $@ built successfully!

$(TARGET_LIB_DIR):
	mkdir $@

PRINT_STATIC_INFO:
	@echo [Log] Building $(TARGET_STATIC_LIB) ...

PRINT_DYNAMIC_INFO:
	@echo [Log] Building $(TARGET_DYNAMIC_LIB) ...

$(TARGET_STATIC_LIB) : PRINT_STATIC_INFO $(filter-out $(MAIN_OBJECT), $(OBJECTS)) | $(TARGET_LIB_DIR)
	$(ARCHIVER) $(ARCHIVER_FLAGS) $@ $(filter-out $<, $^)
	@echo [Log] $@ built successfully!

$(TARGET_DYNAMIC_LIB) : PRINT_DYNAMIC_INFO $(__DEPENDENCY_LIBS) $(__SHARED_DEPENDENCY_LIBS) $(filter-out $(MAIN_OBJECT), $(OBJECTS)) | $(TARGET_LIB_DIR)
	@echo [Log] Linking $@ ...
	$(LINKER) $(LINKER_FLAGS) $(DYNAMIC_LIBRARY_COMPILATION_FLAG) $(filter-out $(MAIN_OBJECT), $(OBJECTS))  $(LIBS)\
	$(addprefix -L, $(dir $(__DEPENDENCY_LIBS) $(__SHARED_DEPENDENCY_LIBS))) \
	$(addprefix -l:, $(notdir $(__DEPENDENCY_LIBS) $(__SHARED_DEPENDENCY_LIBS))) \
	-o $@ $(DYNAMIC_IMPORT_LIBRARY_FLAG)$(TARGET_DYNAMIC_IMPORT_LIB)
	@echo [Log] $@ and lib$(notdir $@.a) built successfully!

$(TARGET): $(__DEPENDENCY_LIBS) $(__SHARED_DEPENDENCY_LIBS) $(TARGET_STATIC_LIB) $(MAIN_OBJECT)
	@echo [Log] Linking $@ ...
	$(LINKER) $(LINKER_FLAGS) $(MAIN_OBJECT) \
	$(addprefix -L, $(dir $(TARGET_STATIC_LIB) $(__DEPENDENCY_LIBS) $(__SHARED_DEPENDENCY_LIBS))) \
	$(addprefix -l:, $(notdir $(TARGET_STATIC_LIB) $(__DEPENDENCY_LIBS) $(__SHARED_DEPENDENCY_LIBS))) $(LIBS) \
	-o $@
	@echo [Log] $(PROJECT_NAME) built successfully!

RM := rm -f
RM_DIR := rm -rf

bin-clean:
	$(RM) $(OBJECTS)
	$(RM) $(MAIN_OBJECT)
	$(RM) $(__EXECUTABLE_NAME)
	$(RM) $(TARGET_STATIC_LIB)
	$(RM) $(TARGET_DYNAMIC_LIB)
	$(RM) $(TARGET_DYNAMIC_IMPORT_LIB)
	$(RM_DIR) $(TARGET_LIB_DIR)
	@echo [Log] Binaries cleaned successfully!
	$(MAKE) --directory=./dependencies/Common clean
# 	$(MAKE) --directory=./shared-dependencies/CallTrace clean
# 	$(MAKE) --directory=./dependencies/HPML clean
# 	$(MAKE) --directory=../../shared-dependencies/HPML clean
#  	$(MAKE) --directory=./dependencies/tgc clean
#-------------------------------------------


#-------------------------------------------
#		Cleaning
#-------------------------------------------

.PHONY: clean-project-internal

clean-project-internal:
	$(MAKE) -f $(addsuffix .makefile, $(PROJECT_NAME)) clean

.PHONY: clean
clean: bin-clean clean-project-internal
	@echo [Log] All cleaned successfully!
#-------------------------------------------



.PHONY: build
.PHONY: build-run
.PHONY: build-release
.PHONY: build-debug
.PHONY: run

.PHONY: build-project-internal-debug
.PHONY: build-project-internal-release

build-project-internal-debug:
	$(MAKE) -f $(addsuffix .makefile, $(PROJECT_NAME)) debug

build-project-internal-release:
	$(MAKE) -f $(addsuffix .makefile, $(PROJECT_NAME)) release

build-release:
	$(MAKE) build-project-internal-release
	$(MAKE) lib-static-release
	$(MAKE) release

build-debug:
	$(MAKE) build-project-internal-debug
	$(MAKE) lib-static-debug
	$(MAKE) debug

build: build-debug

build-run: build
	$(__EXECUTABLE_NAME)

run: build-run
