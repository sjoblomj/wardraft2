#ifndef KNOWLEGEBASEPARSER_H
#define KNOWLEGEBASEPARSER_H

int list_archive_content(char *path, int show_details, int human_readable);

int get_data_for_index(char *archive_name, int index, char extension[], char *dependencies, char description[]);

#endif //KNOWLEGEBASEPARSER_H
