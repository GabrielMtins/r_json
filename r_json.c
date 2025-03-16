#include "r_json.h"

/*
 *	  This file is part of r_json.
 *	  r_json is free software: you can redistribute it and/or modify it
 *	  under the terms of the GNU General Public License as published by the
 *	  Free Software Foundation, either version 3 of the License, or (at
 *	  your option) any later version.
 *
 *	  r_json is distributed in the hope that it will be useful,
 *	  but WITHOUT ANY WARRANTY; without even the implied warranty
 *	  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *	  See the GNU General Public License for more details.
 *
 *	  You should have received a copy of the GNU General Public License
 *	  along with r_json. If not, see <https://www.gnu.org/licenses/>.
 *	  Copyright	Gabriel Martins (C) 2025
*/

/* A biblioteca não depende da standard library, por isso a definição de NULL */

#ifndef NULL
	#ifdef __cplusplus
		#define NULL 0
	#else
		#define NULL ( (void *) 0 )
	#endif
#endif

/* Estados possíveis para o parser. */
enum rjs_states_e {
	RJS_SEARCH_OPEN_BRACKET = 0,
	RJS_SEARCH_END,
	RJS_SEARCH_TOKEN_STRING,
	RJS_SEARCH_COLON,
	RJS_SEARCH_VALUE,
	RJS_READ_VALUE_STRING,
	RJS_READ_VALUE_STRING_CONTROL,
	RJS_READ_VALUE_NUMBER,
	RJS_READ_TOKEN_STRING
};

enum rjs_strtod_states_e {
	RJS_STRTOD_READ_NUMBER,
	RJS_STRTOD_READ_FRACTION,
	RJS_STRTOD_READ_EXPONENT
};

/* Armazena uma mensagem de erro, com a indicação de qual linha o erro
 * aconteceu. */
static void rjs_log(rjs_parser_t *parser, const char *message);

/* Adiciona um caractere na string, verificando se ainda há espaço disponível.
 * Retorna 0 caso não haja. */
static int rjs_add_character(char *str, rjs_size_t pos, char ch);

/* Retorna 1 se o caractere é um whitespace, 0 caso contrário. */
static int rjs_isspace(char c);

/* Retorna 1 se pode ser um número inicial (-, 0 .. 9), 0 caso contrário. */
static int rjs_isstartnumber(char c);

static double rjs_pow(double number, int exponent);

/* Converte uma string para um número do tipo double. O número deve ser passado
 * por referência.
 * Caso ocorra com sucesso, retorna 1, 0 caso contrário. */
static int rjs_strtod(const char *str, double *ret_number);

/* Faz uma comparação entre n caracteres.
 * Retorna 0 caso sejam iguais. */
static int rjs_strncmp(const char *str1, const char *str2, rjs_size_t n);

/* Faz uma comparação entre as strings.
 * Retorna 0 caso sejam iguais. */
static int rjs_strcmp(const char *str1, const char *str2);

/* Aloca a quantidade desejada de memória no parser. Caso não haja mais memória,
 * marca a flag de falta de memória e retorna NULL. */
static void *rjs_alloc(rjs_parser_t *parser, rjs_size_t size);

/* Aloca uma string na memória e retorna um ponteiro para ela. Caso não haja
 * mais memória, retorna NULL. */
static const char *rjs_pushstring(rjs_parser_t *parser, const char *str);

/* Aloca um objeto na memória, retornando NULL caso não haja mais memória. */
static rjs_object_t *rjs_create_obj(rjs_parser_t *parser);

/* Põe um objeto no topo da pilha, indicando que esse é o objeto a ser lido.
 * Caso não haja mais espaço na pilha, retorna 0, 1 caso contrário. */
static int rjs_stack_pushobject(rjs_parser_t *parser, rjs_object_t *obj);

/* Retira um objeto do topo da pilha. Caso não haja objetos, retorna 0,
 * 1 caso contrário. */
static int rjs_stack_popobject(rjs_parser_t *parser);

/* Retorna o objeto no topo da pilha. Caso não haja objetos, retorna NULL. */
static rjs_object_t * rjs_stack_top(rjs_parser_t *parser);

/* Cria uma chave em um objeto, retornando 0 caso haja falhas. */
static int rjs_obj_pushkey(rjs_parser_t *parser, rjs_object_t *obj, rjs_key_t *key);

/* Cria uma chave do tipo string em um objeto, retornado 0 caso haja falhas. */
static int rjs_obj_pushkey_string(rjs_parser_t *parser, rjs_object_t *obj, const char *key, const char *str);

/* Cria uma chave do tipo número em um objeto, retornando 0 caso haja falhas. */
static int rjs_obj_pushkey_number(rjs_parser_t *parser, rjs_object_t *obj, const char *key, const char *str);

/* Cria uma chave do tipo objeto em um objeto, retornando 0 caso haja falhas. */
static int rjs_obj_pushkey_obj(rjs_parser_t *parser, rjs_object_t *obj, const char *key, rjs_object_t *new_obj);

/* Cria uma chave do tipo booleano em um objeto, retornando 0 caso haja falhas. */
static int rjs_obj_pushkey_bool(rjs_parser_t *parser, rjs_object_t *obj, const char *key, int value);

/* Cria uma chave do tipo null em um objeto, retornando 0 caso haja falhas. */
static int rjs_obj_pushkey_null(rjs_parser_t *parser, rjs_object_t *obj, const char *key);

/* Faz a leitura da string, retornando 0 caso haja erros. */
static int rjs_parse_object(rjs_parser_t *parser, const char *str, rjs_size_t *index);

/* Representa uma iteração da leitura da string, retornando 0 caso haja erros. */
static int rjs_parse_object_step(rjs_parser_t *parser, const char *str, rjs_size_t *index);

int rjs_create_parser(rjs_parser_t *parser, char *block, rjs_size_t size){
	if(block == NULL)
		return 0;

	parser->memory.block = block;
	parser->memory.size = size;
	parser->memory.top = 0;

	parser->start_object = NULL;
	
	return 1;
}

int rjs_clean_parser(rjs_parser_t *parser){
	parser->memory.top = 0;
	parser->start_object = NULL;
	parser->out_of_memory_flag = 0;

	parser->tmp_token_pos = parser->tmp_value_pos = 0;
	parser->object_stack_top = -1;

	parser->line_count = 1;

	return 1;
}

int rjs_parse_string(rjs_parser_t *parser, const char *str){
	rjs_size_t index = 0;

	rjs_clean_parser(parser);

	parser->start_object = rjs_create_obj(parser);
	rjs_stack_pushobject(parser, parser->start_object);

	parser->state = RJS_SEARCH_OPEN_BRACKET;
	parser->next_state = RJS_SEARCH_OPEN_BRACKET;
	return rjs_parse_object(parser, str, &index);
}

const char * rjs_get_error(rjs_parser_t *parser){
	return parser->error_log;
}

const rjs_object_t * rjs_get_main_object(rjs_parser_t *parser){
	return parser->start_object;
}

const rjs_key_t * rjs_get_key(const rjs_object_t *object, const char *name){
	rjs_key_t *key;

	if(object->start_key == NULL)
		return NULL;

	for(key = object->start_key; key != NULL; key = key->next){
		if(rjs_strcmp(key->name, name) == 0){
			return key;
		}
	}

	return NULL;
}

const rjs_key_t * rjs_get_key_index(const rjs_object_t *object, rjs_size_t pos){
	rjs_key_t *key;

	if(object->start_key == NULL)
		return NULL;

	for(key = object->start_key; key != NULL; key = key->next){
		if(pos == 0) break;
		pos--;
	}

	return key;
}

int rjs_istype(const rjs_key_t *key, int type){
	if(key == NULL) return 0;

	return key->value.type == type;
}

int rjs_isvalid(void *key){
	return key != NULL;
}

const rjs_object_t * rjs_get_vobj(const rjs_key_t *key){
	return key->value.data.obj;
}

const char * rjs_get_vstring(const rjs_key_t *key){
	return key->value.data.str;
}

double rjs_get_vnumber(const rjs_key_t *key){
	return key->value.data.number;
}

int rjs_get_vbool(const rjs_key_t *key){
	return key->value.data.r_bool;
}

static void rjs_log(rjs_parser_t *parser, const char *message){
	const char *line = " Line: ";
	rjs_size_t pos = 0;
	int line_count = parser->line_count;
	int div_10 = 100000;
	int found_number = 0;

	while(*message != '\0'){
		parser->error_log[pos++] = *(message++);

		if(pos == RJS_MAX_STRING_SIZE - 1)
			break;
	}

	while(*line != '\0'){
		parser->error_log[pos++] = *(line++);

		if(pos == RJS_MAX_STRING_SIZE - 1)
			break;
	}
	
	while(div_10 != 0){
		char c = '0' + (char) (line_count / div_10);

		if(c != '0' || found_number){
			parser->error_log[pos++] = '0' + (char) (line_count / div_10);
			found_number = 1;
		}

		line_count %= div_10;
		div_10 /= 10;

		if(pos == RJS_MAX_STRING_SIZE - 1)
			break;
	}

	parser->error_log[pos] = '\0';
}

static int rjs_add_character(char *str, rjs_size_t pos, char ch){
	if(pos >= RJS_MAX_STRING_SIZE - 1){
		str[RJS_MAX_STRING_SIZE - 1] = '\0';
		return 0;
	}
	
	str[pos] = ch;

	return 1;
}

static int rjs_isspace(char c){
	if(c == ' ' || c == '\n' || c == '\t' || c == '\r')
		return 1;

	return 0;
}

static int rjs_isstartnumber(char c){
	if((c >= '0' && c <= '9') || c == '-')
		return 1;

	return 0;
}

static double rjs_pow(double number, int exponent){
	double result;

	switch(exponent){
		case 0:
			return 1;
			break;

		case 1:
			return number;
			break;
			
		case -1:
			return 1.0 / number;
			break;
	}

	if(exponent < 0){
		number = 1.0 / number;
		exponent = -exponent;
	}

	result = 1.0;

	while(exponent != 0){
		if(exponent % 2 == 1){
			result = result * number;
		}

		number *= number;
		
		exponent /= 2;
	}

	return result;
}

static int rjs_strtod(const char *str, double *ret_number){
	rjs_size_t pos = 0;
	int state = RJS_STRTOD_READ_NUMBER;
	int is_negative = 1;
	double number = 0;
	double fraction = 0;
	double counter_fraction = 1;
	int exponent = 1;
	int is_negative_exponent = 1;

	if(str[0] == '\0') return 0;
	if(str[0] == '0' && (str[1] >= '0' && str[1] <= '9')) return 0;

	if(str[0] == '-'){
		pos++;
		is_negative = -1;
	}

	while(str[pos] != '\0'){
		switch(state){
			case RJS_STRTOD_READ_NUMBER:
				if(str[pos] >= '0' && str[pos] <= '9'){
					number = (str[pos] - '0') + number * 10;
				}
				else if(str[pos] == '.'){
					state = RJS_STRTOD_READ_FRACTION;
				}
				else if(str[pos] == 'e' || str[pos] == 'E'){
					state = RJS_STRTOD_READ_EXPONENT;
					exponent = 0;
				}
				else{
					return 0;
				}

				break;

			case RJS_STRTOD_READ_FRACTION:
				if(str[pos] >= '0' && str[pos] <= '9'){
					fraction = (str[pos] - '0') + fraction * 10;
					counter_fraction /= 10;
				}
				else if(str[pos] == 'e' || str[pos] == 'E'){
					state = RJS_STRTOD_READ_EXPONENT;
					exponent = 0;
				}
				else{
					return 0;
				}

				break;

			case RJS_STRTOD_READ_EXPONENT:
				if(str[pos] == '-'){
					is_negative_exponent = -1;
				}
				else if(str[pos] >= '0' && str[pos] <= '9'){
					exponent = (str[pos] - '0') + exponent * 10;
				}
				else{
					return 0;
				}

				break;
		}

		pos++;
	}

	*ret_number = (number + counter_fraction * fraction) * (is_negative);

	if(exponent != 1){
		*ret_number = rjs_pow(*ret_number, exponent * is_negative_exponent);
	}
	
	return 1;
}

static int rjs_strncmp(const char *str1, const char *str2, rjs_size_t n){
	while(n-- != 0){
		if(*str1 != *str2)
			return *str1 - *str2;

		str1++;
		str2++;
	}

	return 0;
}

static int rjs_strcmp(const char *str1, const char *str2){
	while(*(str1) == *(str2)){
		if(*str1 == '\0')
			return 0;

		str1++;
		str2++;
	}

	return *str1 - *str2;
}

static void *rjs_alloc(rjs_parser_t *parser, rjs_size_t size){
	void *location;

	if(parser->memory.top + size > parser->memory.size){
		parser->out_of_memory_flag = 1;
		return NULL;
	}

	location = (void *) (parser->memory.block + parser->memory.top);
	parser->memory.top += size;

	return location;
}

static const char *rjs_pushstring(rjs_parser_t *parser, const char *str){
	char *location;
	rjs_size_t size = 0;

	if(str == NULL)
		return NULL;

	while(str[size++] != '\0');

	/* padding */
	location = (char *) rjs_alloc(parser, size + 4 - (size % 4));

	if(location == NULL)
		return NULL;

	for(size = 0; str[size] != '\0'; size++)
		location[size] = str[size];

	location[size] = '\0';

	return (const char *) location;
}

static rjs_object_t *rjs_create_obj(rjs_parser_t *parser){
	rjs_object_t *object;

	object = (rjs_object_t *) rjs_alloc(parser, sizeof(rjs_object_t));

	if(object == NULL)
		return NULL;

	object->start_key = object->end_key = NULL;
	object->is_array = 0;

	return object;
}

static int rjs_stack_pushobject(rjs_parser_t *parser, rjs_object_t *obj){
	if(parser->object_stack_top == RJS_OBJECT_STACK_SIZE)
		return 0;

	parser->object_stack[++parser->object_stack_top] = obj;

	return 1;
}

static int rjs_stack_popobject(rjs_parser_t *parser){
	if(parser->object_stack_top == -1)
		return 0;

	parser->object_stack_top--;

	return 1;
}

static rjs_object_t * rjs_stack_top(rjs_parser_t *parser){
	if(parser->object_stack_top == -1)
		return NULL;

	return parser->object_stack[parser->object_stack_top];
}

static int rjs_obj_pushkey(rjs_parser_t *parser, rjs_object_t *obj, rjs_key_t *new_key){
	(void) parser;

	if(obj->start_key == NULL){
		obj->start_key = obj->end_key = new_key;
	}
	else{
		obj->end_key->next = new_key;
		obj->end_key = obj->end_key->next;
	}

	return 1;
}

static int rjs_obj_pushkey_string(rjs_parser_t *parser, rjs_object_t *obj, const char *key, const char *str){
	rjs_key_t *new_key = (rjs_key_t *) rjs_alloc(parser, sizeof(rjs_key_t));

	if(new_key == NULL)
		return 0;

	if(obj->is_array)
		new_key->name = NULL;
	else
		new_key->name = rjs_pushstring(parser, key);

	new_key->value.type = RJS_KEY_STRING;
	new_key->value.data.str = rjs_pushstring(parser, str);
	new_key->next = NULL;

	return rjs_obj_pushkey(parser, obj, new_key);
}

static int rjs_obj_pushkey_number(rjs_parser_t *parser, rjs_object_t *obj, const char *key, const char *str){
	rjs_key_t *new_key = (rjs_key_t *) rjs_alloc(parser, sizeof(rjs_key_t));

	if(new_key == NULL)
		return 0;

	if(obj->is_array)
		new_key->name = NULL;
	else
		new_key->name = rjs_pushstring(parser, key);

	new_key->value.type = RJS_KEY_NUMBER;

	if(!rjs_strtod(str, &new_key->value.data.number))
		return 0;

	new_key->next = NULL;

	return rjs_obj_pushkey(parser, obj, new_key);
}

static int rjs_obj_pushkey_obj(rjs_parser_t *parser, rjs_object_t *obj, const char *key, rjs_object_t *new_obj){
	rjs_key_t *new_key = (rjs_key_t *) rjs_alloc(parser, sizeof(rjs_key_t));

	if(new_key == NULL)
		return 0;

	if(obj->is_array)
		new_key->name = NULL;
	else
		new_key->name = rjs_pushstring(parser, key);

	new_key->value.type = RJS_KEY_OBJECT;
	new_key->value.data.obj = new_obj;
	new_key->next = NULL;

	return rjs_obj_pushkey(parser, obj, new_key);
}

static int rjs_obj_pushkey_bool(rjs_parser_t *parser, rjs_object_t *obj, const char *key, int value){
	rjs_key_t *new_key = (rjs_key_t *) rjs_alloc(parser, sizeof(rjs_key_t));

	if(new_key == NULL)
		return 0;

	if(obj->is_array)
		new_key->name = NULL;
	else
		new_key->name = rjs_pushstring(parser, key);

	new_key->value.type = RJS_KEY_BOOLEAN;
	new_key->value.data.r_bool = value;
	new_key->next = NULL;

	return rjs_obj_pushkey(parser, obj, new_key);
}

static int rjs_obj_pushkey_null(rjs_parser_t *parser, rjs_object_t *obj, const char *key){
	rjs_key_t *new_key = (rjs_key_t *) rjs_alloc(parser, sizeof(rjs_key_t));

	if(new_key == NULL)
		return 0;

	new_key->name = rjs_pushstring(parser, key);
	new_key->value.type = RJS_KEY_NULL;
	new_key->next = NULL;

	return rjs_obj_pushkey(parser, obj, new_key);
}

static int rjs_parse_object(rjs_parser_t *parser, const char *str, rjs_size_t *index){
	rjs_size_t previous_break = 0;

	while(str[(*index)] != '\0' && (rjs_stack_top(parser) != NULL)){

		if(!rjs_parse_object_step(parser, str, index)){
			if(parser->out_of_memory_flag){
				rjs_log(parser, "Out of memory.");
			}

			return 0;
		}

		if(parser->out_of_memory_flag){
			rjs_log(parser, "Out of memory.");
			return 0;
		}

		parser->state = parser->next_state;
		if(str[(*index)] == '\n' && previous_break != *index){
			previous_break = *index;
			parser->line_count++;
		}
	}

	if(rjs_stack_top(parser) != NULL){
		rjs_log(parser, "Expected close bracket or comma character.");
		return 0;
	}

	return 1;
}

static int rjs_parse_object_step(rjs_parser_t *parser, const char *str, rjs_size_t *index){
	rjs_object_t *top_object = rjs_stack_top(parser);
	char current = str[*index];
	int success = 1;

	switch(parser->state){
		case RJS_SEARCH_OPEN_BRACKET:
			if(rjs_isspace(current)){
				(*index)++;
			}
			else if(current == '{'){
				(*index)++;
				parser->next_state = RJS_SEARCH_TOKEN_STRING;
			}
			else{
				rjs_log(parser, "Expected open bracket.");
				success = 0;
			}

			break;

		case RJS_SEARCH_TOKEN_STRING:
			if(rjs_isspace(current)){
				(*index)++;
			}
			else if(current == '\"'){
				(*index)++;
				parser->next_state = RJS_READ_VALUE_STRING;

				parser->tmp_token_pos = 0;
				parser->tmp_value_pos = -1;
			}
			else if(current == '}'){
				parser->next_state = RJS_SEARCH_END;
			}
			else{
				rjs_log(parser, "Expected quote.");
				success = 0;
			}

			break;

		case RJS_SEARCH_COLON:
			if(rjs_isspace(current)){
				(*index)++;
			}
			else if(current == ':'){
				(*index)++;
				parser->next_state = RJS_SEARCH_VALUE;
			}
			else{
				success = 0;
				rjs_log(parser, "Expected colon.");
			}

			break;

		case RJS_SEARCH_VALUE:
			if(rjs_isspace(current)){
				(*index)++;
			}
			else if(current == '\"'){
				(*index)++;
				parser->next_state = RJS_READ_VALUE_STRING;
				parser->tmp_value_pos = 0;
				parser->tmp_token_pos = -1;
			}
			else if(rjs_isstartnumber(current)){
				parser->next_state = RJS_READ_VALUE_NUMBER;
				parser->tmp_value_pos = 0;
			}
			else if(current == '{'){
				rjs_object_t *new_obj = rjs_create_obj(parser);
				rjs_stack_pushobject(parser, new_obj);

				if(!rjs_obj_pushkey_obj(parser, top_object, parser->tmp_token, new_obj)){
					success = 0;
					parser->out_of_memory_flag = 1;
				}

				parser->next_state = RJS_SEARCH_OPEN_BRACKET;
			}
			else if(current == '['){
				rjs_object_t *new_obj = rjs_create_obj(parser);
				new_obj->is_array = 1;
				rjs_stack_pushobject(parser, new_obj);

				if(!rjs_obj_pushkey_obj(parser, top_object, parser->tmp_token, new_obj)){
					success = 0;
					parser->out_of_memory_flag = 1;
				}

				(*index)++;
				parser->next_state = RJS_SEARCH_VALUE;
			}
			else if(current == 't'){
				if(rjs_strncmp(str + *index, "true", 4) == 0){
					if(!rjs_obj_pushkey_bool(parser, top_object, parser->tmp_token, 1)){
						success = 0;
						parser->out_of_memory_flag = 1;
					}
				}

				*index += 4;
				parser->next_state = RJS_SEARCH_END;
			}
			else if(current == 'f'){
				if(rjs_strncmp(str + *index, "false", 5) == 0){
					if(!rjs_obj_pushkey_bool(parser, top_object, parser->tmp_token, 0)){
						success = 0;
						parser->out_of_memory_flag = 1;
					}
				}

				*index += 5;
				parser->next_state = RJS_SEARCH_END;
			}
			else if(current == 'n'){
				if(rjs_strncmp(str + *index, "null", 4) == 0){
					if(!rjs_obj_pushkey_null(parser, top_object, parser->tmp_token)){
						success = 0;
						parser->out_of_memory_flag = 1;
					}
				}

				*index += 4;
				parser->next_state = RJS_SEARCH_END;
			}
			else if(current == ']' || current == '}'){
				parser->next_state = RJS_SEARCH_END;
			}
			else{
				success = 0;
				rjs_log(parser, "Expected value: number, array, object, boolean or null.");
			}

			break;

		case RJS_READ_VALUE_STRING:
			if(current == '\"'){
				(*index)++;
				
				if(parser->tmp_value_pos == -1){
					parser->next_state = RJS_SEARCH_COLON;

					rjs_add_character(parser->tmp_token, parser->tmp_token_pos++, '\0');
				}
				else{
					parser->next_state = RJS_SEARCH_END;
					rjs_add_character(parser->tmp_value, parser->tmp_value_pos++, '\0');

					if(!rjs_obj_pushkey_string(parser, top_object, parser->tmp_token, parser->tmp_value)){
						success = 0;
						parser->out_of_memory_flag = 1;
					}
				}
			}
			else if(current == '\\'){
				(*index)++;

				parser->next_state = RJS_READ_VALUE_STRING_CONTROL;
			}
			else{
				(*index)++;

				if(parser->tmp_value_pos == -1)
					rjs_add_character(parser->tmp_token, parser->tmp_token_pos++, current);
				else
					rjs_add_character(parser->tmp_value, parser->tmp_value_pos++, current);
			}

			break;
			
		case RJS_READ_VALUE_STRING_CONTROL:
			{
				char control = '\0';

				if(current == '\"') control = '\"';
				else if(current == '\\') control = '\\';
				else if(current == '/') control = '/';
				else if(current == 'b') control = '\b';
				else if(current == 'f') control = '\f';
				else if(current == 'n') control = '\n';
				else if(current == 'r') control = '\r';
				else if(current == 't') control = '\t';

				if(control == '\0'){
					success = 0;
					rjs_log(parser, "Expected control character.");
				}
				else{
					if(parser->tmp_value_pos == -1)
						rjs_add_character(parser->tmp_token, parser->tmp_token_pos++, control);
					else
						rjs_add_character(parser->tmp_value, parser->tmp_value_pos++, control);

					(*index)++;
					parser->next_state = RJS_READ_VALUE_STRING;
				}
			}

			break;

		case RJS_READ_VALUE_NUMBER:
			if(rjs_isspace(current) || current == ',' || current == '}' || current == ']'){
				rjs_add_character(parser->tmp_value, parser->tmp_value_pos++, '\0');

				parser->next_state = RJS_SEARCH_END;

				if(!rjs_obj_pushkey_number(parser, top_object, parser->tmp_token, parser->tmp_value)){
					success = 0;
					rjs_log(parser, "Error while parsing number.");
				}
			}
			else{
				(*index)++;

				rjs_add_character(parser->tmp_value, parser->tmp_value_pos++, current);
			}

			break;

		case RJS_SEARCH_END:
			if(rjs_isspace(current)){
				(*index)++;
			}
			else if(current == ','){
				(*index)++;
				parser->next_state = RJS_SEARCH_TOKEN_STRING;

				if(top_object->is_array){
					parser->next_state = RJS_SEARCH_VALUE;
				}
			}
			else if(current == '}'){
				if(top_object->is_array){
					success = 0;
					rjs_log(parser, "Expected square bracket.");
				}
				else{
					(*index)++;
					rjs_stack_popobject(parser);
				}
			}
			else if(current == ']'){
				if(!top_object->is_array){
					success = 0;
					rjs_log(parser, "Expected close bracket.");
				}
				else{
					(*index)++;
					rjs_stack_popobject(parser);
				}
			}
			else{
				success = 0;
				rjs_log(parser, "Expected close bracket or comma character.");
			}

			break;
	}

	return success;
}
