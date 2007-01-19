ROOTDIR = $(PWD)

include makefiles/platforms.mk

LIBDIR = zlibrary
APPDIRS = fbreader GeometricCalculator SampleApplications/0 SampleApplications/1

all:
	@for dir in $(LIBDIR) $(APPDIRS); do \
		if [ -d $$dir ]; then \
			cd $$dir; \
			if ! $(MAKE); then \
				exit 1; \
			fi; \
			cd $(ROOTDIR); \
		fi; \
	done;

install: all do_install

do_install:
	@for dir in $(LIBDIR) $(APPDIRS); do \
		if [ -d $$dir ]; then \
			cd $$dir; make $@; cd $(ROOTDIR); \
		fi; \
	done

packages: all
	@for dir in $(LIBDIR) $(APPDIRS); do \
		if [ -d $$dir ]; then \
			cd $$dir; make package; cd $(ROOTDIR); \
		fi; \
	done

clean:
	@for dir in $(LIBDIR) $(APPDIRS); do \
		if [ -d $$dir ]; then \
			cd $$dir; make $@; cd $(ROOTDIR); \
		fi; \
	done

distclean: clean
	rm -vf *.tgz *.ipk *.deb *.prc *.log

archives: clean
