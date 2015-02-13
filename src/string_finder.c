#include <string_finder.h>

#include <file_buffer.h>
#include <logger.h>

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>

#define FILE_SEPARATOR	'/'

/*
 * Perform specified action on a regular file, and do not recurse.
 * out:		the output stream for the action
 * path:	the path of the file on which to perform the action
 * file_action:	the actions to perform on the file
 * returns:	0 on success,
 *		-1 on error,
 *		   with "errno" set by "fopen" if opening the file failed,
 *		   or by "file_action"
 */
static int act_on_file(FILE *out, const char *path,
		       int (*file_action)(FILE *out, FILE *in,
					  const char *path))
{
	FILE *entry_file = fopen(path, "r");

	if (entry_file == NULL) {
		printlg(ERROR_LEVEL, "Failed to open file %s.\n", path);
		return -1;
	} else {
		int error = file_action(out, entry_file, path);
		fclose(entry_file);
		return error;
	}
}

/*
 * A file name starting with this character
 * is either the directory itself, or the parent directory,
 * and should be skipped.
 */
#define LOOP_DIR_CHAR '.'
/*
 * Given a real directory, recursively (ie. depth first)
 * perform specified action on files contained in directory.
 * out:			the output stream for the action
 * current_path:	the path of the current directory
 * file_action:		the actions to perform on a normal file
 * current_dir:		the current directory
 * returns		0 on success,
 *			-1 on error, with errno set
 *			   by "opendir" if opening a subdirectory failed,
 *			   or by "act_on_file"
 */
static int _traverse_dir(FILE *out, const char *current_path,
			 int (*file_action)(FILE *out, FILE *in,
					    const char *path),
			 DIR *current_dir)
{
	size_t current_path_len = strlen(current_path);
	char full_path[current_path_len + 1 + NAME_MAX + 1];
	char *next_segment_start = full_path + current_path_len + 1;
	struct dirent *entry;
	int error = 0;

	memcpy(full_path, current_path, current_path_len);
	full_path[current_path_len] = FILE_SEPARATOR;

	while (!error && (entry = readdir(current_dir)) != NULL) {
		DIR *subdir;

		if (entry->d_name[0] == LOOP_DIR_CHAR) {
			continue;
		}

		strncpy(next_segment_start, entry->d_name,
			NAME_MAX + 1);
		subdir = opendir(full_path);
		if (subdir == NULL) {
			if (errno == ENOTDIR) {
				error = act_on_file(out, full_path,
						    file_action);
			} else {
				printlg(ERROR_LEVEL,
					"Failed to open sub directory %s.\n",
					full_path);
				error = -1;
			}
		} else {
			error = _traverse_dir(out, full_path,
					      file_action,
					      subdir);

			if (error < 0) {
				printlg(ERROR_LEVEL,
					"Failed to process subdirectory %s.\n",
					full_path);
			}

			closedir(subdir);
		}
	}

	return error;
}

/*
 * General entry point for performing specified actions on
 * files rooted at a given path, which may be a file, or a directory.
 * out:			the output stream for the action
 * root_path:		the originally-specified path
 * file_action:		the actions to perform on a normal file
 * returns		0 on success,
 *			-1 on error, with errno set
 *			   by "opendir" if opening a subdirectory failed,
 *			   or by "act_on_file"
 */
static int traverse_dir(FILE *out, const char *root_path,
			int (*file_action)(FILE *out, FILE *in,
					   const char *path))
{
	DIR *root_dir = opendir(root_path);
	int ret;

	if (root_dir == NULL) {
		if (errno == ENOTDIR) {
			return act_on_file(out, root_path, file_action);
		}

		printlg(ERROR_LEVEL, "Failed to open root directory, %s.\n",
			root_path);
		return -1;
	}

	ret = _traverse_dir(out, root_path, file_action, root_dir);

	closedir(root_dir);
	return ret;
}

/* marker for the beginning and end of a string */
#define STRING_MARKER	'\"'
/* marker for the beginning and end of a character */
#define CHAR_MARKER	'\''
/*
 * When inside a string,
 * this character indicates that any special meaning of the next character
 * should be ignored. 
 * A real newline is not part of the string,
 * and therefore is not affected by an escape.
 */
#define ESCAPE_MARKER	'\\'
/* character marking the end of a line */
#define LINE_BREAK	'\n'

/* states while reading a line */
enum string_state {
	NO_STRING, /* Cursor is not pointing inside a string yet. */
	/*
	 * Pointer is treating characters in the normal fashion:
	 * a quotation mark indicates the end of the string;
	 * a backslash indicates that the next character's special meaning
	 * will be ignored.
	 */
	NORMAL_CHAR,
	/* Ignore any special meaning of the next character that is read. */
	ESCAPED_CHAR
};

/*
 * Print the location of a line in a file,
 * ie. the file name and the line number, starting from 1.
 * out:		the output stream to which to print the message.
 * file_name:	the name of the file containing the line
 * line_number:	the line number to print
 */
static void print_line_location(FILE *out,
				const char *file_name, size_t line_number)
{
	fprintf(out, "%s (%u):\t", file_name, (unsigned) line_number);
}

/*
 * Check if the file contains non-text characters,
 * which will not print properly on terminal,
 * and rewind the buffer.
 * buffer:	the file buffer to check for non-text characters
 * returns	0 iff all characters are printable or whitespace in ASCII,
 *		1 otherwise
 */
static int has_non_text(file_buffer_t *buffer)
{
	size_t file_size = get_file_size(buffer);
	long position;

	while ((position = ftell_buffer(buffer)) < (long) file_size) {
		int byte_value = fgetc_buffer(buffer);

		if (!(isprint(byte_value) || isspace(byte_value))) {
			return 1;
		}
	}

	rewind_buffer(buffer);
	return 0;
}

/*
 * Perform action on file, which is converted into a buffer,
 * only if all the characters are text characters.
 * out:			the output stream to which to print,
 *			and the first argument for "action"
 * in:			the file input stream to convert to
 *			a buffer from which to read
 *			as the second argument to "action"
 * in_file_name:	the name of the file from which to read,
 *			and the third and final argument to "action"
 * action:		the buffer-reading action to perform
 * returns		0 on success or the file contains non-text characters,
 *			-1 on failure, with errno set by
 *			   "init_file_buffer" if initializing the input buffer
 *			   failed,
 *			   or by "action"
 */
static int do_text_buffer_action(FILE *out, FILE *in, const char *in_file_name,
				 int (*action)(FILE *out, file_buffer_t *buffer,
					       const char *in_file_name))
{
	file_buffer_t buffer;
	int error;

	if (init_file_buffer(&buffer, in)) {
		printlg(ERROR_LEVEL, "Failed to generate buffer.\n");
		return -1;
	}

	if (has_non_text(&buffer)) {
		error = 0;
	} else {
		error = action(out, &buffer, in_file_name);
	}


	destroy_file_buffer(&buffer);
	return error;
}

/*
 * the template for the warning to print if
 * a line was completed without finding the end of a string.
 * Arguments are file name (char *) and line number (unsigned).
 */
#define INCOMPLETE_WARNING	"File %s, line %u " \
				"might not contain a real, complete string.\n"

/*
 * Given a buffer to a file only containing text characters,
 * find and print the separate strings.
 * out:			the output stream to which to print
 * buffer:		the file input buffer in which to search for strings
 * in_file_name:	the name of the file from which to read
 * returns		0 on success,
 *			-1 on failure, with errno set by
 *			   "fgetc_buffer" if the file unexpectedly ended
 */
static int
_find_strings_action(FILE *out, file_buffer_t *buffer, const char *in_file_name)
{
	size_t file_size = get_file_size(buffer);
	size_t line_number = 1;
	enum string_state state = NO_STRING;
	int error = 0;
	char marker_char;

	while (ftell_buffer(buffer) < (long) file_size) {
		int current_input = fgetc_buffer(buffer);
		unsigned char current_char;
		int need_to_print, end_line;

		if (current_input == EOF) {
			printlg(ERROR_LEVEL,
				"Unexpectedly reached end of file %s "
				"while searching for strings.\n",
				in_file_name);
			error = -1;
			break;
		}

		current_char = (char) current_input;

		/*
		 * If we reached the end of the line, always start a new line,
		 * and reset the string state.
		 */
		if (current_char == LINE_BREAK) {
			if (state != NO_STRING) {
				printlg(WARNING_LEVEL, INCOMPLETE_WARNING,
					in_file_name, (unsigned) line_number);
				fprintf(out, "\n");
				state = NO_STRING;
			}
			line_number++;
		}

		need_to_print = 0;
		end_line = 0;
		/*
		 * If we are not in a string, and read a quotation mark,
		 * we have entered the string,
		 * and need to print location of the line,
		 * and the opening quotation mark.
		 */
		if (state == NO_STRING) {
			if (current_char == STRING_MARKER ||
			    current_char == CHAR_MARKER) {
				print_line_location(out,
						    in_file_name, line_number);
				state = NORMAL_CHAR;
				marker_char = current_char;
				need_to_print = 1;
			}
		} else {
			/*
			 * If we are inside a string,
			 * every character needs to be printed,
			 * including escape characters
			 * and the closing quotation mark.
			 */
			if (state == ESCAPED_CHAR) {
				/*
				 * Ignore any special meaning of a character
				 * following the escape character,
				 * and exit the escaping state.
				 */
				state = NORMAL_CHAR;
			} else if (current_char == marker_char) {
				/* Close the string, and end this line. */
				state = NO_STRING;
				end_line = 1;
			} else if (current_char == ESCAPE_MARKER) {
				/* Ignore the meaning of the next character. */
				state = ESCAPED_CHAR;
			}
			need_to_print = 1;
		}
		if (need_to_print) {
			fprintf(out, "%c", current_char);
		}
		if (end_line) {
			debug_assert(need_to_print);
			fprintf(out, "\n");
		}
	}
	fprintf(out, "\n");

	return error;
}

/*
 * Separately print the strings in the file,
 * indicating their file and line number.
 * out:			the output stream to which to print
 * in:			the file input stream in which to search for strings
 * in_file_name:	the name of the file from which to read
 * returns		0 on success or the file contains non-text characters,
 *			-1 on failure, with errno set by
 *			   "init_file_buffer" if initializing the input buffer
 *			   failed,
 *			   or by "fgetc_buffer" if the file unexpectedly ended
 */
static int find_strings_action(FILE *out, FILE *in, const char *in_file_name)
{
	return do_text_buffer_action(out, in, in_file_name,
				     _find_strings_action);
}

int find_strings(FILE *out, const char *root_path)
{
	return traverse_dir(out, root_path, find_strings_action);
}

/*
 * the color by which to mark strings inside quotation marks,
 * including the quotation marks themselves
 */
#define STRING_COLOR		SET_COLOR("31;1")
/*
 * We have found at least one string in the line,
 * and therefore need to print the whole string,
 * with the strings colored.
 * out:			the output stream to which to print
 * buffer:		the buffer from which to read the line characters
 * in_file_name:	the name of the file from which to read
 * line_number:		the line number to print
 * line_start:		the position of the beginning of the line
 *			to which to rewind
 * returns		0 on success,
 *			-1 on failure, with errno set by
 *			   "fseek_buffer" if rewinding
 *			   to the beginning of the line failed,
 *			   or by "fgetc_buffer" if the file unexpectedly ended
 */
static int print_strings_in_line(FILE *out, file_buffer_t *buffer,
				 const char *in_file_name, size_t line_number,
				 long line_start)
{
	enum string_state state;
	char marker_char;
	size_t file_size;

	print_line_location(out, in_file_name, line_number);

	if (fseek_buffer(buffer, line_start, SEEK_SET)) {
		printlg(ERROR_LEVEL,
			"Failed to rewind to beginning of line.\n");
		return -1;
	}

	file_size = get_file_size(buffer);
	state = NO_STRING;
	while (ftell_buffer(buffer) < (long) file_size) {
		int current_input = fgetc_buffer(buffer);
		unsigned char current_char;
		int end_color;

		if (current_input == EOF) {
			printlg(ERROR_LEVEL, "Unexpected end of file "
					     "while in file %s, line %u.\n",
				in_file_name, (unsigned) line_number);
			return -1;
		}

		current_char = (char) current_input;

		/* If we found a line break, we have succeeded. */
		if (current_char == LINE_BREAK) {
			if (state != NO_STRING) {
				printlg(WARNING_LEVEL, INCOMPLETE_WARNING,
					in_file_name, (unsigned) line_number);
				fprintf(out, END_COLOR);
			}
			fprintf(out, "\n");
			return 0;
		}

		end_color = 0;
		/* We have entered the string, and need to print it in color. */
		if (state == NO_STRING) {
			if (current_char == STRING_MARKER ||
			    current_char == CHAR_MARKER) {
				state = NORMAL_CHAR;
				marker_char = current_char;
				fprintf(out, STRING_COLOR);
			}
		} else {
			if (state == ESCAPED_CHAR) {
				/*
				 * The last character was an escape character,
				 * so ignore the meaning of this character.
				 */
				state = NORMAL_CHAR;
			} else if (current_char == marker_char) {
				/*
				 * We have found an unescaped quotation mark,
				 * so stop coloring after printing it.
				 */
				state = NO_STRING;
				end_color = 1;
			} else if (current_char == ESCAPE_MARKER) {
				/* We have found an escape character. */
				state = ESCAPED_CHAR;
			}
		}

		/* Print all characters in the line. */
		fprintf(out, "%c", current_char);
		if (end_color) {
			fprintf(out, END_COLOR);
		}
	}

	/*
	 * It is impossible to finish iterating through the file
	 * without reaching the end of the file or line.
	 */
	debug_assert(0);
	return -1;
}

/*
 * Given a file buffer containing only text characters,
 * read through lines, printing out any that contain strings.
 * out:			the output stream to which to print
 * buffer:		the buffer in which to search for strings
 * in_file_name:	the name of the file from which to read
 * returns		0 on success,
 *			-1 on failure, with errno set by
 *			   "fgetc_buffer" if the file unexpectedly ended,
 *			   or by "print_strings_in_line"
 */
static int _find_string_lines_action(FILE *out, file_buffer_t *buffer,
				     const char *in_file_name)
{
	size_t file_size = get_file_size(buffer);
	size_t line_number = 1;
	int error = 0;
	long line_start;

	while ((line_start = ftell_buffer(buffer)) < (long) file_size) {
		int line_continues = 1;

		while (line_continues && ftell_buffer(buffer) <
		       (long) file_size) {
			int current_input = fgetc_buffer(buffer);
			unsigned char current_char;

			if (current_input == EOF) {
				printlg(ERROR_LEVEL,
					"Unexpected end of file, %s, "
					"while searching line for strings.\n",
					in_file_name);
				line_continues = 0;
				error = -1;
				break;
			}

			if (isprint(current_input)) {
			}

			current_char = (char) current_input;
			if (current_char == LINE_BREAK) {
				/*
				 * Reached the end of the line without
				 * finding a string.
				 */
				line_continues = 0;
			} else if (current_char == STRING_MARKER ||
				   current_char == CHAR_MARKER) {
				/*
				 * Found a string in the line, so print it,
				 * and go to the next line.
				 */
				error = print_strings_in_line(out, buffer,
							      in_file_name,
							      line_number,
							      line_start);

				if (error) {
					break;
				}

				line_continues = 0;
			}
		}
		line_number++;
	}
	fprintf(out, "\n");

	return error;
}

/*
 * Read through lines, printing out any that contain strings.
 * out:			the output stream to which to print
 * in:			the file input stream in which to search for strings
 * in_file_name:	the name of the file from which to read
 * returns		0 on success, or the file contains non-text characters,
 *			-1 on failure, with errno set by
 *			   "fgetc_buffer" if the file unexpectedly ended,
 *			   or by "print_strings_in_line"
 */
static int find_string_lines_action(FILE *out, FILE *in,
				    const char *in_file_name)
{
	return do_text_buffer_action(out, in, in_file_name,
				     _find_string_lines_action);
}

int find_string_lines(FILE *out, const char *root_path)
{
	return traverse_dir(out, root_path, find_string_lines_action);
}
