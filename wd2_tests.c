#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parse_input.h"
#include "wararchivelib.h"

int assert_action(Action expected, Action actual) {
	assert(expected == actual);
	return expected != actual;
}

int assert_error(Error expected, Error actual) {
	assert(expected == actual);
	return expected != actual;
}

int assert_error_argument(const char* expected, const char* actual) {
	assert(strcmp(expected, actual) == 0);
	return strcmp(expected, actual) != 0;
}

int assert_indexes(const int* expected, const int* actual, const int num) {
	for (int i = 0; i < num; i++) {
		assert(expected[i] == actual[i]);
	}
	return 0;
}

int test_requires_arguments() {
	char *argv[] = {"wd2"};
	Instruction i = parse_instructions(1, argv);
	return assert_error(error_no_instructions, i.error);
}

int test_can_parse_legal_flags() {
	int result = 0;

	char *argv0[] = {"wd2", "-v"};
	Instruction i0 = parse_instructions(2, argv0);
	result |= assert_action(print_version_info, i0.action);
	result |= assert_error(no_error, i0.error);

	char *argv1[] = {"wd2", "--version"};
	Instruction i1 = parse_instructions(2, argv1);
	result |= assert_action(print_version_info, i1.action);
	result |= assert_error(no_error, i1.error);

	char *argv2[] = {"wd2", "-h"};
	Instruction i2 = parse_instructions(2, argv2);
	result |= assert_action(print_detailed_help, i2.action);
	result |= assert_error(no_error, i2.error);

	char *argv3[] = {"wd2", "--help"};
	Instruction i3 = parse_instructions(2, argv3);
	result |= assert_action(print_detailed_help, i3.action);
	result |= assert_error(no_error, i3.error);

	return result;
}

int test_can_parse_displaying_of_metadata() {
	int result = 0;

	char *argv0[] = {"wd2", "m"};
	Instruction i0 = parse_instructions(2, argv0);
	result |= assert_action(display_metadata, i0.action);
	result |= assert_error(error_too_few_arguments, i0.error);

	char *argv1[] = {"wd2", "m", "wd2.c"}; // TODO: Filename
	Instruction i1 = parse_instructions(3, argv1);
	result |= assert_action(display_metadata, i1.action);
	result |= assert_error(no_error, i1.error);

	return result;
}

int test_can_parse_listing() {
	int result = 0;

	char *argv0[] = {"wd2", "l"};
	Instruction i0 = parse_instructions(2, argv0);
	result |= assert_action(list_content, i0.action);
	result |= assert_error(error_too_few_arguments, i0.error);

	char *argv1[] = {"wd2", "lq"};
	Instruction i1 = parse_instructions(2, argv1);
	result |= assert_error(error_invalid_instructions, i1.error);

	char *argv2[] = {"wd2", "l", "old/MAINDAT.WAR"}; // TODO: Filename
	Instruction i2 = parse_instructions(3, argv2);
	result |= assert_action(list_content, i2.action);
	result |= assert_error(no_error, i2.error);

	return result;
}

int test_can_parse_detailed_listing() {
	int result = 0;

	char *argv0[] = {"wd2", "lt"};
	Instruction i0 = parse_instructions(2, argv0);
	result |= assert_action(list_content_detailed, i0.action);
	result |= assert_error(error_too_few_arguments, i0.error);

	char *argv1[] = {"wd2", "lqt"};
	Instruction i1 = parse_instructions(2, argv1);
	result |= assert_error(error_invalid_instructions, i1.error);

	char *argv2[] = {"wd2", "lt", "old/MAINDAT.WAR"}; // TODO: Filename
	Instruction i2 = parse_instructions(3, argv2);
	result |= assert_action(list_content_detailed, i2.action);
	result |= assert_error(no_error, i2.error);

	return result;
}

int test_can_parse_detailed_listing_human_readable() {
	int result = 0;

	char *argv0[] = {"wd2", "ltz"};
	Instruction i0 = parse_instructions(2, argv0);
	result |= assert_action(list_content_detailed_human_readable, i0.action);
	result |= assert_error(error_too_few_arguments, i0.error);

	char *argv1[] = {"wd2", "lztq"};
	Instruction i1 = parse_instructions(2, argv1);
	result |= assert_error(error_invalid_instructions, i1.error);

	char *argv2[] = {"wd2", "ltz", "old/MAINDAT.WAR"}; // TODO: Filename
	Instruction i2 = parse_instructions(3, argv2);
	result |= assert_action(list_content_detailed_human_readable, i2.action);
	result |= assert_error(no_error, i2.error);

	return result;
}

int test_cannot_parse_illegal_flags() {
	char *argv[] = {"wd2", "c"};
	Instruction i = parse_instructions(2, argv);
	return assert_error(error_invalid_instructions, i.error);
}

int test_can_read_header_info() {
	int number_of_files = 0;
	int archive_id = 0;
	long hidden_data_size = 0;

	FILE *file = fopen("old/MAINDAT.WAR", "rb"); // TODO: Path

	int valid_archive = get_header_data(file, &number_of_files, &archive_id, &hidden_data_size);

	assert(valid_archive == 1);
	assert(number_of_files == 528);
	assert(hidden_data_size == 0);
	assert(archive_id == 1000);
	assert(strcmp(identify_archive(archive_id), "MAINDAT.WAR\0") == 0);

	if (file != NULL) {
		fclose(file);
	}
	return 0;
}

int test_extraction_with_invalid_index() {
	int result = 0;

	char *argv0[] = {"wd2", "x", "a", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i0 = parse_instructions(5, argv0);
	result |= assert_action(extract_content, i0.action);
	result |= assert_error(error_invalid_index, i0.error);
	result |= assert_error_argument("a", i0.erroneous_argument);

	char *argv1[] = {"wd2", "x", "2-c", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i1 = parse_instructions(5, argv1);
	result |= assert_action(extract_content, i1.action);
	result |= assert_error(error_invalid_index, i1.error);
	result |= assert_error_argument("2-c", i1.erroneous_argument);

	char *argv2[] = {"wd2", "x", "-2", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i2 = parse_instructions(5, argv2);
	result |= assert_action(extract_content, i2.action);
	result |= assert_error(error_invalid_index, i2.error);
	result |= assert_error_argument("-2", i2.erroneous_argument);

	char *argv3[] = {"wd2", "x", "3-2", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i3 = parse_instructions(5, argv3);
	result |= assert_action(extract_content, i3.action);
	result |= assert_error(error_invalid_index, i3.error);
	result |= assert_error_argument("3-2", i3.erroneous_argument);

	char *argv4[] = {"wd2", "x", "3,2", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i4 = parse_instructions(5, argv4);
	result |= assert_action(extract_content, i4.action);
	result |= assert_error(error_invalid_index_not_sorted, i4.error);
	result |= assert_error_argument("3,2", i4.erroneous_argument);

	char *argv5[] = {"wd2", "x", "2,3,3", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i5 = parse_instructions(5, argv5);
	result |= assert_action(extract_content, i5.action);
	result |= assert_error(error_invalid_index_duplicates, i5.error);
	result |= assert_error_argument("2,3,3", i5.erroneous_argument);

	char *argv6[] = {"wd2", "x", "0-5000", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i6 = parse_instructions(5, argv6);
	result |= assert_action(extract_content, i6.action);
	result |= assert_error(error_too_many_indexes, i6.error);

	char *argv7[] = {"wd2", "x", "0-130", "old/MAINDAT.WAR", "non-existent-dir"}; // TODO: Path
	Instruction i7 = parse_instructions(5, argv7);
	result |= assert_action(extract_content, i7.action);
	result |= assert_error(error_no_such_directory, i7.error);

	return result;
}

int test_can_parse_extraction() {
	int result = 0;
	char *argv[] = {"wd2", "x", "2", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i = parse_instructions(5, argv);
	result |= assert_action(extract_content, i.action);
	result |= assert_error(no_error, i.error);
	int expected_indexes[] = { 2 };
	result |= assert_indexes(expected_indexes, i.indexes, 1);
	return result;
}

int test_can_parse_extraction_with_ranges() {
	int result = 0;
	char *argv[] = {"wd2", "x", "2,6,9-11", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i = parse_instructions(5, argv);
	result |= assert_action(extract_content, i.action);
	result |= assert_error(no_error, i.error);
	int expected_indexes[] = { 2, 6, 9, 10, 11 };
	result |= assert_indexes(expected_indexes, i.indexes, 1);
	return result;
}

int test_can_parse_extraction_with_open_range() {
	int result = 0;
	char *argv[] = {"wd2", "x", "2,6,525-", "old/MAINDAT.WAR", "."}; // TODO: Path
	Instruction i = parse_instructions(5, argv);
	result |= assert_action(extract_content, i.action);
	result |= assert_error(no_error, i.error);
	int expected_indexes[] = { 2, 6, 525, 526, 527 };
	result |= assert_indexes(expected_indexes, i.indexes, 1);
	return result;
}


int main() {
	int success = 0;

	success |= test_requires_arguments();
	success |= test_can_parse_legal_flags();
	success |= test_can_parse_displaying_of_metadata();
	success |= test_can_parse_listing();
	success |= test_can_parse_detailed_listing();
	success |= test_can_parse_detailed_listing_human_readable();
	success |= test_cannot_parse_illegal_flags();
	success |= test_can_read_header_info();
	success |= test_extraction_with_invalid_index();
	success |= test_can_parse_extraction();
	success |= test_can_parse_extraction_with_ranges();
	success |= test_can_parse_extraction_with_open_range();

	if (success == 0)
		printf("Tests passed\n");
	else
		printf("Tests failed\n");
	return success;
}
