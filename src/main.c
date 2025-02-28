#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "r_json.h"

char * read_as_string(const char *filename){
	FILE *file = fopen(filename, "rb");
	size_t size;
	char *buffer;

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	buffer = (char *) malloc(size + 1);
	fread(buffer, 1, size, file);
	buffer[size] = '\0';

	fclose(file);

	return buffer;
}

int main(void){
	size_t memory_size;
	void *memory_block;
	rjs_parser_t parser;
	char *buffer;

	memory_size = 1024 * 1024 * 2; /* 2 MB for loading files */
	memory_block = malloc(memory_size);

	buffer = read_as_string("manager.json");

	rjs_create_parser(&parser, memory_block, memory_size);
	if(rjs_parse_string(&parser, buffer)){
		rjs_key_t *key;
		const rjs_object_t *obj;

		obj = rjs_get_main_object(&parser);
		
		key = rjs_get_key(obj, "textures");
		obj = rjs_get_vobj(key);
		key = rjs_get_key_index(obj, 0);
		obj = rjs_get_vobj(key);
		printf("%i\n", rjs_istype(rjs_get_key(obj, "filename"), RJS_KEY_NUMBER));
		printf("%f\n", rjs_get_vnumber(rjs_get_key(obj, "cell_width")));
	}
	else{
		printf("%s\n", parser.error_log);
	}

	free(buffer);
	free(memory_block);

	return 0;
}
