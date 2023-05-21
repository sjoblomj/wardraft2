#include <stdio.h>
#include "knowlegebaseparser.h"
#include "parse_input.h"
#include "wararchivelib.h"

char version[] = "0.1";
char program_name[] = "WarDraft II";

void print_version() {
	printf("%s\nVersion: %s\nBy Ojan (Johan Sjöblom)\n", program_name, version);
}

void print_help_detailed() {
	printf("%s - A WarCraft I and II Archive utility\n\nTODO: Write help\n", program_name);
}

void print_help_condense() {
	printf("For all valid instructions, provide --help flag\n\nExamples:\n");
	printf("- List the content of an archive\n");
	printf("  wd2 l path/to/archive.[war|cud|sud]\n\n");

	printf("- Extract the given indexes to directory\n");
	printf("  wd2 x 1,3,6-8,10- path/to/archive.[war|cud|sud] path/to/output/directory\n\n");

	printf("- Replace the files in the given indexes with those provided\n");
	printf("  wd2 r 0,3-5 path/to/archive.[war|cud|sud] file0 file3 file4 file5\n\n");

	printf("- Inserts the file before the given index\n");
	printf("  wd2 i 3 path/to/archive.[war|cud|sud] file0\n\n");

	printf("- Deletes the files in the given indexes\n");
	printf("  wd2 d 3,5-7 path/to/archive.[war|cud|sud]\n\n");

	printf("- Creates an archive with the given files\n");
	printf("  wd2 c path/to/target.[war|cud|sud] path/to/file1 path/to/file2 ...\n\n");

	printf("- Display metadata about the archive\n");
	printf("  wd2 m path/to/archive\n");
}

int display_archive_metadata(char *path) {
	int return_code = 0;
	int number_of_files = 0;
	int archive_id = 0;
	long hidden_data_size = 0;
	FILE *file = fopen(path, "rb");

	int valid_archive = get_header_data(file, &number_of_files, &archive_id, &hidden_data_size);

	if (valid_archive == 0) {
		printf("'%s' is an invalid War-archive!\n", path);
		return_code = 1;
	} else {
		printf("'%s' is a valid War-archive.\n", path);
		printf("Number of files: %i\nHeader signature: %i - %s\nSize of hidden header-data: %li\n",
			   number_of_files, archive_id, identify_archive(archive_id), hidden_data_size);
	}

	if (file != NULL) {
		fclose(file);
	}
	return return_code;
}

int main(int argc, char **argv) {
	Instruction i = parse_instructions(argc, argv);
	int return_code;

	switch (i.error) {
		case error_no_instructions:
			printf("Error: No instructions given.\n\n");
			print_help_condense();
			return_code = 1;
			break;
		case error_too_few_arguments:
			printf("Error: Too few arguments given.\n\n");
			print_help_condense();
			return_code = 1;
			break;
		case error_invalid_instructions:
			printf("Error: Invalid instructions: '%s'.\n\n", i.erroneous_argument);
			print_help_condense();
			return_code = 1;
			break;
		case error_no_such_file:
			printf("Error: Given file does not exist: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case error_no_such_directory:
			printf("Error: Given directory does not exist: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case error_invalid_index:
			printf("Error: Invalid index: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case error_invalid_index_not_sorted:
			printf("Error: Indexes not sorted: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case error_invalid_index_duplicates:
			printf("Error: Indexes contain duplicates: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case error_too_many_indexes:
			printf("Error: Too many indexes: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case error_file_too_big:
			printf("Error: Given file is too big: '%s'\n", i.erroneous_argument);
			return_code = 1;
			break;
		case no_error:
			return_code = 0;
			break;
	}
	if (return_code > 0)
		return return_code;

	switch (i.action) {
		case print_version_info:
			print_version();
			return 0;
		case print_detailed_help:
			print_help_detailed();
			return 0;
		case print_condense_help:
			print_help_condense();
			return 0;

		case list_content:
			return list_archive_content(argv[2], 0, 0);
		case list_content_detailed:
			return list_archive_content(argv[2], 1, 0);
		case list_content_detailed_human_readable:
			return list_archive_content(argv[2], 1, 1);

		case display_metadata:
			return display_archive_metadata(argv[2]);

		case extract_content:
/*
			printf("Extract content\n");
			printf("Dir: '%s'\n", i.output_directory);

			printf("Expanded indexes: ");
			for (int j = 0; j < i.number_of_indexes; j++) {
				printf("%d ", i.indexes[j]);
			}
			printf("\n");
*/
			extract(i.archive_path, i.output_directory, i.number_of_indexes, i.indexes);

			return 0;

		case insert_content:
			compress(0xfff);
			return 0;

		case replace_content:
			compress(0xfff);
			return 0;

		default:
			printf("Error\n");
			return 1;
	}
}
