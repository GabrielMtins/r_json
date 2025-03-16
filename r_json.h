#ifndef R_JSON_H
#define R_JSON_H

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

#ifdef __cplusplus
extern "C" {
#endif

#define RJS_MAX_STRING_SIZE 256
#define RJS_OBJECT_STACK_SIZE 256

typedef unsigned long rjs_size_t;

/* Enum para os tipos de valores que uma chave
 * em json pode ser. */
enum rjs_key_type {
	RJS_KEY_STRING = 0,
	RJS_KEY_OBJECT,
	RJS_KEY_NUMBER,
	RJS_KEY_BOOLEAN,
	RJS_KEY_NULL
};

typedef struct rjs_object_s rjs_object_t;

/* Estrutura para o manuseamento de memória. */
typedef struct rjs_mem_s {
	char *block; /* Bloco de memória. */
	rjs_size_t size; /* Tamanho do bloco de memória. */
	rjs_size_t top; /* Topo da memória para indicar onde ela está livre. */
} rjs_mem_t;

/* Estrutura que guardará os dados do parser de json. */
typedef struct {
	rjs_mem_t memory;
	rjs_object_t *start_object;

	/* String temporária para armazenar os nomes de tokens. */
	char tmp_token[RJS_MAX_STRING_SIZE];
	int tmp_token_pos;

	/* String temporária para armazenar os valores (números, booleans
	 * null, strings etc). */
	char tmp_value[RJS_MAX_STRING_SIZE];
	int tmp_value_pos;

	/* Saída de erro, caso haja. */
	char error_log[RJS_MAX_STRING_SIZE];

	/* Stack de objetos. Utilizado para a recursão, e para
	 * saber qual é o objeto que está sendo trabalhado. O objeto
	 * trabalho sempre será o do topo. */
	rjs_object_t *object_stack[RJS_OBJECT_STACK_SIZE];
	int object_stack_top;

	/* Flag para indicar que há falta de memória. */
	int out_of_memory_flag;

	/* Flags de estado. O parser funciona como uma máquina de estado. */
	int state;
	int next_state;

	/* Contador de linha, utilizado para mensagens de erros. */
	int line_count;
} rjs_parser_t;

/* Estrutura para organizar os valores de json. */
typedef struct {
	/* União representando os valores possíveis que um
	 * valor em json pode assumir. */
	union value_u {
		const char *str;
		double number;
		const rjs_object_t *obj;
		int r_bool;
	} data;

	/* O tipo, definido segundo o enum declarado no topo
	 * desse header. */
	int type;
} rjs_value_t;

typedef struct rjs_key_s {
	/* Esse ponteiro será constante para o usuário da biblioteca
	 * não modificar seu conteúdo. Todas as strings serão armazenadas pelo
	 * próprio manuseador de memória da biblioteca.
	 * Caso a chave seja uma chave de array, ela será marcada como NULL. */
	const char *name;
	rjs_value_t value;

	/* Indica a próxima chave, em formado de lista encadeada. */
	struct rjs_key_s *next;
} rjs_key_t;

struct rjs_object_s {
	/* Indica o primeiro objeto e o último, respectivamente. */
	rjs_key_t *start_key;
	rjs_key_t *end_key;

	/* Indica caso o objeto seja uma array. */
	int is_array;
};

/* Cria o parser, com um dado bloco de memória e o seu tamanho.
 * Retorna 1 caso crie o parser com sucesso, 0 em caso contrário. */
int rjs_create_parser(rjs_parser_t *parser, char *block, rjs_size_t size);

/* Limpa a memória do parser para que realize-se uma nova leitura. Sempre é 
 * chamada pela função rjs_create_parser, então não é preciso se preocupar. */
int rjs_clean_parser(rjs_parser_t *parser);

/* Lê e decodifica uma string de json. Retorna 1 caso haja sucesso e 0
 * em caso contrário.
 * Caso haja um erro, ele será impresso no log interno. Chame a função
 * rjs_get_error() para receber a mensagem de erro. */
int rjs_parse_string(rjs_parser_t *parser, const char *str);

/* Retorna a mensagem de erro, caso haja. */
const char * rjs_get_error(rjs_parser_t *parser);

/* Retorna o objeto principal do arquivo json. Caso não haja,
 * retorna NULL */
const rjs_object_t * rjs_get_main_object(rjs_parser_t *parser);

/* Retorna a chave com um dado nome. Caso esse nome não seja encontrado,
 * retorna NULL.
 * A busca é linear, pois a lista utilizada é uma lista encadeada. */
const rjs_key_t * rjs_get_key(const rjs_object_t *object, const char *name);

/* Retorna a chave em uma dada posição. Útil para arrays.
 * Caso não haja uma chave naquela posição, retorna NULL. */
const rjs_key_t * rjs_get_key_index(const rjs_object_t *object, rjs_size_t pos);

/* Retorna 1 se a chave é de um dado tipo, 0 caso contrário. */
int rjs_istype(const rjs_key_t *key, int type);

/* Retorna 1 se um object da biblioteca é válido, 0 caso contrário. */
int rjs_isvalid(void *key);

/* Retorna o objeto armazenado em uma chave. Verifique previamente
 * qual o tipo da chave. */
const rjs_object_t * rjs_get_vobj(const rjs_key_t *key);

/* Retorna a string armazenado em uma chave. Essa string não deve ser modificada,
 * por isso é uma const char *. Verifique previamente qual o tipo da chave. */
const char * rjs_get_vstring(const rjs_key_t *key);

/* Retorna o número armazenado em uma chave. Verifique previamente qual o tipo
 * da chave. */
double rjs_get_vnumber(const rjs_key_t *key);

/* Retorna o valor booleano armazenado em uma chave. Verifique previamente qual
 * o tipo da chave. */
int rjs_get_vbool(const rjs_key_t *key);

#ifdef __cplusplus
}
#endif

#endif
