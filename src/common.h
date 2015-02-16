#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <wand/MagickWand.h>

#define KIMG_FORMAT_CONSTANT "KIMG"
#define KIMG_FORMAT_VERSION  0

typedef struct {
    unsigned int             : 4;
    unsigned int compression : 2;
    unsigned int palette     : 1;
    unsigned int color       : 1;

} kimg_format_t;

typedef struct {
    const char     kimg_constant[4];
    uint8_t        kimg_version;
    kimg_format_t  format;
    uint16_t       width;
    uint8_t        height;
    uint8_t        palette_size;
} kimg_header_t;

#endif // COMMON_H
