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
	class Object;

	class Key {
		private:
			Key(const rjs_key_t *key);

			const rjs_key_t *key;

		public:
			template <typename T>
			T get(void){
				return T();
			}

			bool isValid(void);

		friend class Object;
	};

	class Object {
		private:
			const rjs_object_t *object;
			Object(const rjs_object_t *object);

		public:
			Key operator[](rjs_size_t index);
			Key operator[](const char *str);
			bool isValid(void);
		
		friend class Parser;
		friend class Key;
	};


	template <>
	Object Key::get<Object>(void);

	class Parser {
		private:
			rjs_parser_t parser;

		public:
			Parser(void *block, rjs_size_t size);
			Object getMainObject(void);
			bool parse(const char *str);
			const char * getError();
	};

};

#endif
