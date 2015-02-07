# Test for directory-wide string searching.
# Although results inside a file are always in the same order,
# the files may be traversed in different orders.

# the entries associated with a line in a file
# For "alone" mode, each entry is a separate string.
# For "line" mode, there is at most one entry that includes the whole line. 
class LineStrings:
	# line_number:	the line number,
	#		which will be used for checking equivalence
	#		with other lines
	# first_string:	the first string that will be added to the line
	def __init__(self, line_number, first_string):
		self.strings = [first_string]
		self.line_number = line_number
	# Add a new entry to the line.
	# extra_string:	the new entry to add
	def add_string(self, extra_string):
		self.strings += [extra_string]
	# Check that two lines are the same:
	# They have the same line number and entries.
	# other:	the other line against which to check
	# returns	True if the lines are equal, false otherwise
	def __eq__(self, other):
		if self.line_number != other.line_number:
			print "Unexpected line number. " + \
			      "Expected %d, but got %d."%(self.line_number, \
							  other.line_number)
			return False

		self_size = len(self.strings)
		other_size = len(other.strings)
		if self_size != other_size:
			print "Unequal size. " + \
			      "Expected %d strings, but got %d."%(self_size, \
								  other_size)
			return False

		for string_i in range(self_size):
			self_string = self.strings[string_i]
			other_string = other.strings[string_i]
			if self_string != other_string:
				print "String %d does not match. "%string_i + \
				      "Expected %s, but got %s."%(self_string, \
								  other_string)
				return False
		return True
	def __ne__(self, other):
		return not self == other

# the lines associated with a whole file
class FileStrings:
	# first_line_number:	the number of the first line to add
	# first_string_text:	the first entry to add to the first line
	def __init__(self, first_line_number, first_string_text):
		self.lines = []
		self.last_line = -1
		self.add_line(first_line_number, first_string_text)
	# Add an entry,
	# possibly creating a new line if the line number has changed.
	# line_number:	the number of the line to add
	# string_text:	the entry to add to the line
	def add_line(self, line_number, string_text):
		if self.last_line == line_number:
			self.lines[-1].add_string(string_text)
		else:
			self.last_line = line_number
			self.lines += [LineStrings(line_number, string_text)]
	# Check that two files are equal:
	# They have matching lines.
	# other:	the other file against which to check
	# returns	True if the lines are equal; False otherwise.
	def __eq__(self, other):
		self_size = len(self.lines)
		other_size = len(other.lines)
		if self_size != other_size:
			print "Unequal size. " + \
			      "Expected %d lines, but got %d."%(self_size, \
								other_size)
			return False

		print "\t\tTesting %d lines"%self_size
		for line_i in range(self_size):
			self_line = self.lines[line_i]
			other_line = other.lines[line_i]
			if self_line != other_line:
				print "Failed on line %d"%self_line.line_number
				return False

		return True
	def __ne__(self, other):
		return not self == other

# the files searched in a run of the "string_finder" program
class RunStrings:
	# Add an entry, and possibly a line and file,
	# if the entry is properly formatted.
	# line:	the line in the output, which will be turned into an entry
	#	if it is in the form:
	#	"[file name] ([line number]):\t[entry]"
	def try_add_line(self, line):
		# Split off the entry.
		header_data = line.split("\t", 1)
		if len(header_data) != 2:
			return
		header, data = header_data

		# Split up the file name and line number.
		fname_line = header.split(" ")
		if len(fname_line) != 2:
			print "%s is an improper header"%header
			return
		fname = fname_line[0]
		line = int(fname_line[1][1 : -2])

		if fname == self.last_fname:
			# We are still in the old file.
			self.last_file.add_line(line, data)
		elif self.files.has_key(fname):
			print "File %s already exists!"%fname
			return
		else:
			# We entered a new file.
			self.last_fname = fname
			self.last_file = FileStrings(line, data)
			self.files[fname] = self.last_file
	# result_lines:	the list of output lines representing entries
	def __init__(self, result_lines):
		self.files = {}
		self.last_file = None
		self.last_fname = None

		for line in result_lines:
			if len(line) > 0:
				self.try_add_line(line)
	# Check that two runs of the program are equivalent:
	# They contain the same set of files,
	# although the files may be in different orders.
	# other:	the other run to check against
	def __eq__(self, other):
		self_size = len(self.files)
		other_size = len(other.files)
		if self_size != other_size:
			print "Unequal size. " + \
			      "Expected %d files, but got %d."%(self_size, \
								other_size)
			return False

		for fname in self.files.keys():
			print "\tTesting file %s"%fname
			if not other.files.has_key(fname):
				print "Missing file %s"%fname
				return False
			self_file = self.files[fname]
			other_file = other.files[fname]
			if self_file != other_file:
				print "File %s does not match"%fname
				return False

		return True
	def __ne__(self, other):
		return not self == other

from subprocess import Popen, PIPE

# the command for the "string_finder" program
COMMAND = "../src/string_finder"
# the source directory to scan
SRC_DIR = "dir_test_files/"
# the prefix for both experiments' files containing the expected outputs
OUTPUT_PREFIX = "test_outputs/dir_test_"

# the suffix for the file containing the expected outputs of the test
# in which only strings are shown
ALONE_SUFFIX = "alone"
# the suffix for the file containing the expected outputs of the test
# in which entire lines containing strings are shown
LINE_SUFFIX = "line"

# the option for showing only strings
ALONE_OPTION = "a"
# the option for showing whole lines
LINE_OPTION = "l"
# Run a test, and compare it to the expected values.
# print line:	Do we want to print whole lines?
def run_test(print_line):
	# Determine the option-appropriate values.
	output_suffix = None
	option = None

	if print_line:
		output_suffix = LINE_SUFFIX
		option = LINE_OPTION
	else:
		output_suffix = ALONE_SUFFIX
		option = ALONE_OPTION

	# Open and parse the file containing the expected output.
	expected_output_file = open(OUTPUT_PREFIX + output_suffix, "r")
	expected_lines = expected_output_file.readlines()
	expected_output_file.close()
	expected_run = RunStrings(expected_lines)

	# Read and parse the output of a real execution.
	real_run = Popen([COMMAND, SRC_DIR, option], stdout = PIPE)
	real_lines = real_run.stdout.readlines()
	real_run = RunStrings(real_lines)

	# Check that the outputs are equivalent.
	if expected_run == real_run:
		print "Passed!"
	else:
		print "Failed!"

if __name__ == "__main__":
	print "Running test that only looks for strings"
	run_test(False)
	print "Running test that looks for lines containing strings"
	run_test(True)
