DIRECTORY = $(sort $(realpath $(wildcard ./plugins/*/.)))

gen: dpf/utils/lv2_ttl_generator
	@$(CURDIR)/dpf/utils/generate-ttl.sh
ifeq ($(MACOS),true)
	@$(CURDIR)/dpf/utils/generate-vst-bundles.sh
endif

%:
	for SRCDIR in $(DIRECTORY) ; do \
		export SRCDIR ; \
		echo $$SRCDIR ; \
		$(MAKE) -f ./Makefile.project.mk $(MAKECMDGOALS) ; \
	done