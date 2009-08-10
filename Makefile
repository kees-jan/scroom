SUBDIRS=plugins libs

include master-dir.mk

all: gui

plugins: libs

gui:
	$(Q)echo "Don't forget to build gui/scroom separately!"
	$(Q)echo "It is not included (yet) in the overall build"

.PHONY: gui