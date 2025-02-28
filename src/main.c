#include <stdio.h>
#include <stdlib.h>

#include "r_json.h"

static const char *parse_string =
"{\n"
"	\"x\":2.1,\n"
"	\"y\":2.4\n"
"}\n";

int main(void){
	size_t memory_size = 1024 * 1024 * 4; /* 4MB de memória */
	void *memory_block = malloc(memory_size);
	rjs_parser_t parser;

	rjs_create_parser(&parser, memory_block, memory_size);

	if(rjs_parse_string(&parser, parse_string)){
		const rjs_key_t *key;
		const rjs_object_t *obj;

		obj = rjs_get_main_object(&parser);

		key = rjs_get_key(obj, "x");

		if(rjs_istype(key, RJS_KEY_NUMBER))
			printf("x: %f\n", rjs_get_vnumber(key));

		key = rjs_get_key(obj, "y");

		if(rjs_istype(key, RJS_KEY_NUMBER))
			printf("y: %f\n", rjs_get_vnumber(key));
	}
	else{
		printf("%s\n", rjs_get_error(&parser));
	}

	free(memory_block);

	return 0;
}
