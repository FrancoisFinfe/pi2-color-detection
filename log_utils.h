#ifndef _LOG_UTILS_H_
#define _LOG_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	GLOBAL_ERROR = -2,
	ERROR = -1,
	GLOBAL_SUCCESS = 0,
	SUCCESS = 1,
	WARNING = 2,
	INFO = 3,
	TRACE = 4,
	DEBUG_TRACE = 5,
	DDEBUG_TRACE = 6,

} message_type_t;


void log_print(message_type_t verbosity, message_type_t config_verb, const char *format, ...);
void log_hexdump(message_type_t verbosity, message_type_t config_verb, const unsigned char *data, int size);


#ifdef __cplusplus
}
#endif


#endif
