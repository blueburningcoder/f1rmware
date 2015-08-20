/*
 * This l0dable by Benedikt Roth and Stefan Tomanek serves as your main
 * viewscreen, displaying your nickname the way you want it, and lets the
 * stars zoom by. You can accelerate your vessel by pushing the joystick
 * upwards or bring it to a halt by pressing it down - leaving your ship
 * drifting in the endless space. Attach two LEDs to the modulbus
 * connectors (SS2->GND, SS5->GND), so your r0ket can light up its nacelles
 * when breaking the warp barrier.
 *
 * commit 33fe346942176a0e988818980d04d1a8f746f894 1 parent 0eaf74fa87
 * wertarbyte authored August 13, 2011
 *
 * Modified by Bez, on the CCCamp '15 and afterwards
 * github: blueburningcoder
 *
 */
#include <r0ketlib/config.h>
#include <r0ketlib/display.h>
#include <r0ketlib/fonts.h>
#include <r0ketlib/render.h>
#include <r0ketlib/print.h>
#include <r0ketlib/keyin.h>
#include <rad1olib/pins.h>

#include "usetable.h"

#define NUM_STARS 100
#define SPEED_MAX 10
#define SPEED_DEFAULT 4
#define SPEED_STOP 0
#define SPEED_WARP 6



typedef struct {
    short startZ;
    short oldStartX, oldStartY, oldStartZ, oldEndZ;
} s_line;

typedef struct {
	short x, y, z;
    s_line line;
} s_star;

typedef struct {
	short speed;
} s_ship;

static s_ship ship = {SPEED_DEFAULT};

static s_star stars[NUM_STARS];

static bool isWarp = false;

void init_star(s_star *star, int z);
void set_warp_lights(uint8_t enabled);
void drift_ship(void);
void warp(s_star* star, short tempx, short tempy);

void ram(void)
{
	short centerx = RESX >> 1;
	short centery = RESY >> 1;
	short i;
	uint8_t key = 0;

	for (i = 0; i < NUM_STARS; i++) {
		init_star(stars + i, i + 1);
	}

	static uint8_t count = 0;
	while(1) {
		count++;
		count%=256;
		key = getInputRaw();
		if (key == BTN_ENTER) {
			break;
		} else if ( count%4 == 0 ) {
			if (key == BTN_UP && ship.speed < SPEED_MAX) {
				ship.speed++;
			} else if (key == BTN_DOWN && ship.speed > SPEED_STOP) {
				ship.speed--;
			} else if (key == BTN_NONE && count % 12 == 0) {
				/* converge towards default speed */
				if (ship.speed < SPEED_DEFAULT)
					ship.speed++;
				else if (ship.speed > SPEED_DEFAULT)
					ship.speed--;	
			}
		}

		if (ship.speed > SPEED_WARP) {
			set_warp_lights(1);
		} else {
			set_warp_lights(0);
		}

        if (count % 3 == 0) OFF(LED4);
		if (ship.speed == 0 && count%6==0) 
            drift_ship();
 

		int dx=0;
		int dy=0;
		setExtFont(GLOBAL(nickfont));
		setTextColor(0x00,0xFF);
		dx=DoString(0,0,GLOBAL(nickname));
		dx=(RESX-dx)/2;
		if(dx<0) dx=0;
		dy=(RESY-getFontHeight())/2;

		lcdFill(0x00);
		DoString(dx,dy,GLOBAL(nickname));

		for (i = 0; i < NUM_STARS; i++) {
			stars[i].z -= ship.speed;

			if (ship.speed > 0 && stars[i].z <= 0)
				init_star(stars + i, i + 1);

			short tempx = ((stars[i].x * 30) / stars[i].z) + centerx;
			short tempy = ((stars[i].y * 30) / stars[i].z) + centery;

			if (tempx < 0 || tempx > RESX - 1 || tempy < 0 || tempy > RESY - 1) {
				if (ship.speed > 0) { /* if we are flying, generate new stars in front */
					init_star(stars + i, i + 1);
				} else { /* if we are drifting, simply move those stars to the other end */
					stars[i].x = (((tempx%RESX)-centerx)*stars[i].z)/30;
					stars[i].y = (((tempy%RESY)-centery)*stars[i].z)/30;
				}
				continue;
			}

			lcdSetPixel(tempx, tempy, 0xFF);
			if (stars[i].z < 50) {
				lcdSetPixel(tempx + 1, tempy, 0xFF);
			}
			if (stars[i].z < 20) {
				lcdSetPixel(tempx, tempy + 1, 0xFF);
				lcdSetPixel(tempx + 1, tempy + 1, 0xFF);
			}
            if (isWarp)
                warp(stars + i, tempx, tempy);
		}

		lcdDisplay();

		delayms_queue_plus(50,0);
	}
	set_warp_lights(0);
}

void warp(s_star* star, short tempx, short tempy) {
    short length = 0;
    if (ship.speed < 6);
    else if (ship.speed == 6)
        length = 3;
    else if (ship.speed >= 7)
        length = 8;
    short diffx = tempx - (RESX >> 1),
          diffy = tempy - (RESY >> 1);
    float diff = diffx / diffy;
    if (diffx >= 0 && diffy >= 0) {
        lcdSetPixel(tempx - 1, tempy - 1, 0xEE);
        lcdSetPixel(tempx - 2, tempy - 2, 0xDD);
        lcdSetPixel(tempx - 3, tempy - 3, 0xCC);
    } else if (diffx <= 0 && diffy >= 0) {
        lcdSetPixel(tempx + 1, tempy - 1, 0xEE);
        lcdSetPixel(tempx + 2, tempy - 2, 0xDD);
        lcdSetPixel(tempx + 3, tempy - 3, 0xCC);
    } else if (diffx >= 0 && diffy <= 0) {
        lcdSetPixel(tempx - 1, tempy + 1, 0xEE);
        lcdSetPixel(tempx - 2, tempy + 2, 0xDD);
        lcdSetPixel(tempx - 3, tempy + 3, 0xCC);
    } else if (diffx <= 0 && diffy <= 0) {
        lcdSetPixel(tempx + 1, tempy + 1, 0xEE);
        lcdSetPixel(tempx + 2, tempy + 2, 0xDD);
        lcdSetPixel(tempx + 3, tempy + 3, 0xCC);
    }
    /*
     * for every diffx / diffy -> float
     * there's one diffy
     */

}

void set_warp_lights(uint8_t enabled) {

    if (enabled) {
        ON(LED2);
        ON(LED3);
        isWarp = true;
    } else {
        OFF(LED2);
        OFF(LED3);
        isWarp = false;
    }

	return;
	//gpioSetValue(LEDA_R, 0);
	//gpioSetValue(LEDA_G, 0);
	//gpioSetValue(LEDA_B, enabled);

	//gpioSetValue(LEDB_R, 0);
	//gpioSetValue(LEDB_G, 0);
	//gpioSetValue(LEDB_B, enabled);
}

void drift_ship(void) {
	uint8_t d_x = 1;
	uint8_t d_y = 1;
	for (uint8_t i = 0; i < NUM_STARS; i++) {
		stars[i].x += d_x;
		stars[i].y += d_y;
	}
    ON(LED4);
}

void reset_star(s_star* star) {
    star->line.oldEndZ = star->z;
    star->line.oldStartZ = star->line.startZ;
    star->line.oldStartX = star->x;
    star->line.oldStartY = star->y;
    star->line.oldEndZ = star->z;
}

void init_star(s_star *star, int z)
{
    reset_star(star);
	star->x = (getRandom() % RESX) - (RESX >> 1);
	star->y = (getRandom() % RESY) - (RESY >> 1);
	star->z = z;
    star->line.startZ = star->z;

	return;
}
