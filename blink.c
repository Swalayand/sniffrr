#include <bcm2835.h>

#include <time.h>


int c_sleep_msec(long milliseconds) {
    struct timespec req;
    //struct timespec rem;
    if(milliseconds > 999) {
      req.tv_sec = (int)(milliseconds / 1000);  // Must be Non-Negative 
      req.tv_nsec = (milliseconds - ((long)req.tv_sec * 1000)) * 1000000; // Must be in range of 0 to 999999999 
    }
    else {
      req.tv_sec = 0;                         // Must be Non-Negative 
      req.tv_nsec = milliseconds * 1000000;    // Must be in range of 0 to 999999999 
    }
    //rem = NULL;
    return nanosleep(&req , NULL);
}
int c_sleep_usec(long microsecond) { // micro second
    struct timespec req;
    //struct timespec rem;
    if(microsecond > 999999) {
      req.tv_sec = (int)(microsecond / 1000000);  // Must be Non-Negative 
      req.tv_nsec = (microsecond - ((long)req.tv_sec * 1000000)); // Must be in range of 0 to 999999 
    }
    else {
      req.tv_sec = 0;                         // Must be Non-Negative 
      req.tv_nsec = microsecond * 1000;    // Must be in range of 0 to 999999 
    }
    //rem = NULL;
    return nanosleep(&req , NULL);
}
//------------------------------------------------------
int c_sleep_nsec(long nanoseconds) {
    struct timespec req;
    //struct timespec rem;
    if (nanoseconds > 999999999) {
      req.tv_sec = (int)(nanoseconds / 1000000000);
      req.tv_nsec = (nanoseconds - ((long)req.tv_sec * 1000000000));
    }
    else {
      req.tv_sec = 0;
      req.tv_nsec = nanoseconds / 100;
    }
    //rem = NULL;
    return nanosleep(&req , NULL);
}


//gcc blink.c -l rt -l bcm2835



// Blinks on RPi pin GPIO 11
#define PIN RPI_GPIO_P1_11  // pin 17 di 40 Raspi

int main(int argc, char **argv)
{
    // If you call this, it will not actually access the GPIO
    // Use for testing
//    bcm2835_set_debug(1);

  if (!bcm2835_init()) return 1;

    // Set the pin to be an output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);

    // Blink
    while (1)
    {
	// Turn it on
	//bcm2835_gpio_write(PIN, HIGH);
	
	// wait a bit
	// c_sleep_msec( 500 ); //delay(500);
	
	// turn it off
	//bcm2835_gpio_write(PIN, LOW);
	
        //c_sleep_msec( 500 );
/*
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.398125) # A
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.000128) # B
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.000000750) # C
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.000128) # D
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.000000750) # E
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.0025) # F 2.5 ms
    GPIO.output(18,GPIO.HIGH)
    time.sleep(0.00000225) # G 2.25 us
    GPIO.output(18,GPIO.LOW)
    time.sleep(0.000161375) # H 2.5 ms
*/
  bcm2835_gpio_write(PIN, HIGH);
  c_sleep_msec( 398.125 );
  bcm2835_gpio_write(PIN, LOW);
  c_sleep_usec( 128 );//398.125 );
  bcm2835_gpio_write(PIN, HIGH);
  delay_nano( 750 ); //c_sleep_nsec( 750 );//398.125 );
  bcm2835_gpio_write(PIN, LOW);
  c_sleep_usec( 128 );
  }

  return 0;
}

/*
#include <wiringPi.h>

int main(void) {
    if( wiringPiSetup() == -1 ) return 1;
    pinMode(0,OUTPUT);
    for(;;) {
        digitalWrite(0,1);
        //delay(500);
        c_sleep_msec( 500 );
        digitalWrite(0,0);
        c_sleep_msec( 500 );
        //delay(500);
    }
    return 0;
}

*/