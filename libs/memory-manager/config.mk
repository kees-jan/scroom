# Remove the comment to create a DSO out of the files *not* containing
# a main() function - common1.c and common2.c
SONAME=libsmemman.so.0

PACKAGES=glib-2.0 # gmodule-2.0 gtk+-2.0 cairo

CXXFLAGS += -I ../../inc $(shell pkg-config --cflags $(PACKAGES))
LDFLAGS += $(shell pkg-config --libs $(PACKAGES))

LIBDIR := ../../_lib
