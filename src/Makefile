.PHONY:
include ../common.mk
INCLUDE=-I../include
LIBS=../libs/commonc.a
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
SUBDIRS=
OBJS=string_finder.o string_finder_main.o
TARGETS=string_finder.a string_finder

all: $(SUBDIRS) $(OBJS) $(TARGETS)
string_finder.a: $(OBJS)
	$(AR) $(AR_FLAGS) $@ $^
string_finder: $(OBJS) $(LIBS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^
clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
