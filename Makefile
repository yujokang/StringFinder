.PHONY:libs src tests
include common.mk
INCLUDE=-Iinclude
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
SUBDIRS=libs src tests
OBJS=
TARGETS=
all: $(SUBDIRS) $(OBJS) $(TARGETS)
libs:
	$(MAKE) -C libs
src:
	$(MAKE) -C src
tests:
	$(MAKE) -C tests
clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
