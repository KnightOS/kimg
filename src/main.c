#include "common.h"

struct {
    char               *infile;
    char               *outfile;
    bool               color;
    bool               palette;
    kimg_compression_t compression;
} context;

void print_usage(void) {
    printf("TODO: USAGE\n");
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

    unsigned long x, y;
    unsigned long height;
    unsigned long width, bytewidth;

    uint8_t mask, byte;

    MagickBooleanType mwstatus;


    // Read input file
    infile = fopen(context.infile, "r");
    if (infile == NULL) {
        fprintf(stderr, "No such file or directory: %s\n", context.infile);
        return 1;
    }
    // Open output file
    outfile = fopen(context.outfile, "wb");
    if (outfile == NULL) {
        fprintf(stderr, "Error opening file: %s\n", context.outfile);
        return 1;
    }

    // Set up MagickWand
    MagickWandGenesis();
    input = NewMagickWand();

    // Load input image into MagickWand
    mwstatus = MagickReadImageFile(input, infile);
    if (mwstatus == MagickFalse) {
        ExceptionType type;
        fprintf(stderr, "Error reading image file: %s\n", context.infile);
        fprintf(stderr, "ImageMagick says: %s\n", MagickGetException(input, &type));
        return 1;
    }

    // Get height
    height    = MagickGetImageHeight(input);
    // Get width and calculate bytewidth (width of row in bytes)
    width     = MagickGetImageWidth(input);
    bytewidth = width / 8;
    if (width % 8 != 0) bytewidth++;

    // Convert to grayscale if we're monochrome
    if (!context.color)
      MagickSetImageType(input, GrayscaleType);
      MagickSetImageColorspace(input, GRAYColorspace);

    kimg_header_t header;
    kimg_format_t format;
    // Build kimg format byte
    format.compression = 0;
    format.palette     = 0;
    format.color       = 0;

    // Build kimg header
    strcpy(header.magic, KIMG_MAGIC);
    header.version = KIMG_VERSION;
    header.format  = format;
    header.height  = height;
    header.width   = width;

    fwrite(&header, sizeof(header), 1, outfile);

    iter = NewPixelIterator(input);
    for (y=0; y < height; y++) {
        // Reset mask and byte
        mask = 0x80; // 0b10000000
        byte = 0;

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
                // Set bit if pixel is positive
                if (pixel.red < 32767) {
                    byte |= mask;
                }
                /*
                printf("(%lu, %lu): %0X\n", y, x,
                        (uint8_t)pixel.red);
                */
            }

            // Reset mask and byte and output byte
            if (x % 8 == 0 && x != 0) {
                mask = 0x80; // 0b10000000
                byte = 0;

                fwrite(&byte, sizeof(char), sizeof(byte), outfile);
            // Also output if at the end of the row (zero-pading the right)
            } else if (x == (width - 1)) {
                fwrite(&byte, sizeof(char), sizeof(byte), outfile);
            }

            // Shift mask right 1 position to mask the next bit
            mask = mask >> 1;
        }
    }

    // Clean up MagickWand.
    iter  = DestroyPixelIterator(iter);
    input = DestroyMagickWand(input);
    MagickWandTerminus();
    // Flush output file.
    fclose(outfile);

    return 0;
}
