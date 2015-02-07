#include "string_finder_tvs.h"

/* the name of the input file */
#define UNIFIED_FILE_NAME	"unified"
/* the name of the expected output file for the string-only mode */
#define UNIFIED_ALONE_OUT_NAME	UNIFIED_FILE_NAME "_alone"
/* the name of the expected output file for the whole-line mode */
#define UNIFIED_LINE_OUT_NAME	UNIFIED_FILE_NAME "_line"

/* test the string-only mode */
struct string_finder_tv unified_alone = {
	.test_file_name = UNIFIED_FILE_NAME,
	.result_file_name = UNIFIED_ALONE_OUT_NAME,
	.whole_line = 0,
};

/* test the whole-line mode */
struct string_finder_tv unified_line = {
	.test_file_name = UNIFIED_FILE_NAME,
	.result_file_name = UNIFIED_LINE_OUT_NAME,
	.whole_line = 1,
};

struct string_finder_tv *string_finder_tvs[N_STRING_FINDER_TVS] = {
	&unified_alone, &unified_line
};
