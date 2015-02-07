#include "string_finder_tvs.h"

#include <logger.h>

#include <string.h>
#include <stdio.h>

/* the file separator between the directory and the file name */
#define FILE_SEPARATOR	'/'
/*
 * Open a file to read, given the parent directory name and the file name.
 * dir:		the directory directly containing the file
 * fname:	the name of the file inside the directory
 * returns	the input file stream, or
 *		NULL on "error", with errno set by "fopen"
 */
static FILE *open_read_file(const char *dir, const char *fname)
{
	size_t dir_len = strlen(dir);
	size_t full_fname_len = strlen(fname) + 1;
	char full_path[dir_len + 1 + full_fname_len];
	char *fname_start = full_path + dir_len + 1;
	FILE *read_file;

	memcpy(full_path, dir, dir_len);
	full_path[dir_len] = FILE_SEPARATOR;
	memcpy(fname_start, fname, full_fname_len);

	if ((read_file = fopen(full_path, "r")) == NULL) {
		printlg(ERROR_LEVEL, "Failed to open file %s for reading.\n",
			full_path);
	}

	return read_file;
}

#include <string_finder.h>
#include <compare_files.h>

/*
 * Given all of the input file streams,
 * run the test on "find_string_lines" or "find_strings".
 * output_storage:	the input/output stream to which to write,
 *			and to compare to the expected output
 * tv:			test vector containing the file to read
 * expected_output:	stores the expected output
 * returns		1 if passed, 0 otherwise
 */
static int _test_string_finder(FILE *output_storage,
			       struct string_finder_tv *tv,
			       FILE *expected_output)
{
	int error;

	if (tv->whole_line) {
		error = find_string_lines(output_storage, tv->test_file_name);
	} else {
		error = find_strings(output_storage, tv->test_file_name);
	}

	if (error) {
		printlg(ERROR_LEVEL, "Failed to perform line reading test.\n");
		return 0;
	}

	rewind(output_storage);
	if (!files_equal(output_storage, expected_output)) {
		printlg(ERROR_LEVEL,
			"Expected list of strings does not match output.\n");
		return 0;
	}

	return 1;
}

/*
 * the directory containing the expected outputs,
 * relative to "single_test_files"
 */
#define OUTPUTS_DIR	"../test_outputs"

/*
 * Open the expected output file,
 * create a temporary file stream to store the output,
 * and run a test.
 * tv:		the test to run.
 *		Contains the name of the input file,
 *		and the name of the file containing the expected output
 * returns	1 if passed, 0 otherwise
 */
static int test_string_finder(struct string_finder_tv *tv)
{
	FILE *expected_output = open_read_file(OUTPUTS_DIR,
					       tv->result_file_name);
	FILE *output_storage;
	int passed;

	if (expected_output == NULL) {
		printlg(ERROR_LEVEL,
			"Failed to open file containing expected outputs.\n");
		return -1;
	}

	passed = 0;
	if ((output_storage = tmpfile()) == NULL) {
		printlg(ERROR_LEVEL,
			"Failed to open temporary file to store output.\n");
	} else {
		passed = _test_string_finder(output_storage,
					     tv, expected_output);
		fclose(output_storage);
	}

	fclose(expected_output);
	return passed;
}

#include <unistd.h>
/*
 * the directory containing the input files.
 * The program will move into this directory.
 */
#define INPUTS_DIR	"single_test_files"
/*
 * Move into the directory containing all the test inputs, and run the tests.
 */
static void test_string_finders()
{
	unsigned n_failures;
	unsigned tv_i;

	if (chdir(INPUTS_DIR)) {
		printlg(ERROR_LEVEL, "Failed to switch to directory %s.\n",
			INPUTS_DIR);
	}

	n_failures = 0;

	for (tv_i = 0; tv_i < N_STRING_FINDER_TVS; tv_i++) {
		printlg(INFO_LEVEL, "Running string finder test %u.\n", tv_i);
		if (test_string_finder(string_finder_tvs[tv_i])) {
			printlg(INFO_LEVEL, "Passed!\n");
		} else {
			printlg(ERROR_LEVEL, "Failed!\n");
			n_failures++;
		}
	}

	if (n_failures > 0) {
		printlg(ERROR_LEVEL, "Failed %u / %u tests!\n",
			n_failures, N_STRING_FINDER_TVS);
	} else {
		printlg(INFO_LEVEL, "All tests passed!\n");
	}
}

int main(void)
{
	test_string_finders();

	return 0;
}
