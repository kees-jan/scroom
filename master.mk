# Build all applications and (optionally) DSO from the c-sources which are
# found in the current directory. You can define customizations in the file
# custom.mk is the same directory as the link to this Makefile.

# By default, the output is rather quiet. Do a "make Q=" to see the details of
# each step whilst producing output as usual. Usefull in debugging this makefile and
# customizations in config.mk
Q=@

# Make sure that the phony target all is the first target in this Makefile, no matter what.
.PHONY: all
all:

# Specify the output locations, where the corresponding files will be created
OBJDIR := _obj
APPDIR := _bin
LIBDIR := _lib

# Identify some used helper applications
CC := gcc
CXX := g++
CD := cd
GREP := grep
LDCONFIG := /sbin/ldconfig
LN := ln
MKDIR := mkdir
RM := rm -rf
SED := sed
TEST := test
TOUCH := touch
TRUE := true

# Add (optional) customizations in config.mk in the same directory of the main Makefile
-include config.mk

# All C-sources in this directory will be compiled. The ones not containing a main() function
# will either be used to build a shared object, or form the foundation for a set of applications.
# The difference is made in config.mk in the usage of SONAME
SOURCES := $(if $(wildcard *.c),$(shell $(GREP) -L '^int main' $(wildcard *.c)))
SOURCES_CXX := $(if $(wildcard *.cc),$(shell $(GREP) -L '^int main' $(wildcard *.cc)))

# The sources which represent an application contain a main() function. The name of the application
# will be derived from the name of the source file whilst linking.
SOURCES_MAIN := $(if $(wildcard *.c),$(shell $(GREP) -l '^int main' $(wildcard *.c)))
SOURCES_MAIN_CXX := $(if $(wildcard *.cc),$(shell $(GREP) -l '^int main' $(wildcard *.cc)))

# A couple of standard CFLAGS and CPPFLAGS each application, library, ... should use
CFLAGS += -Wall -Wextra  -ggdb3
CXXFLAGS += -Wall -Wextra -ggdb3
CPPFLAGS += -D_REENTRANT -DDEBUG -I.

# Make sure everything is rebuild when the Makefile itself changes. The list now contains at least
# Makefile (or whatever the link is named) and config.mk, is one is used.
$(addprefix $(OBJDIR)/,$(SOURCES:.c=.o) $(SOURCES_MAIN:.c=.o) $(SOURCES_CXX:.cc=.o) $(SOURCES_MAIN_CXX:.cc=.o)): $(MAKEFILE_LIST)
$(addprefix $(APPDIR)/,$(SOURCES_MAIN:.c=) $(SOURCES_MAIN_CXX:.cc=)): $(MAKEFILE_LIST)

define echo-build-step
	@printf "[%-5s] %s due to changes in %s\n" $1 $2 "$3"
endef

# Build a DSO out of the files *not* containing a main(), when needed.
ifneq ($(SONAME),)
# Define some derived variables from $(SONAME)
# SONAME = libexample.so.0 (would come from config.mk)
# LINKER_NAME = libexample.so
# LIBRARY_NAME = example
LINKER_NAME=$(shell echo $(SONAME)|sed -rne 's/(lib[^.]*\.so).*/\1/p')
LIBRARY_NAME=$(shell echo $(SONAME)|sed -rne 's/lib([^.]*)\..*/\1/p')

# Make sure the shared object gets build
all: $(LIBDIR)/$(SONAME)

# Library code should be compile with Position Independent Code; extend CFLAGS as such.
# At the same time, use the library specific CFLAGS
$(LIBDIR)/$(SONAME): CFLAGS += -fpic $(CFLAGS_$(SONAME))
$(LIBDIR)/$(SONAME): CXXFLAGS += -fpic $(CXXFLAGS_$(SONAME))
$(LIBDIR)/$(SONAME): CPPFLAGS += $(CPPFLAGS_$(SONAME))

# Tell the linker we're about to build a DSO by extending LDFLAGS, also take the library
# specific LDFLAGS into account
$(LIBDIR)/$(SONAME): LDFLAGS += -shared -Wl,-soname,$(SONAME) -Wl,-E $(LDFLAGS_$(SONAME))

# Build the library out of the objects coming from $(SOURCES) and put the resulting so
# in the right directory.
$(LIBDIR)/$(SONAME): $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o)) $(addprefix $(OBJDIR)/,$(SOURCES_CXX:.cc=.o)) | $(LIBDIR)/.d
	$(call echo-build-step,LD,$@,$?)
	$Q$(CC) $(filter %.o,$^) -o $@ $(LDFLAGS)
	$Q$(LDCONFIG) -n $(LIBDIR)
	$Q$(CD) $(LIBDIR) && $(TEST) ! -e $(LINKER_NAME) && $(LN) -s $(SONAME) $(LINKER_NAME) || $(TRUE)
endif

# call(build-c-app,<.c file containing main()>)
define build-c-app
all: $$(APPDIR)/${1:.c=}
$$(APPDIR)/$(1:.c=): LDFLAGS += $$(LDFLAGS_$(1:.c=))
ifneq ($(SONAME),)
$$(APPDIR)/$(1:.c=): LDFLAGS += -L$(LIBDIR) -l$(LIBRARY_NAME) -Wl,-rpath,$(LIBDIR)
else
$$(APPDIR)/$(1:.c=): $$(addprefix $$(OBJDIR)/,$$(SOURCES:.c=.o))
endif
$$(APPDIR)/$(1:.c=): $(OBJDIR)/$(1:.c=.o) | $$(APPDIR)/.d
	$(call echo-build-step,LD,$$@,$$?)
	$$Q$$(CC) $$(filter %.o,$$^) -o $$@ $$(LDFLAGS)
endef

# call(build-cxx-app,<.cc file containing main()>)
define build-cxx-app
all: $$(APPDIR)/${1:.cc=}
$$(APPDIR)/$(1:.cc=): LDFLAGS += $$(LDFLAGS_$(1:.cc=))
ifneq ($(SONAME),)
$$(APPDIR)/$(1:.cc=): LDFLAGS += -L$(LIBDIR) -l$(LIBRARY_NAME) -Wl,-rpath,$(LIBDIR)
else
$$(APPDIR)/$(1:.cc=): $$(addprefix $$(OBJDIR)/,$$(SOURCES_CXX:.cc=.o))
endif
$$(APPDIR)/$(1:.cc=): $(OBJDIR)/$(1:.cc=.o) | $$(APPDIR)/.d
	$(call echo-build-step,LD,$$@,$$?)
	$$Q$$(CXX) $$(filter %.o,$$^) -o $$@ $$(LDFLAGS)
endef

$(OBJDIR)/%.o:%.c | $(OBJDIR)/.d
	$(call echo-build-step,CC,$@,$?)
	$Q$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ -MD -MP

$(OBJDIR)/%.o:%.cc | $(OBJDIR)/.d
	$(call echo-build-step,CXX,$@,$?)
	$Q$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@ -MD -MP

%/.d:
	$Q$(MKDIR) -p $(@D) && $(TOUCH) $@
.PRECIOUS: %/.d

$(foreach app,$(SOURCES_MAIN),$(eval $(call build-c-app,$(app))))
$(foreach app,$(SOURCES_MAIN_CXX),$(eval $(call build-cxx-app,$(app))))

vpath %.c .
vpath %.cc .
vpath %.h .
vpath %.hh .
vpath %.o $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(addprefix $(OBJDIR)/,$(SOURCES:.c=.d) $(SOURCES_MAIN:.c=.d) $(SOURCES_CXX:.cc=.d) $(SOURCES_MAIN_CXX:.cc=.d))
endif

.PHONY: clean
clean:
	$Q$(RM) $(OBJDIR) $(APPDIR) $(LIBDIR)

.SUFFIXES:
MAKEFLAGS += --no-print-directory
