#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "knowlegebaseparser.h"
#include "wararchivelib.h"


unsigned char* decompress(unsigned char *archiveptr, unsigned int *uncompressed_length, int decompress) {
    return "todo";
}

const unsigned int bytes_in_a_flag = 8;


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

    int len, cmp;
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

int get_location_in_buffer(
        const unsigned char *elems,
        int elems_len,
        const unsigned char *search_buffer,
        int search_buffer_index,
        int search_buffer_len
) {

    int i, offset = 0;
    for (i = 0; i < search_buffer_len; i++) {
        if (elems_len <= offset)
            break;
        if (elems[offset] == search_buffer[(i + search_buffer_index) % search_buffer_len])
            offset++;
        else
            offset = 0;
    }
    if (elems_len <= offset) {
        // All the content of elems are in search_buffer
        return i - elems_len;
    }
    return -1;
}

void extend_sliding_window(unsigned char *sliding_window, unsigned char content, int *index, int *size, int max_size) {
    sliding_window[(*index)++ % max_size] = content;
    if (*size < max_size)
        (*size)++;
}

unsigned char reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

int write_flag(
        unsigned short *flags_added,
        unsigned char *flag,
        unsigned char *prev_flag_pos
) {
    if (*flags_added == bytes_in_a_flag) {
        *prev_flag_pos = reverse(*flag);
        *flags_added = 0;
        *flag = 0;

        return 1;
    }
    return 0;
}

unsigned char* compress_content(
        const unsigned char *elems,
        const long elems_len,
        const int sliding_window_size,
        unsigned int *retlen
) {

    unsigned char sliding_window[sliding_window_size + 1];
    unsigned char potential_match[sliding_window_size + 1];
    memset(sliding_window, 0, sliding_window_size);

    unsigned char flag = 0;
    unsigned short flags_added = 0;

    unsigned char *start_of_data;
    unsigned char *data;
    unsigned char *prev_flag_pos;
    start_of_data = data = prev_flag_pos = (unsigned char*) malloc(elems_len + (elems_len / bytes_in_a_flag));
    int sw_index = 0, sw_size = sliding_window_size, pm_index = 0, pm_size = 0;

    *data++ = 0xff;
    for (int i = 0; i < elems_len; i++) {
        potential_match[pm_size] = elems[i];

        int next_index = get_location_in_buffer(potential_match, pm_size + 1, sliding_window, sw_index, sw_size);
        if (next_index == -1 || i == elems_len - 1) {
            if (next_index != -1 && i == elems_len - 1) {
                // Only if it's the last character then add the next character to the text the token is representing
                potential_match[pm_size++] = elems[i];
                pm_size = 0;
            }
            if (pm_size > 2) { // We need 2 bytes to represent an encoding; the data compressed needs to be greater for this to be worthwhile.
                int index = get_location_in_buffer(potential_match, pm_size, sliding_window, sw_index, sw_size);

                flag <<= 1;
                flag |= 0;
                flags_added++;

                int token = ((pm_size - 3) << 12) + index;
                *data++ = token;
                *data++ = token >> 8;

                for (int k = 0; k < pm_size; k++) {
                    extend_sliding_window(sliding_window, potential_match[k], &sw_index, &sw_size, sliding_window_size);
                }

                if (write_flag(&flags_added, &flag, prev_flag_pos)) {
                    prev_flag_pos = data;
                    *data++ = 0xff;
                }
                pm_size = 0;

            } else {
                flag <<= 1;
                flag |= 1;
                flags_added++;

                extend_sliding_window(sliding_window, potential_match[0], &sw_index, &sw_size, sliding_window_size);
                *data++ = potential_match[0];

                if (write_flag(&flags_added, &flag, prev_flag_pos)) {
                    prev_flag_pos = data;
                    *data++ = 0xff;
                }

                pm_size--;
                for (int k = 0; k < pm_size; k++) {
                    potential_match[k] = potential_match[k + 1];
                }
            }
        }
        potential_match[pm_size++] = elems[i];
    }

    if (flags_added > 0) {
        *prev_flag_pos = flag;
    }

    *retlen = data - start_of_data;
    return start_of_data;
}

void compress(int sliding_window_length) {
    FILE *file = fopen("extracted/51.STR", "rb");
    fseek(file, 0, SEEK_END);
    long offset_start = 0;
    long offset_end = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char content[offset_end - offset_start];
    fread(&content, sizeof(content), 1, file);

    unsigned int complen = 0;
    unsigned char *compressed = compress_content(content, offset_end, sliding_window_length, &complen);

    FILE *write_ptr = fopen("extracted/51.STRcmpMINE","wb");

    fwrite(compressed, complen, 1, write_ptr);
    fclose(write_ptr);
    free(compressed);
}
