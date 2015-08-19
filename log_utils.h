#ifndef _LOG_UTILS_H_
#define _LOG_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>



enum eMessageType{
	GLOBAL_ERROR = -2,
	ERROR = -1,
	GLOBAL_SUCCESS = 0,
	SUCCESS = 1,
	WARNING = 2,
	INFO = 3,
	TRACE = 4,
	DEBUG_TRACE = 5,
	DDEBUG_TRACE = 6,

};


void log_print(enum eMessageType type, enum eMessageType config_verb, char *format, ...);
void log_hexdump(enum eMessageType verbosity, enum eMessageType config_verb, const unsigned char *data, int size);



#endif
