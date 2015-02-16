#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wand/MagickWand.h>

#define STR(arg) #arg

#define KIMG_MAGIC   STR(KIMG)
#define KIMG_VERSION 0u

typedef enum {
    NONE,    // 0b00
    RLE,     // 0b01
    FUTURE1, // 0b10
    FUTURE2, // 0b11
} kimg_compression_t;

typedef struct {
    unsigned int             : 4;
    unsigned int compression : 2;
    unsigned int palette     : 1;
    unsigned int color       : 1;

} kimg_format_t;

typedef struct {
    char           magic[4];
    uint8_t        version;
    kimg_format_t  format;
    uint16_t       height;
    uint16_t       width;
    uint8_t        palette_size;
} kimg_header_t;

#endif // COMMON_H
