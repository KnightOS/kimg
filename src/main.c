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
                printf("Invalid compression type: %s\n", argv[i]);
                print_usage();
                return 1;
            }
        } else {
            if (context.infile == NULL) {
                context.infile = argv[i];
            } else if (context.outfile == NULL) {
                context.outfile = argv[i];
            } else {
                printf("Unrecorgnized argument: %s\n", argv[i]);
                print_usage();
                return 1;
            }
        }
    }

    FILE *infile;
    FILE *outfile;

    MagickWand    *mw;
    PixelIterator *imw;
    PixelWand     **pmw;

    unsigned long x, y;
    unsigned long width, height;

    Quantum qr, qg, qb;


    // Read input file
    infile = fopen(context.infile, "r");
    if (infile == NULL) {
        printf("No such file or directory: %s\n", context.infile);
        return 1;
    }

    // Set up ImageMagick
    MagickWandGenesis();
    mw = NewMagickWand();

    // Load MagickWand with image file.
    MagickReadImageFile(mw, infile);

    // Get width and height
    width  = MagickGetImageWidth(mw);
    height = MagickGetImageHeight(mw);

    // Convert to grayscale if we're monochrome
    if (!context.color)
      MagickTransformImageColorspace(mw, GRAYColorspace);

    imw = NewPixelIterator(mw);
    for (y=0; y < height; y++) {
        pmw = PixelGetNextIteratorRow(imw, &width);
        for (x=0; x < (long) width; x++) {
            if (context.color) {
                qr = PixelGetRedQuantum(pmw[x]);
                qg = PixelGetGreenQuantum(pmw[x]);
                qb = PixelGetBlueQuantum(pmw[x]);

                printf("(%lu, %lu): R%f G%f B%f\n", x, y, qr, qg, qb);
            }
        }
    }

    imw = DestroyPixelIterator(imw);
    mw  = DestroyMagickWand(mw);
    MagickWandTerminus();

    return 0;
}
