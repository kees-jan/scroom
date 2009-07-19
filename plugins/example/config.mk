# Remove the comment to create a DSO out of the files *not* containing
# a main() function - common1.c and common2.c
SONAME=libspexample.so.0

CXXFLAGS += -I ../../inc $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs gmodule-2.0)

LIBDIR := ../../_lib