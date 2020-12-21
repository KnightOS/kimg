#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define KIMG_VERSION 0

struct {
	unsigned char *infile;
	FILE *outfile;
	bool bare;
	uint16_t width, height;
} context;

const char usage[] =
	    	"Usage: %s [options] input output\n"
	    	"\n"
	    	"Options:\n"
	    	"\n"
	    	"-m, --monochrome\n"
	    	"\tConverts the image to a monochrome KIMG file. This is on by default for now; color support is not yet implemented.\n"
	    	"\n"
	    	"For more information, see the man page.\n"
;

void
init_context(void)
{
	context.bare = false;
}

void
load_image(const char *const path)
{
	int h, w;
	context.infile = stbi_load(path, &w, &h, NULL, 1);
	if(context.infile == NULL){
		fprintf(stderr, "Error parsing image: %s\n", path);
		exit(1);
	}
	context.height = (uint16_t)h;
	context.width = (uint16_t)w;
}

void
load_file(const char *const path)
{
	if(context.infile == NULL){
		load_image(path);
	}else if(context.outfile == NULL){
		context.outfile = fopen(path, "wb");
		if(context.outfile == NULL){
			fprintf(stderr, "Error opening file: %s\n", path);
			exit(1);
		}
	}else{
		fprintf(stderr, "Extra file specified: %s\n", path);
		exit(1);
	}
}

#define arg(short, long) strcmp(short, argv[i]) == 0 || strcmp(long, argv[i]) == 0

void
parse_arguments(const int argc, const char *const *const argv)
{
	if(argc == 1){
		printf(usage, argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; i++){
		if(arg("-h", "--help")){
			printf(usage, argv[0]);
			exit(0);
		}else if(arg("-b", "--bare")){
			context.bare = true;
		}else if(argv[i][0] == '-'){
			fprintf(stderr, "Unrecognized argument: %s\n", argv[i]);
			exit(1);
		}else{
			load_file(argv[i]);
		}
	}
}

void
write_header(void)
{
	if(!context.bare){
		fwrite("KIMG", 4, 1, context.outfile);           // 0x00: Magic Header
		fprintf(context.outfile, "%c", KIMG_VERSION);    // 0x04: Format Version
		fprintf(context.outfile, "%c", 0);               // 0x05: Format Flags (TODO!)
		fwrite(&context.height,  2, 1, context.outfile); // 0x06: Image Height
		fwrite(&context.width,   2, 1, context.outfile); // 0x08: Image Width										 												 // 0x09: Palette/Image Data...
	}
}

uint16_t
process_data(uint8_t *buffer)
{
	uint8_t byte, mask;
	uint16_t index = 0;
	uint16_t x, y;
	for(y = 0; y < context.height; y++){
		byte = 0;
		mask = 0x80;
		
		for(x = 0; x < context.width; x++){
			if(context.infile[y * context.width + x] < 127)
				byte |= mask;
			mask >>= 1;
			if(mask == 0){
				buffer[index] = byte;
				index += 1;
				mask = 0x80;
				byte = 0;
			}
		}
		if(mask != 0x80){
			buffer[index] = byte;
			index += 1;
		}
	}
	return index;
}

void
write_data(void)
{
	uint16_t size;
	uint8_t *buffer;

	size = context.width * context.height / 8;
	if((context.width * context.height) % 8 != 0)
		size += 1;
	buffer = malloc(size);
	
	if(buffer == NULL){
		fwrite("OOM\n", 4, 1, stderr);
		exit(1);
	}

	size = process_data(buffer);
	fwrite(buffer, size, 1, context.outfile);
	free(buffer);
}

int
main(const int argc, const char *const *const argv)
{
	init_context();
	parse_arguments(argc, argv);

	write_header();
	write_data();

	stbi_image_free(context.infile);
	fflush(context.outfile);
	fclose(context.outfile);
	return 0;
}
