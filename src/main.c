#include "common.h"

struct {
	char *infile;
	char *outfile;
	bool color;
	bool palette;
	kimg_compression_t compression;
} context;

void print_usage(void) {
	printf("TODO: USAGE\n");
	return;
}

void init_context(void) {
	context.color = true;
	context.palette = true;
	context.compression = NONE; // TODO: implement compression
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

	MagickWand *input;
	PixelIterator *iter;
	PixelWand **row;
	MagickPixelPacket pixel;

	unsigned long x, y;
	unsigned long height;
	unsigned long width, bytewidth;

	uint8_t mask, byte;

	MagickBooleanType mwstatus;

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

	MagickWandGenesis();
	input = NewMagickWand();

	mwstatus = MagickReadImageFile(input, infile);
	if (mwstatus == MagickFalse) {
		ExceptionType type;
		fprintf(stderr, "Error reading image file: %s\n", context.infile);
		fprintf(stderr, "ImageMagick says: %s\n", MagickGetException(input, &type));
		return 1;
	}

	height = MagickGetImageHeight(input);
	width = MagickGetImageWidth(input);
	bytewidth = width / 8;
	if (width % 8 != 0) {
		bytewidth++;
	}

	// Convert to grayscale if we're monochrome
	if (!context.color) {
		MagickSetImageType(input, GrayscaleType);
		MagickSetImageColorspace(input, GRAYColorspace);
	}

	kimg_format_t format;
	format.compression = 0;
	format.palette = 0;
	format.color = 0;

	uint16_t _width = (uint16_t)width;
	uint16_t _height = (uint16_t)height;
	uint8_t _format = (uint8_t)format.value;
	fprintf(outfile, "KIMG");
	fputc(KIMG_VERSION, outfile);
	fwrite(&_format, sizeof(uint8_t), 1, outfile);
	fwrite(&_height, sizeof(uint16_t), 1, outfile);
	fwrite(&_width, sizeof(uint16_t), 1, outfile);

	iter = NewPixelIterator(input);
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

			if (mask == 0) {
				fwrite(&byte, sizeof(char), sizeof(byte), outfile);
				mask = 0x80;
				byte = 0;
			}
		}
		if (mask != 0x80) {
			fwrite(&byte, sizeof(char), sizeof(byte), outfile);
			mask = 0x80;
			byte = 0;
		}
	}

	iter  = DestroyPixelIterator(iter);
	input = DestroyMagickWand(input);
	MagickWandTerminus();
	fclose(outfile);

	return 0;
}
