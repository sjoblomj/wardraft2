#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "knowlegebaseparser.h"
#include "wararchivelib.h"


unsigned char* decompress(unsigned char *archiveptr, unsigned int *uncompressed_length, int decompress) {
    return "todo";
}

char* identify_archive(int archive_id) {
    if      (archive_id == 0)    return    "DATA.WAR\0"; // WarCraft I
    else if (archive_id == 1000) return "MAINDAT.WAR\0"; // WarCraft II
    else if (archive_id == 2000) return  "SNDDAT.WAR\0"; // WarCraft II
    else if (archive_id == 3000) return  "REZDAT.WAR\0"; // WarCraft II
    else if (archive_id == 4000) return  "STRDAT.WAR\0"; // WarCraft II
    else if (archive_id == 5000) return  "SFXDAT.SUD\0"; // WarCraft II
    else if (archive_id == 6000) return  "MUDDAT.CUD\0"; // WarCraft II
    return "";
}

int get_header_data(FILE *file, int *number_of_files, int *archive_id, long *hidden_data_size) {
    int header_signature, first_offset;

    fread(&header_signature, 4, 1, file);
    fread(number_of_files, 2, 1, file);
    fread(archive_id, 2, 1, file);

    long int hidden_data_start = ftell(file) + (*number_of_files * 4);
    fread(&first_offset, 4, 1, file);
    *hidden_data_size = first_offset - hidden_data_start;

    return header_signature == 0x00000019; // Magic signature for WAR-archives
}

void get_index_data(char *archive_path, int index, unsigned int *uncompressed_length, unsigned int *compressed) {
    FILE *file = fopen(archive_path, "rb");

    unsigned int offset_start, offset_end;
    fseek(file, 8 + (index * 4), SEEK_SET);
    fread(&offset_start, 4, 1, file);
    fread(&offset_end, 4, 1, file);
    fseek(file, offset_start, SEEK_SET);

    size_t len, cmp;
    fread(&len, 4, 1, file);

    cmp = len >> 24;
    len &= 0x00FFFFFF;
    *compressed = cmp;
    *uncompressed_length = len;

    fclose(file);
}

void make_output_path(const char *directory_path, const int index, char *archive_name, char *outpath) {
    char extension[4];
    char deps[32];
    char desc[250];
    int success = get_data_for_index(archive_name, index, extension, deps, desc);
    if (success != 1)
        strcpy(extension, "UNK");
    sprintf(outpath, "%s/%i.%s", directory_path, index, extension);
}

void extract(char *archive_path, char *directory_path, int number_of_indexes, const int indexes[]) {
    int number_of_files, archive_id = 0;
    long hidden_data_size = 0;
    FILE *file = fopen(archive_path, "rb");
    get_header_data(file, &number_of_files, &archive_id, &hidden_data_size);
    char *archive_name = identify_archive(archive_id);

    for (int i = 0; i < number_of_indexes; i++) {
        unsigned int offset_start, offset_end;
        fseek(file, 8 + (indexes[i] * 4), SEEK_SET);
        fread(&offset_start, 4, 1, file);
        if (indexes[i] == number_of_files - 1) {
            fseek(file, 0, SEEK_END);
            offset_end = ftell(file);
        } else {
            fread(&offset_end, 4, 1, file);
        }
        fseek(file, offset_start, SEEK_SET);

        unsigned char content[offset_end - offset_start];
        fread(&content, sizeof(content), 1, file);

        unsigned int origsize;
        unsigned char *entry = decompress(content, &origsize, 1);

        char outpath[256];
        make_output_path(directory_path, indexes[i], archive_name, outpath);
        FILE *write_ptr = fopen(outpath,"wb");

        fwrite(entry, origsize, 1, write_ptr);
        fclose(write_ptr);
        free(entry);
    }
    fclose(file);
}
