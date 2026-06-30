#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef ROOT_DIR
#define ROOT_DIR
#endif

void read_file(const char* filename, char** output, size_t* filesize);

#endif