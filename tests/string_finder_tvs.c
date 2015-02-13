#include "string_finder_tvs.h"

/* the name of the default input file */
#define UNIFIED_FILE_NAME	"unified"
/* the name of the expected default output file for the string-only mode */
#define UNIFIED_ALONE_OUT_NAME	UNIFIED_FILE_NAME "_alone"
/* the name of the expected default output file for the whole-line mode */
#define UNIFIED_LINE_OUT_NAME	UNIFIED_FILE_NAME "_line"

/* test the default file in string-only mode */
struct string_finder_tv unified_alone = {
	.test_file_name = UNIFIED_FILE_NAME,
	.result_file_name = UNIFIED_ALONE_OUT_NAME,
	.whole_line = 0,
};

/* test the default file in whole-line mode */
struct string_finder_tv unified_line = {
	.test_file_name = UNIFIED_FILE_NAME,
	.result_file_name = UNIFIED_LINE_OUT_NAME,
	.whole_line = 1,
};

/* the name of the fully non-text file */
#define NON_TEXT_FILE_NAME	"non_text"
/* the empty output for any non-text input files */
#define BLANK_OUT_NAME		"blank"
/* test the non-text file in string-only mode */
struct string_finder_tv non_text_alone = {
	.test_file_name = NON_TEXT_FILE_NAME,
	.result_file_name = BLANK_OUT_NAME,
	.whole_line = 0,
};

/* test the non-text file in whole-line mode */
struct string_finder_tv non_text_line = {
	.test_file_name = NON_TEXT_FILE_NAME,
	.result_file_name = BLANK_OUT_NAME,
	.whole_line = 1,
};

/* the name of the file starting with a non-text character */
#define NON_TEXT_START_FILE_NAME	"non_text_start"
/* test the non-text-starting file in string-only mode */
struct string_finder_tv non_text_start_alone = {
	.test_file_name = NON_TEXT_START_FILE_NAME,
	.result_file_name = BLANK_OUT_NAME,
	.whole_line = 0,
};
/* test the non-text-starting file in whole-line mode */
struct string_finder_tv non_text_start_line = {
	.test_file_name = NON_TEXT_START_FILE_NAME,
	.result_file_name = BLANK_OUT_NAME,
	.whole_line = 1,
};

/* the name of the file ending with a non-text character */
#define NON_TEXT_END_FILE_NAME	"non_text_end"
/* test the non-text-ending file in string-only mode */
struct string_finder_tv non_text_end_alone = {
	.test_file_name = NON_TEXT_END_FILE_NAME,
	.result_file_name = BLANK_OUT_NAME,
	.whole_line = 0,
};
/* test the non-text-ending file in whole-line mode */
struct string_finder_tv non_text_end_line = {
	.test_file_name = NON_TEXT_END_FILE_NAME,
	.result_file_name = BLANK_OUT_NAME,
	.whole_line = 1,
};

struct string_finder_tv *string_finder_tvs[N_STRING_FINDER_TVS] = {
	&unified_alone, &unified_line,
	&non_text_alone, &non_text_line,
	&non_text_start_alone, &non_text_start_line,
	&non_text_end_alone, &non_text_end_line
};
