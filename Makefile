CXX       	?= g++
CC		?= gcc
PREFIX	  	?= /usr
MANPREFIX	?= $(PREFIX)/share/man
APPPREFIX 	?= $(PREFIX)/share/applications
VARS  	  	?=

DEBUG 		?= 1
CXXSTD		?= c++17

# https://stackoverflow.com/a/1079861
# WAY easier way to build debug and release builds
ifeq ($(DEBUG), 1)
        BUILDDIR  = build/debug
        CXXFLAGS := -ggdb3 -Wall -Wextra -Wpedantic -DDEBUG=1 $(DEBUG_CXXFLAGS) $(CXXFLAGS)
else
	# Check if an optimization flag is not already set
	ifneq ($(filter -O%,$(CXXFLAGS)),)
    		$(info Keeping the existing optimization flag in CXXFLAGS)
	else
    		CXXFLAGS := -O3 $(CXXFLAGS)
	endif
        BUILDDIR  = build/release
endif

NAME		= brfucky
TARGET		= $(NAME)
OLDVERSION	= 0.0.0
VERSION    	= 0.0.1
BRANCH     	= $(shell git rev-parse --abbrev-ref HEAD)
SRC 	   	= $(wildcard src/*.cpp src/clipboard/x11/*.cpp src/clipboard/wayland/*.cpp src/clipboard/unix/*.cpp)
OBJ 	   	= $(SRC:.cpp=.o)
LDFLAGS   	+= -L./$(BUILDDIR)/fmt -lfmt -lncurses
CXXFLAGS  	?= -mtune=generic -march=native
CXXFLAGS        += -Wno-unused-parameter -fvisibility=hidden -Iinclude -std=$(CXXSTD) $(VARS) -DVERSION=\"$(VERSION)\" -DBRANCH=\"$(BRANCH)\"

ifeq ($(shell uname -s),Darwin)
	CXXFLAGS += -stdlib=libc++
endif

all: fmt $(TARGET)

fmt:
ifeq ($(wildcard $(BUILDDIR)/fmt/libfmt.a),)
	mkdir -p $(BUILDDIR)/fmt
	make -C src/fmt BUILDDIR=$(BUILDDIR)/fmt CXXSTD=$(CXXSTD)
endif

$(TARGET): fmt $(OBJ)
	mkdir -p $(BUILDDIR)
	$(CXX) $(OBJ) -o $(BUILDDIR)/$(TARGET) $(LDFLAGS)

dist:
	bsdtar -zcf $(NAME)-v$(VERSION).tar.gz LICENSE $(TARGET).1 -C $(BUILDDIR) $(TARGET)

clean:
	rm -rf $(BUILDDIR)/$(TARGET) $(OBJ)

distclean:
	rm -rf $(BUILDDIR) $(OBJ)
	find . -type f -name "*.tar.gz" -exec rm -rf "{}" \;
	find . -type f -name "*.o" -exec rm -rf "{}" \;
	find . -type f -name "*.a" -exec rm -rf "{}" \;

updatever:
	sed -i "s#$(OLDVERSION)#$(VERSION)#g" $(wildcard .github/workflows/*.yml) compile_flags.txt

.PHONY: $(TARGET) updatever dist fmt distclean both install all
