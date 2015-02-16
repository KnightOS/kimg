#include "common.h"

struct {
    char             *infile;
    char             *outfile;
    bool             color;
    bool             palette;
    enum {NONE, RLE} compression;
} context;

uint8_t rotl(uint8_t x) {
    return (x << 1) | (x >> 7);
}

void print_usage(void) {
    printf("FIXME: USAGE\n");
    return;
}

void init_context(void) {
    context.color       = true;
    context.palette     = true;
    context.compression = NONE; // TODO: implement compression
}

int main(int argc, char *argv[]) {
    init_context();

    argc--;
    argv++;
    if (!argc) {
        print_usage();
        return 1;
    }

    #define arg(short, long) strcasecmp(short, argv[i]) == 0 || strcasecmp(long, argv[i]) == 0
    for (int i = 0; i < argc; i++) {
        if (arg("-c", "--castle")) {
            context.color       = false;
            context.palette     = false;
            context.compression = NONE;
        } else if (arg("-h", "--help")) {
            print_usage();
            return 0;
        } else if (arg("-m", "--monochrome")) {
            context.color = false;
        } else if (arg("-n", "--no-compression")) {
            context.compression = NONE;
        } else if (arg("-p", "--no-palette")) {
            context.palette = false;
        } else if (arg("-x", "--compression")) {
            i++;
            if (strcasecmp("NONE", argv[i]) == 0) {
                context.compression = NONE;
            } else if (strcasecmp("RLE", argv[i]) == 0) {
                context.compression = RLE;
            } else {
                fprintf(stderr, "Invalid compression type: %s\n", argv[i]);
                print_usage();
                return 1;
            }
        } else {
            if (context.infile == NULL) {
                context.infile = argv[i];
            } else if (context.outfile == NULL) {
                context.outfile = argv[i];
            } else {
                fprintf(stderr, "Unrecorgnized argument: %s\n", argv[i]);
                print_usage();
                return 1;
            }
        }
    }

    FILE *infile;
    FILE *outfile;

    MagickWand        *input;
    PixelIterator     *iter;
    PixelWand         **row;
    MagickPixelPacket pixel;
    uint8_t           *output;

    unsigned long x, y;
    unsigned long width, height;
    unsigned long wrap, padding;

    MagickBooleanType mwstatus;


    // Read input file
    infile = fopen(context.infile, "r");
    if (infile == NULL) {
        fprintf(stderr, "No such file or directory: %s\n", context.infile);
        return 1;
    }

    // Set up ImageMagick
    MagickWandGenesis();
    input = NewMagickWand();

    // Load MagickWand with image file.
    mwstatus = MagickReadImageFile(input, infile);
    if (mwstatus == MagickFalse) {
        ExceptionType type;
        fprintf(stderr, "Error reading image file: %s\n", context.infile);
        fprintf(stderr, "ImageMagick says: %s\n", MagickGetException(input, &type));
        return 1;
    }

    // Get width and height
    width  = MagickGetImageWidth(input);
    height = MagickGetImageHeight(input);
    // Calculate wrap (width of each row in bytes).
    wrap = width / 8;
    if (width % 8 != 0) wrap++;
    fprintf(stderr, "Wrap: %lu\n", wrap);
    // Calculate padding (padding on right side to make column a full byte)
    padding = (wrap * 8) - width;
    fprintf(stderr, "Padding: %lu\n", padding);

    // Allocate output array
    output = (uint8_t*)malloc((width + padding) * height);

    // Convert to grayscale if we're monochrome
    if (!context.color)
      MagickSetImageType(input, GrayscaleType);
      MagickSetImageColorspace(input, GRAYColorspace);

    iter = NewPixelIterator(input);
    for (y=0; y < height; y++) {
        row = PixelGetNextIteratorRow(iter, &width);
        for (x=0; x < width; x++) {
            PixelGetMagickColor(row[x], &pixel);

            if (context.color) {
                // TODO: color support
                /*
                printf("(%lu, %lu): #%0X%0X%0X\n", y, x,
                        (uint8_t)pixel.red,
                        (uint8_t)pixel.green,
                        (uint8_t)pixel.blue);
                */
            } else {
                if (pixel.red < 32767) { // 1
                } else { // 0
                }
                /*
                printf("(%lu, %lu): %0X\n", y, x,
                        (uint8_t)pixel.red);
                */
            }
        }
        if ((width + padding) > width) {
            for (int i = 0; i < padding; i++) {
                if (context.color) {
                // TODO: color support
                } else { // 0
                }
            }
        }
    }

    iter  = DestroyPixelIterator(iter);
    input = DestroyMagickWand(input);
    MagickWandTerminus();

    return 0;
}
