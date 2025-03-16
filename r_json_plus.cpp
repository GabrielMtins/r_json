#include "r_json_plus.hpp"
#include <iostream>

namespace rjs {

	/* Key */

	Key::Key(const rjs_key_t *key){
		this->key = key;
	}

	bool Key::isValid(void){
		return rjs_isvalid((void *) key);
	}

	template <>
	double Key::get<double>(void){
		if(rjs_istype(key, RJS_KEY_NUMBER)){
			return rjs_get_vnumber(key);
		}

		return 0.0;
	}

	template <>
	float Key::get<float>(void){
		if(rjs_istype(key, RJS_KEY_NUMBER)){
			return (float) rjs_get_vnumber(key);
		}

		return 0.0f;
	}

	template <>
	int Key::get<int>(void){
		if(rjs_istype(key, RJS_KEY_NUMBER)){
			return (int) rjs_get_vnumber(key);
		}

		return 0;
	}

	template <>
	unsigned int Key::get<unsigned int>(void){
		if(rjs_istype(key, RJS_KEY_NUMBER)){
			return (unsigned int) rjs_get_vnumber(key);
		}

		return 0;
	}

	template <>
	long Key::get<long>(void){
		if(rjs_istype(key, RJS_KEY_NUMBER)){
			return (long) rjs_get_vnumber(key);
		}

		return 0;
	}

	template <>
	unsigned long Key::get<unsigned long>(void){
		if(rjs_istype(key, RJS_KEY_NUMBER)){
			return (unsigned long) rjs_get_vnumber(key);
		}

		return 0;
	}

	template <>
	const char * Key::get<const char *>(void){
		if(rjs_istype(key, RJS_KEY_STRING)){
			return rjs_get_vstring(key);
		}

		return nullptr;
	}

	template <>
	bool Key::get<bool>(void){
		if(rjs_istype(key, RJS_KEY_BOOLEAN)){
			return (bool) rjs_get_vbool(key);
		}

		return true;
	}

	template <>
	Object Key::get<Object>(void){
		if(rjs_istype(key, RJS_KEY_OBJECT)){
			return Object(rjs_get_vobj(key));
		}

		return Object(nullptr);
	}

	/* Object */

	Object::Object(const rjs_object_t *object){
		this->object = object;
	}

	bool Object::isValid(void){
		return rjs_isvalid((void *) object);
	}

	Key Object::operator[](rjs_size_t index){
		return Key(rjs_get_key_index(object, index));
	}

	Key Object::operator[](const char *str){
		return Key(rjs_get_key(object, str));
	}

	/* Parser */

	Parser::Parser(void *block, rjs_size_t size){
		rjs_create_parser(&parser, (char *) block, size);
	}

	Object Parser::getMainObject(void){
		return Object(rjs_get_main_object(&parser));
	}

	bool Parser::parse(const char *str){
		return rjs_parse_string(&parser, str);
	}

	const char * Parser::getError(){
		return rjs_get_error(&parser);
	}

};
