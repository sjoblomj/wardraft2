#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "knowlegebaseparser.h"
#include "wararchivelib.h"

const char knowledge_base_name[] = "war131new.knw";


void get_archive_name(char *name, char *archive_name) {
	strcpy(archive_name, name);
	int i = 0;

	archive_name[i] = '[';
	for (; name[i]; i++) {
		archive_name[i + 1] = (char) tolower(name[i]);
	}
	archive_name[++i] = ']';
	archive_name[++i] = '\n';
	archive_name[++i] = '\0';
}


void trim_commas(char* str) {
	int len = (int) strlen(str);

	// Trim commas from the start of the string
	char* start = str;
	while (*start == ',') {
		start++;
		len--;
	}

	// Trim commas from the end of the string
	char* end = str + strlen(str) - 1;
	while (len > 0 && *end == ',') {
		end--;
		len--;
	}

	// Copy the trimmed string back to the original array
	memmove(str, start, len);
	str[len] = '\0';
}

void parse_line(const char *line, int *index, char extension[], char dependencies[], char description[]) {
	sscanf(line, "%d=%3s%[0-9,]%[^\n]", index, extension, dependencies, description);
	trim_commas(dependencies);
}

int list_archive_content(char *path, int show_details, int human_readable) {
	int return_code = 0;

	FILE *archive = fopen(path, "rb");
	FILE *knwb = fopen(knowledge_base_name, "r");

	int number_of_files = 0;
	int archive_id = 0;
	long hidden_data_size = 0;

	if (get_header_data(archive, &number_of_files, &archive_id, &hidden_data_size)) {

		char *name = identify_archive(archive_id);
		char archive_name[strlen(name) + 3 + 1];
		get_archive_name(name, archive_name);

		int is_reading = 0;
		char line[256];
		while (fgets(line, sizeof(line), knwb)) {
			if (strcmp(line, "\n") == 0)
				continue;
			if (strcmp(line, archive_name) == 0) {
				is_reading = 1;
				continue;
			}
			if (is_reading && strlen(line) > 0 && line[0] == '[')
				break;

			if (is_reading) {
				unsigned int cmp = 0;
				unsigned int uncompressed_length = 0;

				int index;
				char ext[4];
				char deps[32];
				char desc[250];
				parse_line(line, &index, ext, deps, desc);

				printf("%3d ", index);
				if (show_details) {
					get_index_data(path, index, &uncompressed_length, &cmp);

					printf("%1s ", cmp ? "C" : "");

					if (human_readable) {
						const char* suffixes[] = {"B", "K", "M", "G", "T"};
						int suffix = 0;
						char size_chars[7];
						sprintf(size_chars, "%u", uncompressed_length);

						if (uncompressed_length > 1024) {
							double size = uncompressed_length;
							while (size >= 1024 && suffix < 4) {
								suffix++;
								size /= 1024;
							}
							sprintf(size_chars, "%.1f%s", size, suffixes[suffix]);
						}
						printf("%7s ", size_chars);
					} else {
						printf("%6i ", uncompressed_length);
					}
				}
				printf("%s  %s", ext, desc);
				if (strcmp(deps, "") != 0)
					printf(" [%s]", deps);
				printf("\n");
			}
		}
	}
	if (knwb != NULL) {
		fclose(knwb);
	} else {
		return_code = 1;
	}
	if (archive != NULL) {
		fclose(archive);
	} else {
		return_code = 1;
	}

	return return_code;
}

int get_data_for_index(char *name, int index, char extension[], char dependencies[], char description[]) {
	FILE *knwb = fopen(knowledge_base_name, "r");
	char archive_name[strlen(name) + 3 + 1];
	get_archive_name(name, archive_name);

	int is_reading = 0, found_index = 0;
	char line[256];
	while (fgets(line, sizeof(line), knwb)) {
		if (strcmp(line, "\n") == 0)
			continue;
		if (strcmp(line, archive_name) == 0) {
			is_reading = 1;
			continue;
		}
		if (is_reading && strlen(line) > 0 && line[0] == '[')
			break;

		if (is_reading) {
			int i;
			char ext[4];
			char deps[32];
			char desc[250];
			parse_line(line, &i, ext, deps, desc);

			if (i == index) {
				strcpy(extension, ext);
				strcpy(dependencies, deps);
				strcpy(description, desc);
				found_index = 1;
				break;
			}
		}
	}

	if (knwb != NULL) {
		fclose(knwb);
	} else {
		printf("Unable to close Knowledge Base!\n");
	}
	return found_index;
}
