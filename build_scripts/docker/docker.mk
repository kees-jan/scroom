all:
.PHONY: all

AUTOCONF_HOST=trusty

########################################################################
Q=@

echo-build-step = +@printf "[%-5s] %s\n" $1 $2

########################################################################

ifeq ($(TEST),)
docker-pull = $(Q)docker pull kjdijkzeul/scroom:$(1)-build-env > /dev/null
docker-build = $(Q)docker run -v $$$$(pwd):/home/build/scroom --rm kjdijkzeul/scroom:$(1)-build-env sh -c "cd \$$$$(mktemp -d) && /home/build/scroom/configure && make -j4 check"
docker-autoconf = $(Q)docker run -v $$(pwd):/home/build/scroom --rm kjdijkzeul/scroom:$(AUTOCONF_HOST)-build-env sh -c "cd /home/build/scroom && autogen -i"
else
docker-pull = $(Q)sleep 2
docker-build = $(Q)sleep 3
docker-autoconf = $(Q)true
endif

define pull-one
$(1)-docker:
	$(call echo-build-step,PULL,$(1))
	$(call docker-pull,$(1))
.PHONY: $(1)-docker
endef

define build-one
$(1)-build: configure | $(1)-docker
	$(call echo-build-step,BUILD,$(1))
	$(call docker-build,$(1))
.PHONY: $(1)-build
all: $(1)-build
endef

$(foreach target,$(TARGETS),$(eval $(call pull-one,$(target))))
$(foreach target,$(TARGETS),$(eval $(call build-one,$(target))))

ifeq ($(wildcard configure),) # If configure doesn't yet exist
configure: | $(AUTOCONF_HOST)-docker
	$(call echo-build-step,AUTOCONF,"on $(AUTOCONF_HOST)")
	$(docker-autoconf)

ifeq ($(filter $(AUTOCONF_HOST),$(TARGETS)),) # if AUTOCONF_HOST is in the TARGETS
$(eval $(call pull-one,$(AUTOCONF_HOST)))
BUILD_TARGETS=$(TARGETS)
PULL_TARGETS=$(TARGETS) $(AUTOCONF_HOST)
else # if AUTOCONF_HOST is in the TARGETS (else branch, AUTOCONF_HOST not in TARGETS)
BUILD_TARGETS=$(filter-out $(AUTOCONF_HOST),$(TARGETS)) $(AUTOCONF_HOST)
PULL_TARGETS=$(BUILD_TARGETS)
endif # if AUTOCONF_HOST is in the TARGETS (endif)
else # If configure doesn't yet exist (else branch, configure exists)
BUILD_TARGETS=$(TARGETS)
PULL_TARGETS=$(TARGETS)
endif # If configure doesn't yet exist (endif)

########################################################################
define generate-one-pull-dependency
$(1)-docker: | $(2)-docker
endef
define generate-one-build-dependency
$(1)-build:  | $(2)-build
endef
define recurse-and-call
$(if $(wordlist 3,$(words $2),$2),$(call recurse-and-call,$1,$(wordlist 2,$(words $2),$2)))
$(if $(and $(word 1,$2),$(word 2,$2)),$(call $1,$(word 1,$2),$(word 2,$2)))
endef

$(eval $(call recurse-and-call,generate-one-pull-dependency,$(PULL_TARGETS)))
$(eval $(call recurse-and-call,generate-one-build-dependency,$(BUILD_TARGETS)))

########################################################################
