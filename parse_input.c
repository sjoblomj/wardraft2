#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "parse_input.h"
#include "wararchivelib.h"


const int max_indexes = 4096;

long get_file_size(char *path) {
	FILE *fptr = fopen(path, "rb");
	long size = -1;
	if (fptr != NULL) {
		fseek(fptr, 0L, SEEK_END);
		size = ftell(fptr);
		fclose(fptr);
	}
	return size;
}

int file_exists(char *path) {
	return get_file_size(path) != -1;
}

int directory_exists(char *path) {
    DIR *dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        return 0;
    }
}

int get_number_of_files(char *path) {
	int number_of_files = 0;
	int archive_id = 0;
	long hidden_data_size = 0;
	FILE *file = fopen(path, "rb");

	int valid_archive = get_header_data(file, &number_of_files, &archive_id, &hidden_data_size);

	if (file != NULL) {
		fclose(file);
	}
	if (valid_archive == 0) {
		return -1;
	}
	return number_of_files;
}

typedef enum index_range_status {
    ok, too_many_indexes, invalid_index, not_sorted, duplicates
} Index_range_status;

int expand_range(int start, int end, int *numbers, int *count) {
	for (int i = start; i <= end; i++) {
        if (*count >= max_indexes)
            return 1;
        numbers[*count] = i;
        (*count)++;
	}
    return 0;
}

int parse_number(const char *input, int *i) {
	char num_str[10];

	// read the rest of the number
	int j;
	for (j = 0; input[*i] >= '0' && input[*i] <= '9'; (*i)++, j++) {
		num_str[j] = input[*i];
	}
	num_str[j] = '\0';

	// convert string to integer
	return (int) strtol(num_str, NULL, 10);
}

Index_range_status parse_index_range(char *input, char *archive, int *indexes, int *number_of_indexes) {
	int num_count = 0;
	int i = 0;

	// loop through input string and parse numbers and ranges
	while (input[i] != '\0') {
        if (num_count >= max_indexes)
            return too_many_indexes;

		if (input[i] >= '0' && input[i] <= '9') {
			// found a number
			int num = parse_number(input, &i);

            indexes[num_count++] = num;

		} else if (input[i] == '-') {
			// found a range
			i++;
			if (num_count == 0) {
				return invalid_index;
			}
			int start = indexes[num_count - 1];
			int end;
			if (input[i]) {
				end = parse_number(input, &i);
			} else {
				end = get_number_of_files(archive);
			}
			if (start > end) {
				return invalid_index;
			}
			int error = expand_range(start + 1, end, indexes, &num_count);
            if (error)
                return too_many_indexes;
		} else if (input[i] != ',') {
			return invalid_index;
		} else {
			// skip over commas
			i++;
		}
	}

	// verify that the array is sorted
	for (int j = 0; j < num_count - 1; j++) {
		for (int k = j + 1; k < num_count; k++) {
			if (indexes[j] > indexes[k]) {
				return not_sorted;
			}
		}
	}
    // verify that the array contains no duplicates
	for (int j = 1; j < num_count; j++) {
		if (indexes[j] == indexes[j - 1]) {
			return duplicates;
		}
	}
    *number_of_indexes = num_count;
	return ok;
}

Instruction parse_instructions(int argc, char **argv) {
	Instruction instruction;
    instruction.error = no_error;

    if (argc == 1) {
        instruction.error = error_no_instructions;

    } else if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
		instruction.action = print_version_info;
	} else if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		instruction.action = print_detailed_help;

	} else if (strcmp(argv[1], "m") == 0) {
		instruction.action = display_metadata;
        if (argc != 3) {
            instruction.error = error_too_few_arguments;
            return instruction;
        }

		if (!file_exists(argv[2])) {
            instruction.error = error_no_such_file;
            instruction.erroneous_argument = argv[2];
            return instruction;
        }
        instruction.archive_path = argv[2];

	} else if (strcmp(argv[1], "x") == 0) {
        instruction.action = extract_content;
        int argoffset = 0;
        if (argc < 4 || argc > 5) {
            instruction.error = error_too_few_arguments;
            return instruction;
        }
        if (argc == 4) {
            argoffset = -1;
        }


        if (!file_exists(argv[3 + argoffset])) {
            instruction.error = error_no_such_file;
            instruction.erroneous_argument = argv[3 + argoffset];
            return instruction;
        }
        instruction.archive_path = argv[3 + argoffset];

        if (!directory_exists(argv[4 + argoffset])) {
            instruction.error = error_no_such_directory;
            instruction.erroneous_argument = argv[4 + argoffset];
            instruction.output_directory = argv[4 + argoffset];
            return instruction;
        }
        instruction.output_directory = argv[4 + argoffset];


        int indexes[max_indexes];
        int number_of_indexes;
        char *ranges = argc == 4 ? "0-" : argv[2];
        switch (parse_index_range(ranges, argv[3 + argoffset], indexes, &number_of_indexes)) {
            case invalid_index:
                instruction.error = error_invalid_index;
                instruction.erroneous_argument = ranges;
                break;
            case too_many_indexes:
                instruction.error = error_too_many_indexes;
                instruction.erroneous_argument = ranges;
                break;
            case not_sorted:
                instruction.error = error_invalid_index_not_sorted;
                instruction.erroneous_argument = ranges;
                break;
            case duplicates:
                instruction.error = error_invalid_index_duplicates;
                instruction.erroneous_argument = ranges;
                break;
            case ok:
                instruction.number_of_indexes = number_of_indexes;
                for (int j = 0; j < number_of_indexes; j++) {
                    instruction.indexes[j] = indexes[j];
                }
                break;
        }

	} else if (strchr(argv[1], 'l') != NULL) {
        if (strlen(argv[1]) == 1)
		    instruction.action = list_content;
        else if (strlen(argv[1]) == 2 && strchr(argv[1], 't') != NULL)
            instruction.action = list_content_detailed;
        else if (strlen(argv[1]) == 3 && strchr(argv[1], 't') != NULL && strchr(argv[1], 'z') != NULL)
            instruction.action = list_content_detailed_human_readable;
        else {
            instruction.error = error_invalid_instructions;
            return instruction;
        }

        if (argc != 3) {
            instruction.error = error_too_few_arguments;
            return instruction;
        }

        if (!file_exists(argv[2])) {
            instruction.error = error_no_such_file;
            instruction.erroneous_argument = argv[2];
            return instruction;
        }
        instruction.archive_path = argv[2];

	} else {
		instruction.error = error_invalid_instructions;
        instruction.erroneous_argument = argv[1];
	}
	return instruction;
}
