/**************************************************************************/
/*!
	Original driver from adafruit:
    @file     tcs34725.cpp
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the TCS34725 digital color sensors.
	
	Modified by FFE: replace low level functions from arduino I2C/TWI 
		to linux i2c-dev 


    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#include "tcs34725.h"
#include "smbus.h"
/*========================================================================*/
/*                          PRIVATE FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    @brief  Implements missing powf function
*/
/**************************************************************************/
float powf(const float x, const float y)
{
  return (float)(pow((double)x, (double)y));
}


/**************************************************************************/
/*!
    @brief  Open i2c device (i2c-dev)
*/
/**************************************************************************/

int tcs34725::i2c_begin(const char *dev_name){

	// Open port for reading and writing
	if ((fd = open(dev_name, O_RDWR)) < 0){
		fprintf(stderr, "%s: %s\n", strerror(errno), dev_name);
		return -1;
	}

	// Set the port options and set the address of the device
	if (ioctl(fd, I2C_SLAVE, TCS34725_ADDRESS) < 0){
		close(fd);
		fprintf(stderr, "%s: set slave address: %s\n", strerror(errno), dev_name);
		return -1;
	}

	return fd;  

}


int tcs34725::i2c_end(void){
  int res;
  res = close(fd);
  
  if(res < 0)
    perror("can't close i2c dev");
	
	return res;
}
	

   

/**************************************************************************/
/*!
    @brief  Writes a register and an 8 bit value over I2C
*/
/**************************************************************************/
void tcs34725::write8 (uint8_t reg, uint8_t value)
{
  int res;
  res = i2c_begin(i2c_dev_name);
  
  if (res < 0)
    return;

  if (i2c_smbus_write_byte_data(fd, TCS34725_COMMAND_BIT | reg, value) < 0) {
    i2c_end();
    perror(__FUNCTION__);
	 return;
  }

  i2c_end();

}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t tcs34725::read8(uint8_t reg)
{
  int res;

  res = i2c_begin(i2c_dev_name);
  
  if (res < 0)
    return -1;

  /* return neg value if error or byte (0 - 0xff) */
  res = i2c_smbus_read_byte_data(fd, TCS34725_COMMAND_BIT | reg);

  if(res < 0) {
    i2c_end();
    perror(__FUNCTION__);
	 return (uint8_t) -1;
  }else{
    return (uint8_t) res;
  }
  
  i2c_end();

}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
uint16_t tcs34725::read16(uint8_t reg)
{

  int res;

  res = i2c_begin(i2c_dev_name);
  
  if (res < 0)
    return -1;

  /* return neg value if error or byte (0 - 0xff) */
  res = i2c_smbus_read_word_data(fd, TCS34725_COMMAND_BIT | reg);
  i2c_end();
  
  if(res < 0) {

    perror(__FUNCTION__);
	 return (uint16_t) -1;
  }else{
    return (uint16_t) res;
  }
  

}

/**************************************************************************/
/*!
    Enables the device
*/
/**************************************************************************/
void tcs34725::enable(void)
{
  write8(TCS34725_ENABLE, TCS34725_ENABLE_PON);
  usleep(3 * 1000);
  write8(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);  
}

/**************************************************************************/
/*!
    Disables the device (putting it in lower power sleep mode)
*/
/**************************************************************************/
void tcs34725::disable(void)
{
  /* Turn the device off to save power */
  uint8_t reg = 0;
  reg = read8(TCS34725_ENABLE);
  write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
}

/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
    Constructor
*/
/**************************************************************************/
tcs34725::tcs34725(const char *i2c_device_name, tcs34725IntegrationTime_t it, tcs34725Gain_t gain) 
{
  strcpy(i2c_dev_name, i2c_device_name);
  _tcs34725Initialised = false;
  _tcs34725IntegrationTime = it;
  _tcs34725Gain = gain;
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    Initializes I2C and configures the sensor (call this function before
    doing anything else)
*/
/**************************************************************************/
bool tcs34725::begin(void) 
{
  
  /* Make sure we're actually connected */
  uint8_t x = read8(TCS34725_ID);
  printf("TCS34725 ID = %02x\n", x);
  if ((x != 0x44) && (x != 0x10))
  {
    return false;
  }
  _tcs34725Initialised = true;

  /* Set default integration time and gain */
  setIntegrationTime(_tcs34725IntegrationTime);
  setGain(_tcs34725Gain);

  /* Note: by default, the device is in power down mode on bootup */
  enable();

  return true;
}
  
/**************************************************************************/
/*!
    Sets the integration time for the TC34725
*/
/**************************************************************************/
void tcs34725::setIntegrationTime(tcs34725IntegrationTime_t it)
{
  if (!_tcs34725Initialised) begin();

  /* Update the timing register */
  write8(TCS34725_ATIME, it);

  /* Update value placeholders */
  _tcs34725IntegrationTime = it;
}

/**************************************************************************/
/*!
    Adjusts the gain on the TCS34725 (adjusts the sensitivity to light)
*/
/**************************************************************************/
void tcs34725::setGain(tcs34725Gain_t gain)
{
  if (!_tcs34725Initialised) begin();

  /* Update the timing register */
  write8(TCS34725_CONTROL, gain);

  /* Update value placeholders */
  _tcs34725Gain = gain;
}

/**************************************************************************/
/*!
    @brief  Reads the raw red, green, blue and clear channel values
*/
/**************************************************************************/
void tcs34725::getRawData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
{
  if (!_tcs34725Initialised) begin();

  *c = read16(TCS34725_CDATAL);
  *r = read16(TCS34725_RDATAL);
  *g = read16(TCS34725_GDATAL);
  *b = read16(TCS34725_BDATAL);
  
  /* Set a delay for the integration time */
  switch (_tcs34725IntegrationTime)
  {
    case TCS34725_INTEGRATIONTIME_2_4MS:
      usleep(4 * 1000);
      break;
    case TCS34725_INTEGRATIONTIME_24MS:
      usleep(24 * 1000);
      break;
    case TCS34725_INTEGRATIONTIME_50MS:
      usleep(50 * 1000);
      break;
    case TCS34725_INTEGRATIONTIME_101MS:
      usleep(101 * 1000);
      break;
    case TCS34725_INTEGRATIONTIME_154MS:
      usleep(154 * 1000);
      break;
    case TCS34725_INTEGRATIONTIME_700MS:
      usleep(700 * 1000);
      break;
  }
}

/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to color temperature in degrees
            Kelvin
*/
/**************************************************************************/
uint16_t tcs34725::calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
{
  float X, Y, Z;      /* RGB to XYZ correlation      */
  float xc, yc;       /* Chromaticity co-ordinates   */
  float n;            /* McCamy's formula            */
  float cct;

  /* 1. Map RGB values to their XYZ counterparts.    */
  /* Based on 6500K fluorescent, 3000K fluorescent   */
  /* and 60W incandescent values for a wide range.   */
  /* Note: Y = Illuminance or lux                    */
  X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
  Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  Z = (-0.68202F * r) + (0.77073F * g) + ( 0.56332F * b);

  /* 2. Calculate the chromaticity co-ordinates      */
  xc = (X) / (X + Y + Z);
  yc = (Y) / (X + Y + Z);

  /* 3. Use McCamy's formula to determine the CCT    */
  n = (xc - 0.3320F) / (0.1858F - yc);

  /* Calculate the final CCT */
  cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

  /* Return the results in degrees Kelvin */
  return (uint16_t)cct;
}



/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to HSV.
				normalise_max 
*/
/**************************************************************************/

hsv_t tcs34725::calculateRgbInt2Hsv(uint16_t r, uint16_t g, uint16_t b, uint16_t normalise_max)
{
    hsv_t       out;
	 rgb_t		 in;
    double      min, max, delta;


	/* avoid to have a final rgb val > 1, update normalise_max if too low */ 	
	if(r > normalise_max)
		normalise_max = r;
	
	if(g > normalise_max)
		normalise_max = g;

	if(b > normalise_max)
		normalise_max = b;
	
	

	/* convert rgb int to float */
	in.r = (double) r;
	in.g = (double) g;
	in.b = (double) b;

	in.r /= normalise_max;
	in.g /= normalise_max;
	in.b /= normalise_max;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
       // if max is 0, then r = g = b = 0              
       // s = 0, v is undefined
 		 out.s = 0.0;
       out.h = NAN;                            // its now undefined
       return out;
    }

    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}



/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to HSV.
				normalise_max 
*/
/**************************************************************************/

hsl_t tcs34725::calculateRgbInt2Hsl(uint16_t r, uint16_t g, uint16_t b, uint16_t normalise_max)
{
	hsv_t in;
	hsl_t	out;	
	in = calculateRgbInt2Hsv(r,g,b, normalise_max);

	out.h = in.h;
	out.l = (2 - in.s) * in.v;
 	out.s = in.s * in.v;
 	out.s /= (out.l <= 1) ? (out.l) : 2 - (out.l);
 	out.l /= 2;


    return out;
}


/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to color temperature in degrees
            Kelvin
*/
/**************************************************************************/
uint16_t tcs34725::calculateLux(uint16_t r, uint16_t g, uint16_t b)
{
  float illuminance;

  /* This only uses RGB ... how can we integrate clear or calculate lux */
  /* based exclusively on clear since this might be more reliable?      */
  illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

  return (uint16_t)illuminance;
}


void tcs34725::setInterrupt(bool i) {
  uint8_t r = read8(TCS34725_ENABLE);
  if (i) {
    r |= TCS34725_ENABLE_AIEN;
  } else {
    r &= ~TCS34725_ENABLE_AIEN;
  }
  write8(TCS34725_ENABLE, r);
}

void tcs34725::clearInterrupt(void) {
  int res;
  res = i2c_begin(i2c_dev_name);
  
  if (res < 0)
    return;

  if (i2c_smbus_write_byte(fd, 0x66) < 0){
    i2c_end();
    perror(__FUNCTION__);
	 return;
  }

  i2c_end();

}


void tcs34725::setIntLimits(uint16_t low, uint16_t high) {
   write8(0x04, low & 0xFF);
   write8(0x05, low >> 8);
   write8(0x06, high & 0xFF);
   write8(0x07, high >> 8);
}
