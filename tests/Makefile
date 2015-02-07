.PHONY:
include ../common.mk
INCLUDE=-I../include
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
LIBS_DIR=../libs/
MAIN_ARCHIVE=../src/string_finder.a
SUBDIRS=
STRING_FINDER_TEST_OBJS=string_finder_tvs.o test_string_finder.o
OBJS=$(STRING_FINDER_TEST_OBJS)
TARGETS=test_string_finder
all: $(SUBDIRS) $(OBJS) $(TARGETS)
test_string_finder: $(STRING_FINDER_TEST_OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(MAIN_ARCHIVE) $(LIBS_DIR)commonc.a $(LIBS_DIR)line_gen.a
clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
