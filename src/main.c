#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wand/MagickWand.h>

#define KIMG_VERSION 0

struct {
	char *infile;
	char *outfile;
	bool color;
	bool palette;
	bool bare;
	enum {
		NONE,    // 0b00
		RLE,     // 0b01
		FUTURE1, // 0b10
		FUTURE2, // 0b11
	} compression;
} context;

void print_usage(void) {
	printf("TODO: USAGE\n");
	return;
}

void init_context(void) {
	context.color = true;
	context.palette = true;
	context.compression = NONE; // TODO: compression
	context.bare = false;
}

int parse_arguments(int argc, char **argv) {
	argc--;
	argv++;
	if (!argc) {
		print_usage();
		return 1;
	}

	#define arg(short, long) strcasecmp(short, argv[i]) == 0 || strcasecmp(long, argv[i]) == 0
	for (int i = 0; i < argc; i++) {
		if (arg("-c", "--castle")) {
			context.color = false;
			context.palette = false;
			context.compression = NONE;
		} else if (arg("-h", "--help")) {
			print_usage();
			return 0;
		} else if (arg("-b", "--bare")) {
			context.bare = true;
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
				fprintf(stderr, "Unrecognized argument: %s\n", argv[i]);
				print_usage();
				return 1;
			}
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	init_context();

	int _;
	if ((_ = parse_arguments(argc, argv))) {
		return _;
	}

	FILE *infile;
	FILE *outfile;

	infile = fopen(context.infile, "r");
	if (infile == NULL) {
		fprintf(stderr, "No such file or directory: %s\n", context.infile);
		return 1;
	}

	outfile = fopen(context.outfile, "wb");
	if (outfile == NULL) {
		fprintf(stderr, "Error opening file: %s\n", context.outfile);
		return 1;
	}

	MagickWand *input;

	MagickWandGenesis();
	input = NewMagickWand();

	MagickBooleanType mwstatus = MagickReadImageFile(input, infile);
	if (mwstatus == MagickFalse) {
		ExceptionType type;
		fprintf(stderr, "Error reading image file: %s\n", context.infile);
		fprintf(stderr, "ImageMagick says: %s\n", MagickGetException(input, &type));
		return 1;
	}

	unsigned long height, width;
	unsigned short bytewidth;

	height = MagickGetImageHeight(input);
	width = MagickGetImageWidth(input);
	bytewidth = width / 8;
	if (width % 8 != 0) {
		bytewidth++;
	}

	if (!context.color) {
		MagickSetImageType(input, GrayscaleType);
		MagickSetImageColorspace(input, GRAYColorspace);
	}

	uint8_t _version = KIMG_VERSION;
	uint8_t _format = 0; // TODO: format flags
	uint16_t _height = (uint16_t)height;
	uint16_t _width = (uint16_t)width;

	if (!context.bare) {
		fwrite("KIMG",    4, 1, outfile); // 0x00: Magic Header
		fwrite(&_version, 1, 1, outfile); // 0x04: Format Version
		fwrite(&_format,  1, 1, outfile); // 0x05: Format Flags
		fwrite(&_height,  2, 1, outfile); // 0x06: Image Height
		fwrite(&_width,   2, 1, outfile); // 0x08: Image Width
										  // 0x09: Palette/Image Data...
	}

	PixelIterator *iter;
	PixelWand **row;
	MagickPixelPacket pixel;
	uint8_t mask, byte;

	iter = NewPixelIterator(input);

	unsigned long x, y;
	for (y = 0; y < height; y++) {
		mask = 0x80;
		byte = 0;

		row = PixelGetNextIteratorRow(iter, &width);
		for (x = 0; x < width; x++) {
			PixelGetMagickColor(row[x], &pixel);

			if (context.color) {
				// TODO: color support
			} else {
				if (pixel.red < 32767) {
					byte |= mask;
				}
			}

			mask >>= 1;

			if (mask == 0) { // TODO: build in memory
				fwrite(&byte, sizeof(uint8_t), 1, outfile);
				mask = 0x80;
				byte = 0;
			}
		}
		if (mask != 0x80) {
			fwrite(&byte, sizeof(uint8_t), 1, outfile);
		}
	}

	iter = DestroyPixelIterator(iter);
	input = DestroyMagickWand(input);
	MagickWandTerminus();
	fclose(outfile);

	return 0;
}
