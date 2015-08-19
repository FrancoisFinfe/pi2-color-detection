#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#include <string.h>
#include <limits.h>
#include <sys/time.h>

#include "log_utils.h"


void log_hexdump(enum eMessageType verbosity, enum eMessageType config_verb, const unsigned char *data, int size)
{
	int i;

	log_print(verbosity, config_verb, "%08x: ", 0);


	for(i=0; i<size ; i++){
		if(i && (i%16 == 0)){
			log_print(verbosity, config_verb, "\n");
			if(i<size)
				log_print(verbosity, config_verb, "%08x: ", i);
		}
		log_print(verbosity, config_verb, "%02x ", data[i]);
	}

	log_print(verbosity, config_verb, "\n");
}


void log_print(enum eMessageType type, enum eMessageType config_verb, char *format, ...)
{

	FILE *stream;

	if(type >= SUCCESS){
		stream = stdout;
	}else{
		/* error message */
		stream = stderr;

	}



	if(config_verb >= type)
	{


		va_list args;

		va_start(args, format);
		vfprintf(stream, format, args);
		va_end(args);



	}
}


