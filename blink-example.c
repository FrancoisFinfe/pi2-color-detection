// blink.c
//
// Example program for bcm2835 library
// Blinks a pin on an off every 0.5 secs
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o blink blink.c -l bcm2835
// sudo ./blink
//
// Or you can test it before installing with:
// gcc -o blink -I ../../src ../../src/bcm2835.c blink.c
// sudo ./blink
//
// Author: Mike McCauley
// Copyright (C) 2011 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $

#include <bcm2835.h>

// Blinks on RPi Plug P1 pin 11 (which is GPIO pin 17)
#define PIN RPI_GPIO_P1_11

uint8_t ehw298_bcm2835_pin[] = {
	4, 17, 18, 27, 22, 23, 24, 25,
	5, 6, 12, 13, 19, 16, 26, 20};





int main(int argc, char **argv)
{
    // If you call this, it will not actually access the GPIO
    // Use for testing
//    bcm2835_set_debug(1);
	int i;

    if (!bcm2835_init())
	return 1;


	for(i=0; i<sizeof(ehw298_bcm2835_pin); i++){
		bcm2835_gpio_fsel(ehw298_bcm2835_pin[i], BCM2835_GPIO_FSEL_OUTP);	
	}

    // Set the pin to be an output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);

    // Blink
    while (1)
    {
		for(i=0; i<sizeof(ehw298_bcm2835_pin); i++){

			// Turn it on
			bcm2835_gpio_write(ehw298_bcm2835_pin[i], HIGH);
	
			// wait a bit
			usleep(250*1000);
	
			// turn it off
			bcm2835_gpio_write(ehw298_bcm2835_pin[i], LOW);
	
			// wait a bit
			usleep(250 * 1000);
		}
    }
    bcm2835_close();
    return 0;
}


