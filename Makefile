default: release_debug

.PHONY: default release debug all clean

include make-utils/flags.mk
include make-utils/cpp-utils.mk

# Use C++23
$(eval $(call use_cpp23))

CXX_FLAGS += -pthread

LD_FLAGS += -luuid -lssl -lcrypto -ldl

CXX_FLAGS += -isystem budgetwarrior/cpp-httplib -Ibudgetwarrior/include -Ibudgetwarrior/loguru -Ibudgetwarrior/fmt/include

$(eval $(call auto_folder_compile,src))
$(eval $(call auto_folder_compile,src/pages))
$(eval $(call auto_folder_compile,src/api))
$(eval $(call auto_folder_compile,budgetwarrior/src))

AUTO_SRC_FILES := $(filter-out budgetwarrior/src/budget.cpp, $(AUTO_SRC_FILES))
AUTO_CXX_SRC_FILES := $(filter-out budgetwarrior/src/budget.cpp, $(AUTO_CXX_SRC_FILES))

$(eval $(call auto_add_executable,server))

release_debug: release_debug_server
release: release_server
debug: debug_server

all: release release_debug debug

prefix ?= /usr/local
bindir = $(prefix)/bin
mandir = $(prefix)/share/man

clean: base_clean

include make-utils/cpp-utils-finalize.mk
