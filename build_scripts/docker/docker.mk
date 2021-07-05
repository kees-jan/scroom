all:
.PHONY: all

########################################################################
Q=@

echo-build-step = +@printf "[%-5s] %s\n" $1 $2

########################################################################

ifeq ($(TEST),)
docker-pull = $(Q)docker pull ghcr.io/kees-jan/scroom-build-container:$(1) > /dev/null
docker-build = $(Q)docker run -v $$$$(pwd):/home/build/scroom --rm ghcr.io/kees-jan/scroom-build-container:$(1) sh -x -c "D=\$$$$(mktemp -d) && cmake -S /home/build/scroom -B \$$$$D -GNinja && cmake --build \$$$$D && cd \$$$$D && xvfb-run ctest -j \$$$$(nproc)"
else
docker-pull = $(Q)sleep 2
docker-build = $(Q)sleep 3
endif

define pull-one
$(1)-docker:
	$(call echo-build-step,PULL,$(1))
	$(call docker-pull,$(1))
.PHONY: $(1)-docker
endef

define build-one
$(1)-build: | $(1)-docker
	$(call echo-build-step,BUILD,$(1))
	$(call docker-build,$(1))
.PHONY: $(1)-build
all: $(1)-build
endef

$(foreach target,$(TARGETS),$(eval $(call pull-one,$(target))))
$(foreach target,$(TARGETS),$(eval $(call build-one,$(target))))

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

$(eval $(call recurse-and-call,generate-one-pull-dependency,$(TARGETS)))
$(eval $(call recurse-and-call,generate-one-build-dependency,$(TARGETS)))

########################################################################
