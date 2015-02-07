/* functions for finding and printing strings in a file or directory */
#ifndef STRING_FINDER_H
#define STRING_FINDER_H
#include <stdio.h>

/*
 * Separately print out each instance of strings
 * in the file or the entire directory.
 * out:		the output stream to which to print the strings
 * root_path:	the path to the file or root directory to search
 * returns	0 on success, -1 otherwise.
 */
int find_strings(FILE *out, const char *root_path);
/*
 * Print lines in the file or the entire directory
 * that contain strings.
 * out:		the output stream to which to print the lines
 * root_path:	the path to the file or root directory to search
 * returns	0 on success, -1 otherwise.
 */
int find_string_lines(FILE *out, const char *root_path);

#endif /* STRING_FINDER_H */
