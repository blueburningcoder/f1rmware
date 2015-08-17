/*
 * Copyright 2015 team rad1o
 *
 * This is a small sample for a l0dable
 *
 */

#include <libopencm3/lpc43xx/gpio.h>
#include <rad1olib/pins.h>
#include <rad1olib/setup.h>
#include <r0ketlib/keyin.h>
#include <r0ketlib/itoa.h>
#include "usetable.h"

void ram(void){
	lcdPrintln("l0dable-Test");
	lcdDisplay();

	lcdNl();
	lcdDisplay();
	while(1){
	    lcdSetCrsr(0,8*8);
	    lcdPrint("ctr=");
	    lcdPrint(IntToStr(_timectr/1000,5,F_LONG));
	    lcdDisplay();
	    switch(getInputRaw()){
		case BTN_ENTER:
		    return;
		case BTN_LEFT:
		    TOGGLE(LED3);   // the LED to the left
            delay(10000000);
		    break;
		case BTN_RIGHT:
<<<<<<< HEAD
		    TOGGLE(LED2);   // the LED on the lower right
		    delay(10000000);
=======
		    TOGGLE(LED3);
		    delayNop(10000000);
>>>>>>> c3d4f2ef7a61ff9142d8a438f24f243c48192dbb
		    break;
		case BTN_UP:
		    TOGGLE(LED4);   // the red LED on the top
		    delay(1000000);
		    break;
		case BTN_DOWN:
		    TOGGLE(LED1);   // the charging-LED
		    delay(10000000);
		    break;
	    };
	}
	getInputWaitRelease();
	return;
}
