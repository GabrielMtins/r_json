#ifndef R_JSON_PLUS_HPP
#define R_JSON_PLUS_HPP

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
