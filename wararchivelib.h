#ifndef wararchivelib
#define wararchivelib

char* identify_archive(int archive_id);

// Returns 1 if archive is a valid WAR-archive
int get_header_data(FILE *file, int *number_of_files, int *archive_id, long *hidden_data_size);

void get_index_data(char *archive_path, int index, unsigned int *uncompressed_length, unsigned int *compressed);

void extract(char *archive_path, char *directory_path, int number_of_indexes, const int indexes[]);

#endif // wararchivelib
