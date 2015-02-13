/*
 * definition and declaration of test vector struct
 * for testing "find_strings" and "find_string_lines" on single files.
 */
/* a test vector for one of the string searching functions */
struct string_finder_tv {
	/*
	 * the name of file in which to search
	 * This name will be relative to the test input file directory,
	 * "single_test_files", in which the tests will run.
	 */
	char *test_file_name;
	/*
	 * the name of the file containing the expected output
	 * This name will be relative to the test output file directory,
	 * "test_outputs".
	 */
	char *result_file_name;
	/* Will we run "find_string_lines"? If not, run "find_strings". */
	int whole_line;
};

#define N_STRING_FINDER_TVS	8
/* the test vectors that will be run by "test_string_finders" */
extern struct string_finder_tv *string_finder_tvs[N_STRING_FINDER_TVS];
