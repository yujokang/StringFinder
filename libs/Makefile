.PHONY:
include ../common.mk
INCLUDE=-I../include
CPPFLAGS=$(_CPPFLAGS) $(INCLUDE)
GIT_CMD=git clone
SUBDIRS=
OBJS=
TARGETS=CommonC ./commonc.a ../include/data_structs.h ../include/debug_assert.h ../include/get_random.h ../include/logger.h ../include/permutation.h ../include/xmath.h ../include/file_buffer.h LineGen ./line_gen.a ../include/compare_files.h
all: $(SUBDIRS) $(OBJS) $(TARGETS)
CommonC:
	$(GIT_CMD) https://github.com/yujokang/CommonC.git
LineGen:
	$(GIT_CMD) https://github.com/yujokang/LineGen.git
./commonc.a: CommonC
	$(MAKE) -C CommonC
	cp CommonC//commonc.a ./commonc.a
../include/data_structs.h: CommonC
	cp CommonC/include/data_structs.h ../include/data_structs.h
../include/debug_assert.h: CommonC
	cp CommonC/include/debug_assert.h ../include/debug_assert.h
../include/get_random.h: CommonC
	cp CommonC/include/get_random.h ../include/get_random.h
../include/logger.h: CommonC
	cp CommonC/include/logger.h ../include/logger.h
../include/permutation.h: CommonC
	cp CommonC/include/permutation.h ../include/permutation.h
../include/xmath.h: CommonC
	cp CommonC/include/xmath.h ../include/xmath.h
../include/file_buffer.h: CommonC
	cp CommonC/include/file_buffer.h ../include/file_buffer.h
./line_gen.a: LineGen
	$(MAKE) -C LineGen
	cp LineGen/line_gen.a ./line_gen.a
../include/compare_files.h: LineGen
	cp LineGen/include/compare_files.h ../include/compare_files.h

clean:
	$(RM) $(RM_FLAGS) $(OBJS) $(TARGETS)
