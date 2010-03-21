# Remove the comment to create a DSO out of the files *not* containing
# a main() function - common1.c and common2.c
SONAME=libspcolormap.so.0

PACKAGES=glib-2.0 gmodule-2.0 gtk+-2.0 cairo

CXXFLAGS += -I ../../inc $(shell pkg-config --cflags $(PACKAGES)) # -I ../tiff
LDFLAGS += $(shell pkg-config --libs $(PACKAGES)) # -lboost_thread-mt # -L ../../_lib/scroom -lsptiff

LIBDIR := ../../_lib/scroom