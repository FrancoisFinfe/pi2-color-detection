

#include <bcm2835.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "tcs34725.h"

#include "RunningAverage.h"

#define AverageSamples 5

const char *prog	= "pi2-color-detection";
char *config_file_path = (char *) "settings.xml";
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

// Running average
RunningAverage ra[4] = {RunningAverage(AverageSamples),RunningAverage(AverageSamples),RunningAverage(AverageSamples),RunningAverage(AverageSamples)};


uint8_t ehw298_bcm2835_pin[] = {
	4, 17, 18, 27, 22, 23, 24, 25,
	5, 6, 12, 13, 19, 16, 26, 20
};



#define ehw298_gpio_count	4

uint16_t normalise_max_rgb;

tcs34725 *tcs;



int init(void){
	int res = 0;
	int i;

	if (!bcm2835_init()) {
		printf("bcm2835 init failed\n");
		return -1;
	}

	for(i=0; i<ehw298_gpio_count; i++){
		bcm2835_gpio_fsel(ehw298_bcm2835_pin[i], BCM2835_GPIO_FSEL_OUTP);	
	}



	tcs = new tcs34725(i2c_dev_name, TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
	normalise_max_rgb = 20480;	// see TCS34725 datasheet
	
	if (tcs->begin()) {
		/* sensor found */			
	} else {
		printf("No TCS34725 found\n");
		return -2;		
	}


	return res;
		


}


void init_average(void) {
  ra[RED].clear();
  ra[GREEN].clear();
  ra[BLUE].clear();
  ra[CLEAR].clear();
}



void detect_color_it(void) {

  tcs->getRawData(&measure[RED], &measure[GREEN], &measure[BLUE], &measure[CLEAR]);
 
  ra[RED].addValue(measure[RED]);
  ra[GREEN].addValue(measure[GREEN]);
  ra[BLUE].addValue(measure[BLUE]);
  ra[CLEAR].addValue(measure[CLEAR]);
 
  color[RED] = ra[RED].getAverage();
  color[GREEN] = ra[GREEN].getAverage();
  color[BLUE] = ra[BLUE].getAverage();
  color[CLEAR] = ra[CLEAR].getAverage();

  hsv_t hsv = tcs->calculateRgbInt2Hsv(color[RED], color[GREEN], color[BLUE], normalise_max_rgb);

  printf("\e[0;0H");
  

  printf("R: \e[01;31m%6d\e[00;37m G \e[01;32m%6d\e[00;37m B \e[01;34m%6d\e[00;37m C \e[01;37m%6d\e[00;37m\n",
		color[RED],color[GREEN],color[BLUE],color[CLEAR]);
	
  printf("H: \e[01;31m%0.5f\e[00;37m S \e[01;32m%0.5f\e[00;37m V \e[01;34m%0.5f\e[00;37m \n",
		hsv.h, hsv.s, hsv.v);

 
  if(color[CLEAR] > 1000){
    
/*
    if(color[RED] > 2*color[GREEN] && color[RED] > 2*color[BLUE])
      printf("RED!");
    else if(color[RED] > 2*color[BLUE] && color[GREEN] > 2*color[BLUE])
      printf("YELLOW!");
    else if(color[BLUE] > color[RED] && color[BLUE] > color[GREEN])
      printf("BLUE!");*/
  }
 

 
}





void command_usage(void){
	printf("usage:  %s [options]\n", prog);
	printf("Options are:\n");
	printf(" -h              help\n");
	printf(" -c              xml config file path (default = settings.xml)\n");
	printf(" -d              i2c dev name (default = /dev/i2c-1)\n");

}



int main(int argc, char **argv)
{
	// If you call this, it will not actually access the GPIO
	// Use for testing
	//    bcm2835_set_debug(1);
	int i,c;
	int res;


	while((c = getopt(argc, argv, "c:d:h")) != -1) {
		switch(c){
			case 'c':
				config_file_path = optarg;

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

	printf("Config file: %s\n", config_file_path);
	printf("I2C-dev device name: %s\n", i2c_dev_name);
	

	res = init();
	
	if(res < 0) {
		printf("Error: init failed\n");
		return res;
	}

	init_average();

	sleep(1);
	printf("\e[2J"); // clear screen


    // Blink
    while (1)
    {
			// Turn it on
			bcm2835_gpio_write(ehw298_bcm2835_pin[3], (i++)&1 ? HIGH : LOW);
	
			// wait a bit
			usleep(1*1000);
			
			detect_color_it();
	
	
    }

	bcm2835_close();	
   return 0;
}


