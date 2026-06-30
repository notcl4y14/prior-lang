#include "utils.h"

void read_file(const char* filename, char** output, size_t* filesize) {
    /* Open file */
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Failed to load file \"%s\"\n", filename);
        return;
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    *filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Allocating output */
    *output = malloc(*filesize + 1);

    /* Reading into output */
    fread(*output, sizeof(uint8_t), *filesize, file);

    /* Adding a null terminator */
    // *output[*filesize] = '\0';

    /* Close file */
    fclose(file);
}