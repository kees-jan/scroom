Q=@

ifeq ($(SUBDIRS),)
SUBDIRS:=$(shell find * -maxdepth 0 -type d)
endif

all clean: $(SUBDIRS)

$(SUBDIRS):
	$(Q)$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(SUBDIRS)