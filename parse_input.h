#ifndef parse_input
#define parse_input


typedef enum action {
	print_version_info, print_detailed_help, print_condense_help,
	list_content, list_content_detailed, list_content_detailed_human_readable,
	display_metadata, extract_content, insert_content, replace_content,
} Action;

typedef enum error {
	no_error, error_too_few_arguments, error_no_instructions, error_invalid_instructions,
	error_invalid_index, error_too_many_indexes, error_invalid_index_not_sorted, error_invalid_index_duplicates,
	error_no_such_file, error_no_such_directory, error_file_too_big,
} Error;

typedef struct instruction {
	Action action;
	Error error;
	char* erroneous_argument;
	char* archive_path;
	char* output_directory;
	char** input_files;
	int number_of_indexes;
	int indexes[4096];
} Instruction;


Instruction parse_instructions(int argc, char **argv);

#endif // parse_input
