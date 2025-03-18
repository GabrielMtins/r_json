#ifndef R_JSON_PLUS_HPP
#define R_JSON_PLUS_HPP

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

#include "r_json.h"

namespace rjs {
	/* Para facilitar a escrita. */
	typedef const char * string;

	class Object;

	class Key {
		private:
			Key(const rjs_key_t *key);

			const rjs_key_t *key;

		public:
			/* Retorna o valor da chave do tipo desejado.
			 * Os tipos suportados são:
			 * Tipos numéricos: int, float, double, unsigned int, unsigned long
			 * Tipos de string: const char *
			 * Tipos booleanos: bool
			 * Tipos de objeto : rjs::Object
			 * */
			template <typename T>
			T get(void){
				return T();
			}

			string getName(void);

			/* Retorna true caso a chave seja válida. */
			bool isValid(void);

			/* Avança para a próxima chave. Retorna true caso haja a próxima chave. */
			bool next(void);

			/* Retorna o tipo segundo a enumeração da r_json.h */
			int getType(void);

			/* Retorna o nome do tipo. */
			string getTypeName(void);


		friend class Object;
	};

	class Object {
		private:
			const rjs_object_t *object;
			Object(const rjs_object_t *object);

		public:
			/* Retorna a chave na posição indicada pelo index. */
			Key operator[](rjs_size_t index);
			Key operator[](int index);
			/* Retorna a chave dada pela string str. */
			Key operator[](const char *str);
			/* Retorna true caso o objeto seja válido. */
			bool isValid(void);
			/* Retorna true caso o objeto seja uma array. */
			bool isArray(void);
		
		friend class Parser;
		friend class Key;
	};


	template <>
	Object Key::get<Object>(void);

	class Parser {
		private:
			rjs_parser_t parser;

		public:
			/* Cria um parser com a memória e o tamanho dado. */
			Parser(void *block, rjs_size_t size);
			/* Retorna o objeto principal. */
			Object getMainObject(void);
			/* Decodifica uma string. Retorna true caso haja sucesso. */
			bool parse(const char *str);
			/* Retorna uma mensagem de erro. */
			const char * getError();
	};

};

#endif
