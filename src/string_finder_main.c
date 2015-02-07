#include <string_finder.h>

#include <logger.h>

/* argument index for directory or path name */
#define DIR_INDEX		1
/* always require path name */
#define MIN_N_ARGS		(DIR_INDEX + 1)
/* argument index for optional printing mode */
#define LINE_OPTION_INDEX	MIN_N_ARGS
#define MAX_N_ARGS		(LINE_OPTION_INDEX + 1)

/* option for only printing string */
#define ALONE_OPTION		'a'
/* option for printing whole line containing string */
#define LINE_OPTION		'l'

int main(int argc, char *argv[])
{
	char display_option;

	if (argc < MIN_N_ARGS) {
		printlg(ERROR_LEVEL, "Please enter the path to search.\n");
		return -1;
	}

	if (argc > MAX_N_ARGS) {
		printlg(ERROR_LEVEL,
			"You should only enter the input directory, "
			"and, optionally, the display option.\n");
		return -1;
	}

	if (argc > MIN_N_ARGS) {
		display_option = *argv[LINE_OPTION_INDEX];
	} else {
		display_option = ALONE_OPTION;
	}

	switch (display_option) {
	case ALONE_OPTION:
		return find_strings(stdout, argv[DIR_INDEX]);
	case LINE_OPTION:
		return find_string_lines(stdout, argv[DIR_INDEX]);
	default:
		printlg(ERROR_LEVEL,
			"Invalid display option, \"%c\". "
			"Enter either \"%c\" to display strings only, "
			"or \"%c\" to display the whole line "
			"containing the string.\n",
			display_option, ALONE_OPTION, LINE_OPTION);
		return -1;
	}
}
