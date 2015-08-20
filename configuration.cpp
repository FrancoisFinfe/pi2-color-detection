
#include "configuration.hpp"

#include <string.h>




#include <iostream>

//void log_print(enum eMessageType type, enum eMessageType config_verb, char *format, ...);
#define configuration_log(LEVEL, MSG, ...)		log_print(LEVEL, ctx->verbo, MSG, __VA_ARGS__)


int configuration_init(configuration_context_t *ctx, const char *config_file_path, message_type_t verbosity){

	try{
		ctx->verbo = verbosity;
		ctx->conf = (pi2_color_detect_config_(config_file_path , ::xml_schema::flags::dont_validate));



	}
	catch (const xml_schema::exception& e)
	{
		ostringstream stream;
		stream << e;
		log_print(ERROR, ctx->verbo, "%s: %s\n", __FUNCTION__ , stream.str().c_str() );

		return -1;
	}
	catch (const xml_schema::properties::argument&)
	{
		log_print(ERROR, ctx->verbo, "%s: %s\n", __FUNCTION__ , "invalid property argument (empty namespace or location)\n" );
		return -1;
	}
	catch (const xsd::cxx::xml::invalid_utf16_string&)
	{
		log_print(ERROR, ctx->verbo, "%s: %s\n", __FUNCTION__ , "invalid UTF-16 text in DOM model\n" );
		return -1;
	}
	catch (const xsd::cxx::xml::invalid_utf8_string&)
	{
		log_print(ERROR, ctx->verbo, "%s: %s\n", __FUNCTION__ , "invalid UTF-8 text in object model" );
		return -1;
	}

	/* no exception, no error */
	return 0;


}

int configuration_get(configuration_context_t *ctx, color_config_t *c_config, int array_size){

	int index = 0,b_index = -1, i;
	::pi2_color_detect_config::color_sequence& cs (ctx->conf->color());

	::pi2_color_detect_config::color_iterator it (cs.begin());


	for(; it != cs.end() && index < array_size ; ++it) {
		::pi2_color_detect_config::color_type& color(*it);


		if(color.description() == "none") {
			// none element found, backup current index and set index to point the last element of array
			b_index = index;
			index = array_size-1;
		}

		strncpy(c_config[index].name, color.description().c_str(), sizeof(c_config[0].name) );

		c_config[index].hue.min = color.hue().min();
		c_config[index].hue.max = color.hue().max();

		c_config[index].sat.min = color.sat().min();
		c_config[index].sat.max = color.sat().max();

		c_config[index].lum.min = color.lum().min();
		c_config[index].lum.max = color.lum().max();

		c_config[index].gpio_mask = color.gpio_mask();
		c_config[index].gpio_value = color.gpio_value();


		log_print(INFO, ctx->verbo, "%s: color %d \"%s\", hue min %f max %f, sat min %f max %f, lum min %f max %f, gpio_mask %x, gpio_value %x\n",
				__FUNCTION__ , index,
				c_config[index].name,
				c_config[index].hue.min,
				c_config[index].hue.max,
				c_config[index].sat.min,
				c_config[index].sat.max,
				c_config[index].lum.min,
				c_config[index].lum.max,
				c_config[index].gpio_mask,
				c_config[index].gpio_value
				);

		if(b_index != -1){
			index = b_index;
			b_index = -1;
		}else{
			index++;
		}


	}

	for(i=index; i<(array_size-1); i++){
		memset(&c_config[i], 0, sizeof(color_config_t));
	}

	return index;

}





void configuration_free(configuration_context_t *ctx){
	/* reset will call delete method and assign null value to null*/
	ctx->conf.reset();
}


