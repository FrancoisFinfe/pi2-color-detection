

#include <bcm2835.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "tcs34725.h"

#include "RunningAverage.h"
#include "configuration.hpp"

#undef COLOR_AVERAGE_SAMPLES


const char *default_config_file_path = (char *) "settings.xml";
const char *prog	= "pi2-color-detection";
char config_file_path[PATH_MAX] = {0};
char *i2c_dev_name = (char *) "/dev/i2c-1";

// Colors
enum color_e {
	RED = 0,
	GREEN = 1,
	BLUE = 2,
	CLEAR = 3
};

uint16_t measure[4];
uint16_t color[4];

#ifdef COLOR_AVERAGE_SAMPLES

// Running average
RunningAverage ra[4] = {RunningAverage(COLOR_AVERAGE_SAMPLES),RunningAverage(COLOR_AVERAGE_SAMPLES),RunningAverage(COLOR_AVERAGE_SAMPLES),RunningAverage(COLOR_AVERAGE_SAMPLES)};

#endif
// this array converts user pin number to bcm2835
// e.g. user bit 1 -->  user_bcm2835_pin_lut[1] => 17 bcm2835 pin
uint8_t user_bcm2835_pin_lut[] = {
	4, 17, 18, 27, 22, 23, 24, 25,
	5, 6, 12, 13, 19, 16, 26, 20
};



#define EHW298_GPIO_COUNT	4
#define COLOR_CONFIG_COUNT	9
#define ACT_TOGGLE_LED		3
uint16_t normalise_max_rgb;
tcs34725 *tcs;
configuration_context_t config_ctx;

color_config_t color_configs[COLOR_CONFIG_COUNT];

/* The last element must be the "none color" configuration
 * If the detected color doesn't match to any configured color,
 * the none config is "applied".
 * Only gpio_val & gpio_mask of none config are used by program, other are ignored
 */




color_config_t* find_color_config_by_name(const char *name){
	int i;

	for(i=0 ; i<COLOR_CONFIG_COUNT ; i++){
		if (!strncmp(name, color_configs[i].name, strlen(name)))
			return &color_configs[i];
	}

	/* no config found */

	return NULL;

}




void convert_gpio_user_to_bcm2835(uint32_t user, uint32_t *bcm2835){
	int i;

	*bcm2835 = 0;

	for(i=0 ; i < (EHW298_GPIO_COUNT > 32 ? 32 : EHW298_GPIO_COUNT) && user ; i++){
		if(user&1){
			*bcm2835 |= 1 << user_bcm2835_pin_lut[i];
		}

		user >>= 1;
	}


}



void convert_color_configs_user_to_bcm2835(color_config_t *configs, int array_size){
	int i;


	for(i=0 ; i< array_size ; i++){
		convert_gpio_user_to_bcm2835(configs[i].gpio_value, &configs[i].bcm2835_gpio_value);
		convert_gpio_user_to_bcm2835(configs[i].gpio_mask, &configs[i].bcm2835_gpio_mask);
	}

}


/* if conf_file is null, this function will set the default value for */

void define_default_file_path(char conf_file[PATH_MAX]){
	char *c;

	if(!strlen(conf_file)){
		size_t len = readlink("/proc/self/exe", conf_file, PATH_MAX-1);
		conf_file[len] = '\0';
		// will return the executable path

		//remove file name, keep only the dir
		c = strrchr(conf_file, '/');

		if(c){
			*(c+1) = '\0';	// next char after the last "\"
			strcat(conf_file, default_config_file_path);
		}else{
			strcpy(conf_file, default_config_file_path);
		}
	}

}

int init(void){
	int res = 0;

	if (!bcm2835_init()) {
		printf("bcm2835 init failed\n");
		return -1;
	}

#ifdef ACT_TOGGLE_LED
	bcm2835_gpio_fsel(user_bcm2835_pin_lut[ACT_TOGGLE_LED], BCM2835_GPIO_FSEL_OUTP);
#endif

#if 0
	int i;
	for(i=0; i<EHW298_GPIO_COUNT; i++){
		bcm2835_gpio_fsel(user_bcm2835_pin_lut[i], BCM2835_GPIO_FSEL_OUTP);
	}
#endif



	tcs = new tcs34725(i2c_dev_name, TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
	normalise_max_rgb = 20480;	// see TCS34725 datasheet
	
	if (tcs->begin()) {
		/* sensor found */			
	} else {
		printf("No TCS34725 found\n");
		return -2;		
	}

	res = configuration_init(&config_ctx, config_file_path, DEBUG_TRACE);

	if(res < 0){
		printf("Reading configuration failed\n");
		return res;
	}

	configuration_get(&config_ctx, color_configs, COLOR_CONFIG_COUNT);

	convert_color_configs_user_to_bcm2835(&color_configs[0], COLOR_CONFIG_COUNT);


	return res;
		


}

#ifdef COLOR_AVERAGE_SAMPLES

void init_average(void) {
  ra[RED].clear();
  ra[GREEN].clear();
  ra[BLUE].clear();
  ra[CLEAR].clear();
}

#endif


void set_gpio_color(const color_config_t *color) {
	bcm2835_gpio_set_output_mask(color->bcm2835_gpio_mask);
	bcm2835_gpio_write_mask(color->bcm2835_gpio_value, color->bcm2835_gpio_mask);
}

color_config_t *find_matching_color(const hsl_t *hsl) {
	color_config_t *found = NULL;
	color_config_t *cur = NULL;
	int i;

	for(i=0 ; i< (COLOR_CONFIG_COUNT-1) ; i++){
		cur = &color_configs[i];

		if ( (((cur->hue.min <= cur->hue.max) &&  (hsl->h > cur->hue.min) && (hsl->h < cur->hue.max)) ||
			  ((cur->hue.min >  cur->hue.max) && ((hsl->h > cur->hue.min) || (hsl->h < cur->hue.max)))) && /* e.g. min = 350° max 10°, red */
			 (hsl->s > cur->sat.min) &&
			 (hsl->s < cur->sat.max) &&
			 (hsl->l > cur->lum.min) &&
			 (hsl->l < cur->lum.max)) {


			if(!found)
				found = cur;
			else{
				/* if another color was found previously, chose the closest in hue */
				double hue_cur;
				double hue_found;
				hue_cur = (cur->hue.min + cur->hue.max) /2;
				hue_found = (found->hue.min + found->hue.max) /2;


				if( fabs(hsl->h - hue_cur) < fabs(hsl->h - hue_found) ){
					found = cur;
				}
			}
		}

	}

	/* if no color found, return none element */
	if(!found)
		found = &color_configs[COLOR_CONFIG_COUNT - 1];

	return found;

}


void detect_color_it(int verbose) {

	color_config_t *match_color;
#ifdef COLOR_AVERAGE_SAMPLES
	tcs->getRawData(&measure[RED], &measure[GREEN], &measure[BLUE], &measure[CLEAR]);


	ra[RED].addValue(measure[RED]);
	ra[GREEN].addValue(measure[GREEN]);
	ra[BLUE].addValue(measure[BLUE]);
	ra[CLEAR].addValue(measure[CLEAR]);

	color[RED] = ra[RED].getAverage();
	color[GREEN] = ra[GREEN].getAverage();
	color[BLUE] = ra[BLUE].getAverage();
	color[CLEAR] = ra[CLEAR].getAverage();

#else
	tcs->getRawData(&color[RED], &color[GREEN], &color[BLUE], &color[CLEAR]);
#endif
	hsv_t hsv = tcs->calculateRgbInt2Hsv(color[RED], color[GREEN], color[BLUE], normalise_max_rgb);
	hsl_t hsl = tcs->calculateRgbInt2Hsl(color[RED], color[GREEN], color[BLUE], normalise_max_rgb);

	match_color = find_matching_color(&hsl);

	if(verbose){
		printf("\e[2J"); // clear screen
		printf("\e[0;0H");


		printf("R: \e[01;31m%9d\e[00;37m G: \e[01;32m%9d\e[00;37m B: \e[01;34m%9d\e[00;37m :C \e[01;37m%9d\e[00;37m\n",
			color[RED],color[GREEN],color[BLUE],color[CLEAR]);

		printf("H: \e[01;31m%9.5f\e[00;37m S: \e[01;32m%9.5f\e[00;37m V: \e[01;34m%9.5f\e[00;37m \n",
			hsv.h, hsv.s, hsv.v);
	
		printf("H: \e[01;31m%9.5f\e[00;37m S: \e[01;32m%9.5f\e[00;37m L: \e[01;34m%9.5f\e[00;37m \n",
			hsl.h, hsl.s, hsl.l);

		if(match_color){
			printf("\nCurrent color: \e[01;37m\"%s\"\e[00;37m, gpio: %x, mask: %x",
					match_color->name,
					match_color->gpio_value,
					match_color->gpio_mask);
		}
	}
 


	if(match_color) {
		set_gpio_color(match_color);
	}

  /*
  if(color[CLEAR] > 1000){
    

    if(color[RED] > 2*color[GREEN] && color[RED] > 2*color[BLUE])
      printf("RED!");
    else if(color[RED] > 2*color[BLUE] && color[GREEN] > 2*color[BLUE])
      printf("YELLOW!");
    else if(color[BLUE] > color[RED] && color[BLUE] > color[GREEN])
      printf("BLUE!");


  }
   */

	fflush(stdout);

 
}




int waitKeyPress(uint32_t timeout_second){
   fd_set fdset;
   struct timeval timeout;
   int  rc;
   int  val;

	printf("Press any key to continue (timeout = %d s)", timeout_second);
	fflush(stdout);

   timeout.tv_sec = timeout_second;   /* wait for 6 seconds for data */
   timeout.tv_usec = 0;


   FD_ZERO(&fdset);

   FD_SET(STDIN_FILENO, &fdset);

   rc = select(1, &fdset, NULL, NULL, &timeout);
   if (rc == -1)  /* select failed */
   {
		val = -1;
   }
   else if (rc == 0)  /* select timed out */
   {
		val = 0;
   }
   else 
   {
      if (FD_ISSET(STDIN_FILENO, &fdset)) 
      {
         val = getchar();
      }
   }

	printf("\n");

	return val;

}


void command_usage(void){
	printf("usage:  %s [options]\n", prog);
	printf("Options are:\n");
	printf(" -h             help\n");
	printf(" -c conf_file   xml config file path (default = settings.xml)\n");
	printf(" -d i2c_dev     i2c dev name (default = /dev/i2c-1)\n");
	printf(" -s             silent mode (don't dumb output value)\n");


}



int main(int argc, char **argv)
{
	// If you call this, it will not actually access the GPIO
	// Use for testing
	//    bcm2835_set_debug(1);
	int i,c, silent_mode = 0;
	int res;

	

	while((c = getopt(argc, argv, "c:d:hs")) != -1) {
		switch(c){
			case 'c':
				strcpy(config_file_path, optarg);
				break;
			case 's':
				silent_mode = 1;
				break;
			case 'd':
				i2c_dev_name = optarg;
				break;
			case '?':
			case 'h':
			default:
			
			command_usage();
			exit(1);
			break;

		}
	}

	define_default_file_path(config_file_path);

	printf("Config file: %s\n", config_file_path);
	printf("I2C-dev device name: %s\n", i2c_dev_name);
	

	res = init();
	
	if(res < 0) {
		printf("Error: init failed\n");
		return res;
	}

#ifdef COLOR_AVERAGE_SAMPLES
	init_average();
#endif

	waitKeyPress(10);



    // Blink
    while (1)
    {
			// Turn it on
#ifdef ACT_TOGGLE_LED
			bcm2835_gpio_write(user_bcm2835_pin_lut[ACT_TOGGLE_LED], (i++)&1 ? HIGH : LOW);
#endif
			// wait a bit
			usleep(1*1000);
			
			detect_color_it(silent_mode ? 0 : 1);
	
	
    }

	bcm2835_close();	
   return 0;
}


