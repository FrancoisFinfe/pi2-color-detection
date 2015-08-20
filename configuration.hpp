#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include <stdlib.h>
#include "log_utils.h"


#include "settings.hxx"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif


typedef struct float_spec_s{
	double min;
	double max;
} float_spec_t;


typedef struct color_config_s{
	char name[64];
	unsigned int gpio_value;	// "user" gpio bit assignment (config)
	unsigned int gpio_mask;
	/* bcm2835 gpio bit assignment. This is not a configuration parameter.
	 * After reading the config, gpio_value is converted to bcm2835_gpio_value
	 */
	unsigned int bcm2835_gpio_value;
	unsigned int bcm2835_gpio_mask;


	float_spec_t hue;
	float_spec_t sat;
	float_spec_t lum;

} color_config_t;


typedef struct configuration_context{
	std::auto_ptr<pi2_color_detect_config> conf;
	message_type_t verbo;	// current verbosity
	
} configuration_context_t;

int configuration_init(configuration_context_t *ctx, const char *config_file_path, message_type_t verbosity);
int configuration_get(configuration_context_t *ctx, color_config_t *c_config, int array_size);
void configuration_free(configuration_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
