SUBDIRS=plugins libs
GUI=gui

include master-dir.mk

all: gui

plugins: libs

gui:
	$(Q)if [ ! -e $(GUI)/Makefile ] ; then ( cd $(GUI) && ./autogen.sh ) ; fi && $(MAKE) -C $(GUI)

clean: guiclean

guiclean:
	$(Q)make -C $(GUI) distclean

.PHONY: gui guiclean